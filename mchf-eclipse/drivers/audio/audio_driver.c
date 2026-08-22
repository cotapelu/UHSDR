/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                               mcHF QRP Transceiver                              **
 **                             K Atanassov - M0NKA 2014                            **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:                                                                     **
 **  Description:                                                                   **
 **  Last Modified:                                                                 **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

// Common
#include <assert.h>
#include "uhsdr_board.h"
#include "ui_driver.h"
#include "profiling.h"

#include <stdio.h>
#include <math.h>
#include "codec.h"

#include "cw_gen.h"

#include <limits.h>
#include "softdds.h"

#include "audio_driver.h"
#include "audio_driver_ctx.h"  /* T31.2: internal context */
#include "audio_nr.h"
#include "audio_agc.h"
#include "audio_management.h"
#include "radio_management.h"
#include "usbd_audio_if.h"
#include "ui_spectrum.h"
#include "filters.h"
#include "hal_i2s.h"
#include "rtty.h"
#include "psk.h"
#include "cw_decoder.h"
#include "freedv_uhsdr.h"
#include "freq_shift.h"
#include "audio_nr.h"
#include "ui_configuration.h"
#ifdef USE_CONVOLUTION
#include "audio_convolution.h"
#endif

#include "fm_subaudible_tone_table.h" // hm.
#include "uhsdr_math.h"
#include "audio_driver_filters.h"
#include "tx_processor.h"


// Audio driver context - defined here, declared in audio_driver.h
__MCHF_SPECIALMEM AudioDriver_Context_t g_audio_driver_ctx = {0};
// SSB filters - now handled in ui_driver to allow I/Q phase adjustment

#define LMS2_NOTCH_STATE_ARRAY_SIZE (DSP_NOTCH_NUMTAPS_MAX + IQ_BLOCK_SIZE)

#ifdef USE_LMS_AUTONOTCH
typedef struct
{
    float32_t   errsig2[IQ_BLOCK_SIZE];
    arm_lms_norm_instance_f32	lms2Norm_instance;
    arm_lms_instance_f32	    lms2_instance;
    float32_t	                lms2StateF32[LMS2_NOTCH_STATE_ARRAY_SIZE];
    float32_t	                lms2NormCoeff_f32[DSP_NOTCH_NUMTAPS_MAX];
    float32_t	                lms2_nr_delay[DSP_NOTCH_BUFLEN_MAX];
} LMSData;
#endif


// variables for RX IIR filters

// variables for RX antialias IIR filter

// static float32_t Koeff[20];
// variables for RX manual notch, manual peak & bass shelf IIR biquad filter

// variables for RX treble shelf IIR biquad filter

// variables for ZoomFFT lowpass filtering

// sr = 12ksps, Fstop = 2k7, we lowpass-filtered the audio already in the main aido path (IIR),
// so only the minimum size filter (4 taps) is used here

// 12ksps, Fstop = 2k7, KAISER, 40 taps, a good interpolation filter after the interpolation, maybe too many taps?

// this is wrong! Interpolation filters act at the sample rate AFTER the interpolation, in this case at 12ksps
// 6ksps, Fstop = 2k65, KAISER
//

#ifdef USE_SIMPLE_FREEDV_FILTERS
//******* From here 2 set of filters for the I/Q FreeDV aliasing filter**********
// I- and Q- Filter instances for FreeDV downsampling aliasing filters

#endif

//******* End of 2 set of filters for the I/Q FreeDV aliasing filter**********

// S meter public
SMeter	sm;

// ATTENTION: These data structures have been placed in CCM Memory (64k)
// IF THE SIZE OF  THE DATA STRUCTURE GROWS IT WILL QUICKLY BE OUT OF SPACE IN CCM
// Be careful! Check mchf-eclipse.map for current allocation
AudioDriverState   __MCHF_SPECIALMEM ads;
AudioDriverBuffer  __MCHF_SPECIALMEM adb;

#if defined(USE_LMS_AUTONOTCH)
LMSData            __MCHF_SPECIALMEM lmsData;
#endif

#ifdef USE_LEAKY_LMS
lLMS leakyLMS;
#endif

SnapCarrier   sc;

/**
 * @returns offset frequency in Hz for current frequency translate mode
 */
