#ifndef __AUDIO_DRIVER_FILTERS_STANDALONE_H
#define __AUDIO_DRIVER_FILTERS_STANDALONE_H

#include <stdint.h>

typedef float float32_t;

#define B0 0
#define B1 1
#define B2 2
#define A1 3
#define A2 4

void AudioDriver_SetBiquadCoeffs(float32_t* coeffsTo, const float32_t* coeffsFrom);
void AudioDriver_SetBiquadCoeffsAllInstances(void* biquad_inst_array, uint32_t idx, const float32_t* coeffsFrom);
void AudioDriver_ScaleBiquadCoeffs(float32_t coeffs[5], const float32_t scalingA, const float32_t scalingB);
void AudioDriver_CalcBandstop(float32_t coeffs[5], float32_t f0, float32_t FS);
void AudioDriver_CalcBandpass(float32_t coeffs[5], float32_t f0, float32_t FS);
void AudioDriver_CalcHighShelf(float32_t coeffs[5], float32_t f0, float32_t S, float32_t gain, float32_t FS);
void AudioDriver_CalcLowShelf(float32_t coeffs[5], float32_t f0, float32_t S, float32_t gain, float32_t FS);

#endif
