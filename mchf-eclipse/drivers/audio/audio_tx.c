/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------** **                                                                                 **
 **  File name:       audio_tx.c                                                    **
 **  Description:     TX audio processing (extracted from audio_rx.c, T31.2)        **
 **  Last Modified:   2026-08-22                                                     **
 **  Licence:         GNU GPLv3                                                      **
 ************************************************************************************/

#include "audio_tx.h"
#include "audio_driver.h"

#include "audio_driver_ctx.h"  /* T31.2: internal context */

#include "uhsdr_board.h"     // for AUDIO_SAMPLE_RATE, is_dsp_mnotch, is_dsp_mpeak
#include "codec.h"           // for CODEC_SPEAKER_MAX_VOLUME
#include "tx_processor.h"    // for TxProcessor_Set

//
// However, the cookbook formulae by Robert Bristow-Johnson AND the Iowa Hills IIR Filter designer
// use this formula:
//
// y[n] = coeffs[B0] * x[n] + coeffs[B1] * x[n-1] + coeffs[B2] * x[n-2] - coeffs[A1] * y[n-1] - coeffs[A2] * y[n-2]
//
// Therefore, we have to use negated coeffs[A1] and coeffs[A2] for use with the ARM function
// notch implementation
//
// we also have to divide every coefficient by a0 !
// y[n] = coeffs[B0]/a0 * x[n] + coeffs[B1]/a0 * x[n-1] + coeffs[B2]/a0 * x[n-2] - coeffs[A1]/a0 * y[n-1] - coeffs[A2]/a0 * y[n-2]

/**
 * @brief Filter Init used for processing audio for RX and TX
 */
void AudioDriver_SetRxTxAudioProcessingAudioFilters(uint8_t dmod_mode)
{
    float32_t FSdec = AUDIO_SAMPLE_RATE / (ts.filters_p->sample_rate_dec != 0 ? ts.filters_p->sample_rate_dec : 1) ;

    // the notch filter is in biquad 1 and works at the decimated sample rate FSdec

    float32_t coeffs[5];
    const float32_t *coeffs_ptr;

    // setting the Coefficients in the notch filter instance
    if (is_dsp_mnotch())
    {
        AudioDriver_CalcBandstop(coeffs, ts.dsp.notch_frequency , FSdec);
        coeffs_ptr = coeffs;
    }
    else   // passthru
    {
        coeffs_ptr = biquad_passthrough;
    }

    AudioDriver_SetBiquadCoeffsAllInstances(IIR_biquad_1, 0, coeffs_ptr);

    // this is an auto-notch-filter detected by the NR algorithm
    // biquad 1, 4th stage
    //    if(0) // temporarily deactivated
    //    if((is_dsp_notch()) && (ts.filters_p->sample_rate_dec == RX_DECIMATION_RATE_12KHZ))

    AudioDriver_SetBiquadCoeffsAllInstances(IIR_biquad_1, 15, biquad_passthrough);

    // the peak filter is in biquad 1 and works at the decimated sample rate FSdec
    if(is_dsp_mpeak())
    {
        AudioDriver_CalcBandpass(coeffs, ts.dsp.peak_frequency, FSdec);
        coeffs_ptr = coeffs;
    }
    else   //passthru
    {
        coeffs_ptr = biquad_passthrough;
    }

    AudioDriver_SetBiquadCoeffsAllInstances(IIR_biquad_1, 5, coeffs_ptr);

    // EQ shelving filters
    //
    // the bass filter is in biquad 1 and works at the decimated sample rate FSdec
    //
    // Bass / lowShelf
    AudioDriver_CalcLowShelf(coeffs, 250, 0.7, ts.dsp.bass_gain, FSdec);
    AudioDriver_SetBiquadCoeffsAllInstances(IIR_biquad_1, 10, coeffs);

    // Treble = highShelf
    // the treble filter is in biquad 2 and works at 48000ksps
    AudioDriver_CalcHighShelf(coeffs, 3500, 0.9, ts.dsp.treble_gain, AUDIO_SAMPLE_RATE);
    AudioDriver_SetBiquadCoeffsAllInstances(IIR_biquad_2, 0, coeffs);

    TxProcessor_Set(dmod_mode);
}