int32_t AudioDriver_GetTranslateFreq()
{
    int32_t fdelta = 0;
    switch (ts.iq_freq_mode)
    {
    case FREQ_IQ_CONV_P6KHZ:
        fdelta = 6000;
        break;
    case FREQ_IQ_CONV_M6KHZ:
        fdelta = - 6000;
        break;
    case FREQ_IQ_CONV_P12KHZ:
        fdelta = 12000;
        break;
    case FREQ_IQ_CONV_M12KHZ:
        fdelta = -12000;
        break;
    }
    return fdelta;
}

// RX variables for FM squelch IIR filters

static void AudioDriver_AudioFillSilence(AudioSample_t *s, size_t size)
{
    memset(s,0,size*sizeof(*s));
}

static void AudioDriver_IqFillSilence(IqSample_t *s, size_t size)
{
    memset(s,0,size*sizeof(*s));
}

//*----------------------------------------------------------------------------
//* Function Name       : I2S_RX_CallBack
//* Object              :
//* Object              : audio sample processor
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void AudioDriver_I2SCallback(AudioSample_t *audio, IqSample_t *iq, AudioSample_t *audioDst, int16_t blockSize)
{
    static bool to_rx = false;	// used as a flag to clear the RX buffer
    static bool to_tx = false;	// used as a flag to clear the TX buffer
    bool muted = false;

    if(ts.show_debug_info)
    {
        Board_GreenLed(LED_STATE_ON);
    }

    if((ts.txrx_mode == TRX_MODE_RX))
    {
        if((to_rx) || ts.audio_processor_input_mute_counter > 0)	 	// the first time back to RX, clear the buffers to reduce the "crash"
        {
            muted = true;
            AudioDriver_IqFillSilence(iq, blockSize);
            if (to_rx)
            {
                UhsdrHwI2s_Codec_ClearTxDmaBuffer();
            }
            if ( ts.audio_processor_input_mute_counter >0)
            {
                ts.audio_processor_input_mute_counter--;
            }
            to_rx = false;                          // caused by the content of the buffers from TX - used on return from SSB TX
        }

#ifdef USE_CONVOLUTION
        AudioDriver_RxProcessorConvolution(iq, audio, blockSize, muted);
#else
        AudioDriver_RxProcessor(iq, audio, blockSize, muted);
#endif
        if (ts.audio_dac_muting_buffer_count > 0)
        {
            ts.audio_dac_muting_buffer_count--;
        }

        if (muted == false)
        {
            if (ts.cw_keyer_mode != CW_KEYER_MODE_STRAIGHT && (ts.cw_text_entry || ts.dmod_mode == DEMOD_CW)) // FIXME to call always when straight mode reworked
            {
                CwGen_Process(adb.iq_buf.i_buffer, adb.iq_buf.q_buffer, blockSize);
            }
        }

        to_tx = true;		// Set flag to indicate that we WERE receiving when we go back to transmit mode
    }
    else  			// Transmit mode
    {
        if (to_tx)
        {
            TxProcessor_PrepareRun(); // last actions before we go live
        }
        if((to_tx) || (ts.audio_processor_input_mute_counter > 0) || ts.audio_dac_muting_flag || ts.audio_dac_muting_buffer_count > 0)	 	// the first time back to TX, or TX audio muting timer still active - clear the buffers to reduce the "crash"
        {
            muted = true;
            AudioDriver_AudioFillSilence(audio, blockSize);
            to_tx = false;                          // caused by the content of the buffers from TX - used on return from SSB TX
            if (ts.audio_processor_input_mute_counter > 0)
            {
                ts.audio_processor_input_mute_counter--;
            }
        }

        TxProcessor_Run(audio, iq, audioDst,blockSize, muted);

        // Pause or inactivity
        if (ts.audio_dac_muting_buffer_count)
        {
            ts.audio_dac_muting_buffer_count--;
        }

        to_rx = true;		// Set flag to indicate that we WERE transmitting when we eventually go back to receive mode
    }

    UiDriver_Callback_AudioISR();

    if(ts.show_debug_info)
    {
        Board_GreenLed(LED_STATE_OFF);
    }

#ifdef USE_PENDSV_FOR_HIGHPRIO_TASKS
    // let us trigger a pendsv irq here in order to trigger execution of UiDriver_HighPrioHandler()
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
#endif
}

