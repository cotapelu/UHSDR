/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                **
 **                                        UHSDR                                   **
 **               a powerful firmware for STM32 based SDR transceivers             **
 **                                                                                **
 **--------------------------------------------------------------------------------**
 **                                                                                **
 **  Description:   Code for fast convolution filtering, DD4WH 2018_08_19          **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

#include "audio_convolution.h"
#include "audio_driver.h"
#include "arm_const_structs.h"
#include "filters.h"

// we cannot use a shared buffer structure with FreeDV,
// because we need to filter with convolution and simultaneously
// use FreeDV buffers, if we switch on digital voice mode FreeDV
// user code

#ifdef USE_CONVOLUTION
__IO int32_t Sample_in_head = 0;
__IO int32_t Sample_in_tail = 0;
__IO int32_t Sample_out_head = 0;
__IO int32_t Sample_out_tail = 0;

Sample_Buffer* Sample_in_buffers[SAMPLE_BUFFER_FIFO_SIZE];
Sample_Buffer* Sample_out_buffers[SAMPLE_BUFFER_FIFO_SIZE];

int Sample_in_buffer_peek(Sample_Buffer** c_ptr)
{
    int ret = 0;

    if (Sample_in_head != Sample_in_tail)
    {
        Sample_Buffer* c = Sample_in_buffers[Sample_in_tail];
        *c_ptr = c;
        ret++;
    }
    return ret;
}


int Sample_in_buffer_remove(Sample_Buffer** c_ptr)
{
    int ret = 0;

    if (Sample_in_head != Sample_in_tail)
    {
        Sample_Buffer* c = Sample_in_buffers[Sample_in_tail];
        Sample_in_tail = (Sample_in_tail + 1) % SAMPLE_BUFFER_FIFO_SIZE;
        *c_ptr = c;
        ret++;
    }
    return ret;
}

/* no room left in the buffer returns 0 */
int Sample_in_buffer_add(NR_Buffer* c)
{
    int ret = 0;
    int32_t next_head = (Sample_in_head + 1) % SAMPLE_BUFFER_FIFO_SIZE;

    if (next_head != Sample_in_tail)
    {
        /* there is room */
        Sample_in_buffers[Sample_in_head] = c;
        Sample_in_head = next_head;
        ret ++;
    }
    return ret;
}

void Sample_in_buffer_reset()
{
    Sample_in_tail = Sample_in_head;
}

int8_t Sample_in_has_data()
{
    int32_t len = Sample_in_head - Sample_in_tail;
    return len < 0?len+SAMPLE_BUFFER_FIFO_SIZE:len;
}

int32_t Sample_in_has_room()
{
    // FIXME: Since we cannot completely fill the buffer
    // we need to say full 1 element earlier
    return SAMPLE_BUFFER_FIFO_SIZE - 1 - Sample_in_has_data();
}


//*********Out Buffer handling

int Sample_out_buffer_peek(Sample_Buffer** c_ptr)
{
    int ret = 0;

    if (Sample_out_head != Sample_out_tail)
    {
        Sample_Buffer* c = Sample_out_buffers[Sample_out_tail];
        *c_ptr = c;
        ret++;
    }
    return ret;
}


int Sample_out_buffer_remove(NR_Buffer** c_ptr)
{
    int ret = 0;

    if (Sample_out_head != Sample_out_tail)
    {
        Sample_Buffer* c = Sample_out_buffers[Sample_out_tail];
        Sample_out_tail = (Sample_out_tail + 1) % SAMPLE_BUFFER_FIFO_SIZE;
        *c_ptr = c;
        ret++;
    }
    return ret;
}

/* no room left in the buffer returns 0 */
int Sample_out_buffer_add(Sample_Buffer* c)
{
    int ret = 0;
    int32_t next_head = (Sample_out_head + 1) % SAMPLE_BUFFER_FIFO_SIZE;

    if (next_head != Sample_out_tail)
    {
        /* there is room */
        Sample_out_buffers[Sample_out_head] = c;
        Sample_out_head = next_head;
        ret ++;
    }
    return ret;
}

