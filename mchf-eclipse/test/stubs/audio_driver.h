#ifndef __AUDIO_DRIVER_H
#define __AUDIO_DRIVER_H
#include <stdint.h>
#define NUM_AUDIO_CHANNELS 1
void AudioDriver_SetBiquadCoeffs(float32_t* coeffsTo, const float32_t* coeffsFrom);
void AudioDriver_SetBiquadCoeffsAllInstances(arm_biquad_casd_df1_inst_f32 biquad_inst_array[NUM_AUDIO_CHANNELS], uint32_t idx, const float32_t* coeffsFrom);
void AudioDriver_ScaleBiquadCoeffs(float32_t coeffs[5], const float32_t scalingA, const float32_t scalingB);
void AudioDriver_CalcBandstop(float32_t coeffs[5], float32_t f0, float32_t FS);
void AudioDriver_CalcBandpass(float32_t coeffs[5], float32_t f0, float32_t FS);
void AudioDriver_CalcHighShelf(float32_t coeffs[5], float32_t f0, float32_t S, float32_t gain, float32_t FS);
void AudioDriver_CalcLowShelf(float32_t coeffs[5], float32_t f0, float32_t S, float32_t gain, float32_t FS);
#endif