void Sample_out_buffer_reset()
{
    Sample_out_tail = Sample_out_head;
}

int8_t Sample_out_has_data()
{
    int32_t len = Sample_out_head - Sample_out_tail;
    return len < 0?len+SAMPLE_BUFFER_FIFO_SIZE:len;
}

int32_t Sample_out_has_room()
{
    // FIXME: Since we cannot completely fill the buffer
    // we need to say full 1 element earlier
    return SAMPLE_BUFFER_FIFO_SIZE - 1 - Sample_out_has_data();
}
#endif

#ifdef USE_CONVOLUTION

static ConvolutionBuffers cob;
ConvolutionBuffersShared cbs;

void AudioDriver_CalcConvolutionFilterCoeffs (int N, float32_t f_low, float32_t f_high, float32_t samplerate, int wintype, int rtype, float32_t scale)
{
	/****************************************************************
	 *  Partitioned Convolution code adapted from wdsp library
	 *  (c) by Warren Pratt under GNU GPLv3
	 ****************************************************************/

	// TODO: how can I define an array with a function ???
	//float32_t *c_impulse = (float32_t *) malloc0 (N * sizeof (complex));
	//float32_t c_impulse[CONVOLUTION_MAX_NO_OF_COEFFS * 2];
	float32_t ft = (f_high - f_low) / (2.0 * samplerate);
	float32_t ft_rad = 2.0 * PI * ft;
	float32_t w_osc = PI * (f_high + f_low) / samplerate;
  int i, j;
  float32_t m = 0.5 * (float32_t)(N - 1);
  float32_t delta = PI / m;
  float32_t cosphi;
  float32_t posi, posj;
  float32_t sinc, window, coef;

  if (N & 1)
  {
    switch (rtype)
    {
    case 0:
      cbs.impulse[N >> 1] = scale * 2.0 * ft;
      break;
    case 1:
      cbs.impulse[N - 1] = scale * 2.0 * ft;
      cbs.impulse[  N  ] = 0.0;
      break;
    }
  }
  for (i = (N + 1) / 2, j = N / 2 - 1; i < N; i++, j--)
  {
    posi = (float32_t)i - m;
    posj = (float32_t)j - m;
    sinc = sinf (ft_rad * posi) / (PI * posi);
    switch (wintype)
    {
    case 0: // Blackman-Harris 4-term
      cosphi = cosf (delta * i);
      window  =             + 0.21747
          + cosphi *  ( - 0.45325
          + cosphi *  ( + 0.28256
          + cosphi *  ( - 0.04672 )));
      break;
    case 1: // Blackman-Harris 7-term
      cosphi = cosf (delta * i);
      window  =       + 6.3964424114390378e-02
          + cosphi *  ( - 2.3993864599352804e-01
          + cosphi *  ( + 3.5015956323820469e-01
          + cosphi *  ( - 2.4774111897080783e-01
          + cosphi *  ( + 8.5438256055858031e-02
          + cosphi *  ( - 1.2320203369293225e-02
          + cosphi *  ( + 4.3778825791773474e-04 ))))));
      break;
    }
    coef = scale * sinc * window;
    switch (rtype)
    {
    case 0:
      cbs.impulse[i] = + coef * cosf (posi * w_osc);
      cbs.impulse[j] = + coef * cosf (posj * w_osc);
      break;
    case 1:
      cbs.impulse[2 * i + 0] = + coef * cosf (posi * w_osc);
      cbs.impulse[2 * i + 1] = - coef * sinf (posi * w_osc);
      cbs.impulse[2 * j + 0] = + coef * cosf (posj * w_osc);
      cbs.impulse[2 * j + 1] = - coef * sinf (posj * w_osc);
      break;
    }
  }
  //return c_impulse;
}
#endif


#ifdef USE_CONVOLUTION
void AudioDriver_SetConvolutionFilter (int nc, float32_t f_low, float32_t f_high, float32_t samplerate, int wintype, float32_t gain)
{
	/****************************************************************
	 *  Partitioned Convolution code adapted from wdsp library
	 *  (c) by Warren Pratt under GNU GPLv3
	 ****************************************************************/

  int i;
  // this calculates the impulse response (=coefficients) of a complex bandpass filter
  // it needs to be complex in order to allow for SSB demodulation
  // this writes the calculated coeffs into the adb.impulse array
  AudioDriver_CalcConvolutionFilterCoeffs (nc, f_low, f_high, samplerate, wintype, 1, gain);
  cbs.buffidx = 0;
  for (i = 0; i < cbs.nfor; i++)
  {
    // I right-justified the impulse response => take output from left side of output buff, discard right side
    // Be careful about flipping an asymmetrical impulse response.
    // DD4WH: unsure how to interpret that, maybe like this:
    // maskgen is of size: 2 * FFT_size * sizeof(complex)
    // maskgen: left half is filled with zeros!? (starts to be filled from maskgen[2 * FFT_size], which is the centre of the array)
    // right half of maskgen: is filled with the relevant part of the impulse response
    // next round takes the next part of the impulse response
    // ???the right half of impulse is not being used (discarded), because 2 * FFT_size * (nfor-1) is maximum of pointer
    for(int idx = 0; idx < cbs.size * 2; idx++)
    {
    	cbs.maskgen[idx] = 0;
    	cbs.maskgen[idx + cbs.size * 2] = cbs.impulse[idx + i * cbs.size * 2];
    }
    //memcpy (&(a->maskgen[2 * a->size]), &(impulse[2 * a->size * i]), a->size * sizeof(complex));

    // do FFT
    arm_cfft_f32(&arm_cfft_sR_f32_len256, cbs.maskgen, 0, 1);
    // take input from maskgen and put output into fmask
    for(int idx = 0; idx < cbs.size * 4; idx++)
    {
    	cob.fmask[i][idx] = cbs.maskgen[idx];
    }

    //fftw_execute (a->maskplan[i]);
  }
  // after the loop is finished,  fmask[nfor][FFT_size * 2] is filled with the FFT outputs of the partitioned filter response

}
#endif

#ifdef USE_CONVOLUTION
void convolution_handle()
{
	// put everything that should happen, when 128 samples are ready, into this function
	/****************************************************************
	 *
	 *  Partitioned Convolution code adapted from wdsp library
	 *
	 *  (c) by Warren Pratt under GNU GPLv3
	 *
	 *  thanks, Warren!
	 *
	 ****************************************************************/


	            // input file: cob.i_buffer_convolution
	            // IIIIIIIIIIIIIIII
	            // <- cbs.size   ->
	            //
	            // input file: cob.q_buffer_convolution
	            // QQQQQQQQQQQQQQQQ
	            // <- cbs.size   ->
	            //

	            // NEW AUDIO SAMPLES: the two files interleaved
	            // IQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQ
	            // <-      2 * cbs.size          ->
	            //

	            // fill new samples into SECOND HALF of fftin
	            // first half is filled with the old audio samples
	            //				| OLD  |					| NEW |
	            // IQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQ
	            // <-      2 * cbs.size          -><-      2 * cbs.size          ->

	            // FFT of fftin
	            // output of FFT is copied into fftout[buffidx]
	            // IQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQ
	            // <-      2 * cbs.size          -><-      2 * cbs.size          ->
	            // there are as many FFT output buffers as there are convolution blocks

	            // Complex multiply / accumulate with all FFT outputs of nfor last rounds, see below :-) !

	            // inverse FFT of the accumulated complex multiply outputs
	            // IQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQ
	            // <-      2 * cbs.size          -><-      2 * cbs.size          ->
	            // inverse FFT result is in accum

	            // discard first half of inverse FFT output and separate I & Q into separate buffers
	            // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQIQ
	            // <-      2 * cbs.size          -><-      2 * cbs.size          ->
	            //
	            // i_buffer_convolution
	            // IIIIIIIIIIIIIIII
	            // <- cbs.size   ->
	            // --> this is the filtered audio of I

	            // q_buffer_convolution
	            // QQQQQQQQQQQQQQQQ
	            // <- cbs.size   ->
	            // this is the filtered audio of Q

	 /*
	  * partitioned block overlap-and-save algorithm
	  */
	            	int fft_conv_size = cbs.size * 2;
	                int i, j, k;
	                // copy input buffer to right half of FFT input buffer
	                // left half already contains data from last round
	                for(int idx=0; idx < cbs.size; idx++)
	                {
	                	cob.fftin[fft_conv_size + 2 * idx + 0] = cob.i_buffer_convolution[idx];
	                  	cob.fftin[fft_conv_size + 2 * idx + 1] = cob.q_buffer_convolution[idx];
	                }

	                // fftin --> one input buffer
	                // fftout[nfor] --> changing output buffers !
	                // FFT performed on fftin inplace
	                arm_cfft_f32(&arm_cfft_sR_f32_len256, cob.fftin, 0, 1);
	                // copy output from fftin into current fftout buffer
	                for (int idx = 0; idx < fft_conv_size * 2; idx++)
	                {
	                	cob.fftout[cbs.buffidx][idx] = cob.fftin[idx];
	                }
	                //fftw_execute (a->pcfor[a->buffidx]);

	                k = cbs.buffidx;
	                // fill the array accum with zeros
	                for (int idx = 0; idx < fft_conv_size * 2; idx++)
	                {
	                	cob.accum[idx] = 0;
	                }
	                //memset (a->accum, 0, 2 * a->size * sizeof (complex));


	                for (j = 0; j < cbs.nfor; j++)
	                {
	                  for (i = 0; i < fft_conv_size; i++)
	                  {
	                    // this sums up the complex multiply results for real and imaginary components
	                    // stored in accum
	                    cob.accum[2 * i + 0] += cob.fftout[k][2 * i + 0] * cob.fmask[j][2 * i + 0]
				                              - cob.fftout[k][2 * i + 1] * cob.fmask[j][2 * i + 1];
	                    cob.accum[2 * i + 1] += cob.fftout[k][2 * i + 0] * cob.fmask[j][2 * i + 1]
											  + cob.fftout[k][2 * i + 1] * cob.fmask[j][2 * i + 0];
	                  }
	                  // k points to the next relevant fftout result
	                  // it must flip over when 0 is reached
	                  k = k -1;
	                  if(k < 0)
	                  {
	                	  k = cbs.nfor - 1;
	                  }
	                  //k = (k + a->idxmask) & a->idxmask;
	                }
	                cbs.buffidx +=1;
	                if(cbs.buffidx >= cbs.nfor)
	                {
	                	cbs.buffidx = 0;
	                }
	                // a->buffidx = (a->buffidx + 1) & a->idxmask;
	                // inverse FFT
	                // input: accum
	                // audio is in right half of the output buffer accum
	                arm_cfft_f32(&arm_cfft_sR_f32_len256, cob.accum, 1, 1);
	                // fftw_execute (a->crev);
	                // copy FFT input buffer to left half of FFT input buffer for next round
	                // --> overlap 50%
	                for(int idx=0; idx < cbs.size; idx++)
	                {
	                	cob.fftin[2 * idx + 0] = cob.fftin[fft_conv_size + 2 * idx + 0];
	                	cob.fftin[2 * idx + 1] = cob.fftin[fft_conv_size + 2 * idx + 1];
	                }
	                //memcpy (a->fftin, &(a->fftin[2 * a->size]), a->size * sizeof(complex));
	                // copy I & Q inverse FFT results (from second half of accum) to adb.i_buffer_convolution and adb.q_buffer_convolution
	                for(int idx = 0; idx < cbs.size; idx++)
	                {
	                	cob.i_buffer_convolution[idx] = cob.accum[fft_conv_size + 2 * idx + 0];
	                	cob.q_buffer_convolution[idx] = cob.accum[fft_conv_size + 2 * idx + 1];
	                }
	                // these buffers now contain the bandpass filtered audio
	                // which already has suppressed opposite sideband (if cutoff-frequencies have been set correctly for the BP filter)
	                // so no further "demodulation" is necessary for SSB / CW

	////////////////////////////////////////////////////////////////////////////////////////////////////////////



	/* ************************************************
	 *  AGC
	 * ************************************************/
	            // perform AGC on I and Q
	                // we need the stereo version of the AGC
	    AudioDriver_RxAgcWdsp(cbs.size, cob.i_buffer_convolution, cob.q_buffer_convolution);

	            /*
	             *  TODO: deal with Digimodes and FM
	             *
	             */

	/* ************************************************
	 *  DEMODULATION
	 * ************************************************/


	    // for first test, we assume USB demodulation (hard coded filter passband +250Hz to +2700Hz in AudioDriver_SetRxAudioProcessing)
	    // for LSB, you would use -2700Hz to -250Hz

	    // very easy: in SSB, the real part of the (second half of the) iFFT output is the demodulated audio!
	    // switching between sidebands is only done by selecting the passband cutoff frequencies (coefficients) of the bandpass filter

	    // that means, that the cob.i_buffer_convolution already contains the demodulated audio for SSB and CW mode!

	    // i_buffer_convolution
	    // IIIIIIIIIIIIIIII
	    // <- cbs.size   ->
	    // --> this is the demodulated audio of size -> cob.size)
}

#endif

#ifdef USE_CONVOLUTION
//
//*----------------------------------------------------------------------------
//* Function Name       : Convolution-based audio_rx_processor
//* Object              :
//* Object              : audio sample processor based on partitioned block convolution, DD4WH 2018_08_18
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void AudioDriver_RxProcessorConvolution(AudioSample_t * const src, AudioSample_t * const dst, const uint16_t blockSize)
{
    // this is the main RX audio function
	// it is driven with 32 samples in the complex buffer scr, meaning 32 * I AND 32 * Q
	// blockSize is thus 32, DD4WH 2018_02_06

	//	const int16_t blockSizeDecim = blockSize/(int16_t)ads.decimation_rate;
	const int16_t blockSizeDecim = blockSize/(int16_t)cbs.DF;
    // we copy volatile variables which are used multiple times to local consts to let the compiler to its optimization magic
    // since we are in an interrupt, no one will change these anyway
    // shaved off a few bytes of code
    const uint8_t dmod_mode = ts.dmod_mode;
    const uint8_t tx_audio_source = ts.tx_audio_source;
    const uint8_t iq_freq_mode = ts.iq_freq_mode;
    const uint8_t  dsp_active = ts.dsp_active;
#ifdef USE_TWO_CHANNEL_AUDIO
    const bool use_stereo = ((dmod_mode == DEMOD_IQ || dmod_mode == DEMOD_SSBSTEREO || (dmod_mode == DEMOD_SAM && ads.sam_sideband == SAM_SIDEBAND_STEREO)) && ts.stereo_enable);
#endif
    float post_agc_gain_scaling;

    if (tx_audio_source == TX_AUDIO_DIGIQ)
    {

        for(uint32_t i = 0; i < blockSize; i++)
        {
            // 16 bit format - convert to float and increment
            // we collect our I/Q samples for USB transmission if TX_AUDIO_DIGIQ
            audio_in_put_buffer(src[i].l);
            audio_in_put_buffer(src[i].r);
        }
    }

    if (ads.af_disabled == 0 )
    {
        // Split stereo channels
        for(uint32_t i = 0; i < blockSize; i++)
        {
            if(src[i].l > ADC_CLIP_WARN_THRESHOLD/4)            // This is the release threshold for the auto RF gain
            {
                ads.adc_quarter_clip = 1;
                if(src[i].l > ADC_CLIP_WARN_THRESHOLD/2)            // This is the trigger threshold for the auto RF gain
                {
                    ads.adc_half_clip = 1;
                    if(src[i].l > ADC_CLIP_WARN_THRESHOLD)          // This is the threshold for the red clip indicator on S-meter
                    {
                        ads.adc_clip = 1;
                    }
                }
            }
            adb.i_buffer[i] = (float32_t)src[i].l;
            adb.q_buffer[i] = (float32_t)src[i].r;
        }

        // artificial amplitude imbalance for testing of the automatic IQ imbalance correction
        //    arm_scale_f32 (adb.i_buffer, 0.6, adb.i_buffer, blockSize);


        AudioDriver_RxHandleIqCorrection(blockSize);


        // Spectrum display sample collect for magnify == 0
        AudioDriver_SpectrumNoZoomProcessSamples(blockSize);

        if(iq_freq_mode)            // is receive frequency conversion to be done?
        {
            AudioDriver_FreqConversion(adb.i_buffer, adb.q_buffer, blockSize, iq_freq_mode == FREQ_IQ_CONV_P6KHZ || iq_freq_mode == FREQ_IQ_CONV_P12KHZ);
        }

        // Spectrum display sample collect for magnify != 0
        AudioDriver_SpectrumZoomProcessSamples(blockSize);

        //  Demodulation, optimized using fast ARM math functions as much as possible

        bool dvmode_signal = false;

#ifdef USE_FREEDV
        if (ts.dvmode == true && ts.digital_mode == DigitalMode_FreeDV)
        {
            dvmode_signal = AudioDriver_RxProcessorDigital(src, adb.a_buffer[1], blockSize);
        }
#endif

	// Convolution Filtering DD4WH 2018_08_18

	//      1. decimation
	//		2. collect samples until we have 128 samples to deal within the convolution
	//		3. Convolution Filtering
	//		4. AGC working on both I & Q independently
	//		5. demodulation (LSB, USB, AM, SAM)
	//		6. LPC-based noise blanker
	//		7. spectral noise reduction
	//		8. scaling
	//		9. auto-notch LMS filter
	//		10. biquad filter (manual notch, peak, bass adjustment)
	//      11. RTTY/BPSK/CW decoding
	//		12. interpolation
	//		13. biquad treble filter
	//		14. mute/scale for lineout, beep etc.
	//		15. account for 128 output samples
//

        if (dvmode_signal == false)
        {
        	//
        	if(cbs.DF != 1)
        	{
        	//      1. decimation
        	// I & Q are decimated in place the same buffer
        		arm_fir_decimate_f32(&DECIMATE_RX_I, adb.i_buffer, adb.i_buffer, blockSize);
        		arm_fir_decimate_f32(&DECIMATE_RX_Q, adb.q_buffer, adb.q_buffer, blockSize);
        	}
/********************************************************************************************************************/
        	//		2. collect samples until we have 128 samples to deal within the convolution
            // put those samples into a new buffer cob.i_buffer_convolution and cob.q_buffer_convolution

            // when 128 samples are ready, continue HERE:
            // BTW, please use cbs.size instead of hard coded 128, because that could change.
            // TODO: I will set cbs.size in the function AudioDriver_SetRxAudioProcessing

                const uint8_t  dsp_active = ts.dsp_active;
                static int trans_count_in=0;
                static int outbuff_count=0;
                static int Sample_fill_in_pt=0;
                static Sample_Buffer* out_buffer = NULL;
                //
                // attention -> change no of samples according to decimation factor!

                for (int k = 0; k < blockSizeDecim; k++) //transfer our noisy audio to our NR-input buffer
                {
                	// use new buffers for I & Q, because we cannot use FreeDV buffers
                	cob.i_buffer_convolution[Sample_fill_in_pt][trans_count_in] = adb.i_buffer[k];
                	cob.q_buffer_convolution[Sample_fill_in_pt][trans_count_in] = adb.q_buffer[k];

                    //mmb.nr_audio_buff[Sample_fill_in_pt].samples[trans_count_in].real=inout_buffer[k];
                    //mmb.nr_audio_buff[Sample_fill_in_pt].samples[trans_count_in].imag=inout_buffer[k+1];
                    //trans_count_in++;
                    trans_count_in++; // count the samples towards FFT-size
                }

                if (trans_count_in >= (cbs.size * 2)) // FFT size is always (input block size * 2)
                    //FFT_SIZE has to be an integer mult. of blockSizeDecim!!!
                {
                	// I do not understand the next line? Where is that saved?
                	// makes no sense to me to call a function with a return value, where the return value is not used
                    //Sample_in_buffer_add(&mmb.nr_audio_buff[Sample_fill_in_pt]); // save pointer to full buffer
                    Sample_in_buffer_add(&mmb.nr_audio_buff[Sample_fill_in_pt]); // save pointer to full buffer
                    trans_count_in=0;                              // set counter to 0
                    Sample_fill_in_pt++;                               // increase pointer index
                    Sample_fill_in_pt %= SAMPLE_BUFFER_NUM;            // make sure, that index stays in range

                    //at this point we have transfered one complete block of 128 (?) samples to one buffer
                }

                //**********************************************************************************
                //don't worry!  in the mean time the noise reduction routine is (hopefully) doing it's job within ui
                //as soon as "fdv_audio_has_data" we can start harvesting the output
                //**********************************************************************************

                if (out_buffer == NULL && SAMPLE_out_has_data() > 1)
                {
                    Sample_out_buffer_peek(&out_buffer);
                }

                float32_t Sample_dec_buffer[blockSizeDecim];

                if (out_buffer != NULL)  // Convolution-routine has finished it's job
                {
                    for (int j=0; j < blockSizeDecim; j=j+2) // transfer filtered data back to our buffer
                    {
                    	//                        NR_dec_buffer[j]   = out_buffer->samples[outbuff_count+NR_FFT_SIZE].real; //here add the offset in the buffer
                    	//                        NR_dec_buffer[j+1] = out_buffer->samples[outbuff_count+NR_FFT_SIZE].imag; //here add the offset in the buffer
                    	// totally unclear to me, what ->samples means . . .
                    	// why offset?
                    	Sample_dec_buffer[j]   = out_buffer->samples[outbuff_count+NR_FFT_SIZE].real; //here add the offset in the buffer
                    	Sample_dec_buffer[j+1] = out_buffer->samples[outbuff_count+NR_FFT_SIZE].imag; //here add the offset in the buffer
                        outbuff_count++;
                    }

                    if (outbuff_count >= (cbs.size * 2)) // we reached the end of the buffer coming from NR
                    {
                        outbuff_count = 0;
                        Sample_out_buffer_remove(&out_buffer);
                        out_buffer = NULL;
                        Sample_out_buffer_peek(&out_buffer);
                    }
                }

                {
                    arm_copy_f32(NR_dec_buffer, inout_buffer, blockSizeDecim);
                }


/********************************************************************************************************************/

/*
 *
 *  FROM HERE: OLD CODE!!!!
 *  TODO: work on this
 *
 */

        } // end of if (dvmode_signal == false)

    } // end of     if (ads.af_disabled == 0 )

    // interpolation

// TODO: at this point we have 128 real audio samples filtered and AGC�ed in the variable 	 (for mono modes)
    // for stereo modes (not yet implemented), the other channel is in cob.q_buffer_convolution

    // now we have to make blocks of 32 samples out of that for further processing
    // and put these into adb.a_buffer[0] and adb.a_buffer[1]
    // (the latter is only different from adb.a_buffer[0] for stereo modes, not yet implemented)



    bool do_mute_output =
            ts.audio_dac_muting_flag
            || ts.audio_dac_muting_buffer_count > 0
            || (ads.af_disabled)
            || ((dmod_mode == DEMOD_FM) && ads.fm_squelched);
    // this flag is set during rx tx transition, so once this is active we mute our output to the I2S Codec

    if (do_mute_output)
        // fill audio buffers with zeroes if we are to mute the receiver completely while still processing data OR it is in FM and squelched
        // or when filters are switched
    {
        arm_fill_f32(0, adb.a_buffer[0], blockSize);
        arm_fill_f32(0, adb.a_buffer[1], blockSize);
        if (ts.audio_dac_muting_buffer_count > 0)
        {
            ts.audio_dac_muting_buffer_count--;
        }
    }
    else
    {
#ifdef USE_TWO_CHANNEL_AUDIO
        // BOTH CHANNELS "FIXED" GAIN as input for audio amp and headphones/lineout
        // each output path has its own gain control.
    	// Do fixed scaling of audio for LINE OUT
    	arm_scale_f32(adb.a_buffer[1], LINE_OUT_SCALING_FACTOR, adb.a_buffer[1], blockSize);
    	if (use_stereo)
    	{
    		arm_scale_f32(adb.a_buffer[0], LINE_OUT_SCALING_FACTOR, adb.a_buffer[0], blockSize);
    	}
    	else
    	{
    		// we simply copy the data from the other channel
    		arm_copy_f32(adb.a_buffer[1], adb.a_buffer[0], blockSize);
    	}
#else
        // VARIABLE LEVEL FOR SPEAKER
        // LINE OUT (constant level)
    	arm_scale_f32(adb.a_buffer[1], LINE_OUT_SCALING_FACTOR, adb.a_buffer[0], blockSize);       // Do fixed scaling of audio for LINE OUT and copy to "a" buffer in one operation
#endif
    	//
        // AF gain in "ts.audio_gain-active"
        //  0 - 16: via codec command
        // 17 - 20: soft gain after decoder
        //
        AudioDriver_MchfVolumeWorkaround(adb.a_buffer[1], blockSize, ts.rx_gain[RX_AUDIO_SPKR].value, ts.rx_gain[RX_AUDIO_SPKR].active_value, CODEC_SPEAKER_MAX_VOLUME);
    }



    float32_t usb_audio_gain = ts.rx_gain[RX_AUDIO_DIG].value/31.0;

    // Transfer processed audio to DMA buffer
    for(int i=0; i < blockSize; i++)                            // transfer to DMA buffer and do conversion to INT
    {
        // TODO: move to softdds ...
        if((ts.beep_active) && (ads.beep.step))         // is beep active?
        {
            // Yes - Calculate next sample
            // shift accumulator to index sine table
            adb.a_buffer[1][i] += (float32_t)softdds_nextSample(&ads.beep) * ads.beep_loudness_factor; // load indexed sine wave value, adding it to audio, scaling the amplitude and putting it on "b" - speaker (ONLY)
        }
        else                    // beep not active - force reset of accumulator to start at zero to minimize "click" caused by an abrupt voltage transition at startup
        {
            ads.beep.acc = 0;
        }

        if (do_mute_output)
        {
            dst[i].l = 0;
            dst[i].r = 0;
        }
        else
        {
        	dst[i].l = adb.a_buffer[1][i];
        	dst[i].r = adb.a_buffer[0][i];
        }
        // Unless this is DIGITAL I/Q Mode, we sent processed audio
        if (tx_audio_source != TX_AUDIO_DIGIQ)
        {
        	int16_t vals[2];

#ifdef USE_TWO_CHANNEL_AUDIO
        	vals[0] = adb.a_buffer[0][i] * usb_audio_gain;
        	vals[1] = adb.a_buffer[1][i] * usb_audio_gain;
#else
        	vals[0] = vals[1] = adb.a_buffer[0][i] * usb_audio_gain;
#endif

        	audio_in_put_buffer(vals[0]);
            audio_in_put_buffer(vals[1]);
        }

    }

}



#endif

