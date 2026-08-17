/* Standalone version of audio_driver_filters.c for host unit testing */
#include <stdint.h>
#include <math.h>
#include <stdio.h>

#ifndef PI
#define PI 3.14159265358979323846f
#endif

typedef float float32_t;

typedef struct {
    uint32_t numStages;
    float32_t *pCoeffs;
    float32_t *pState;
} arm_biquad_casd_df1_inst_f32;

#define NUM_AUDIO_CHANNELS 1
#define B0 0
#define B1 1
#define B2 2
#define A1 3
#define A2 4

void AudioDriver_SetBiquadCoeffs(float32_t* coeffsTo,const float32_t* coeffsFrom)
{
    coeffsTo[0] = coeffsFrom[0];
    coeffsTo[1] = coeffsFrom[1];
    coeffsTo[2] = coeffsFrom[2];
    coeffsTo[3] = coeffsFrom[3];
    coeffsTo[4] = coeffsFrom[4];
}

void AudioDriver_SetBiquadCoeffsAllInstances(arm_biquad_casd_df1_inst_f32 biquad_inst_array[NUM_AUDIO_CHANNELS], uint32_t idx, const float32_t* coeffsFrom)
{
    AudioDriver_SetBiquadCoeffs(&biquad_inst_array[idx].pCoeffs[0], coeffsFrom);
}

void AudioDriver_ScaleBiquadCoeffs(float32_t coeffs[5], const float32_t scalingA, const float32_t scalingB)
{
    coeffs[A1] /= scalingA;
    coeffs[A2] /= scalingA;
    coeffs[B0] /= scalingB;
    coeffs[B1] /= scalingB;
    coeffs[B2] /= scalingB;
}

void AudioDriver_CalcBandstop(float32_t coeffs[5], float32_t f0, float32_t FS)
{
    float32_t w0 = 2 * PI * f0 / FS;
    float32_t Q = 1.0f / (2.0f * 0.707f); /* BW = 1 octave */
    float32_t alpha = sinf(w0) / (2 * Q);

    coeffs[B0] = 1;
    coeffs[B1] = - 2 * cosf(w0);
    coeffs[B2] = 1;
    coeffs[A1] = -2 * cosf(w0);
    coeffs[A2] = 1 - alpha;

    AudioDriver_ScaleBiquadCoeffs(coeffs, (1 - alpha), (1 - alpha));
}

void AudioDriver_CalcBandpass(float32_t coeffs[5], float32_t f0, float32_t FS)
{
    float32_t BW = 2.0f;
    float32_t w0 = 2 * PI * f0 / FS;
    float32_t alpha = sinf (w0) * sinhf( log(2) / 2 * BW * w0 / sinf(w0) ); //

    coeffs[B0] = alpha;
    coeffs[B1] = 0;
    coeffs[B2] = -alpha;
    coeffs[A1] = 2 * cosf(w0); // already negated!
    coeffs[A2] = 1 - alpha;
}

void AudioDriver_CalcHighShelf(float32_t coeffs[5], float32_t f0, float32_t S, float32_t gain, float32_t FS)
{
    float32_t w0 = 2 * PI * f0 / FS;
    float32_t A = powf(10.0f, gain/40.0); // gain ranges from -20 to 5
    float32_t alpha = sinf(w0) / 2 * sqrtf( (A + 1/A) * (1/S - 1) + 2 );
    float32_t cosw0 = cosf(w0);

    coeffs[B0] =      A * ( (A + 1) + (A - 1) * cosw0 + 2 * sqrtf(A) * alpha );
    coeffs[B1] =    - 2 * A * ( (A - 1) + (A + 1) * cosw0 );
    coeffs[B2] =      A * ( (A + 1) + (A - 1) * cosw0 - 2 * sqrtf(A) * alpha );
    coeffs[A1] =       2 * ( (A - 1) - (A + 1) * cosw0 );
    coeffs[A2] =      - (A + 1) + (A - 1) * cosw0 + 2 * sqrtf(A) * alpha;

    AudioDriver_ScaleBiquadCoeffs(coeffs, ( (A + 1) - (A - 1) * cosw0 + 2 * sqrtf(A) * alpha ), ( (A + 1) + (A - 1) * cosw0 + 2 * sqrtf(A) * alpha ) );
}

void AudioDriver_CalcLowShelf(float32_t coeffs[5], float32_t f0, float32_t S, float32_t gain, float32_t FS)
{
    float32_t w0 = 2 * PI * f0 / FS;
    float32_t A = powf(10.0f, gain/40.0); // gain ranges from -20 to 5
    float32_t alpha = sinf(w0) / 2 * sqrtf( (A + 1/A) * (1/S - 1) + 2 );
    float32_t cosw0 = cosf(w0);

    coeffs[B0] =      A * ( (A + 1) - (A - 1) * cosw0 + 2 * sqrtf(A) * alpha );
    coeffs[B1] =   2 * A * ( (A - 1) - (A + 1) * cosw0 );
    coeffs[B2] =      A * ( (A + 1) - (A - 1) * cosw0 - 2 * sqrtf(A) * alpha );
    coeffs[A1] =    - 2 * ( (A - 1) + (A + 1) * cosw0 );
    coeffs[A2] =      - (A + 1) + (A - 1) * cosw0 - 2 * sqrtf(A) * alpha;

    AudioDriver_ScaleBiquadCoeffs(coeffs, ( (A + 1) + (A - 1) * cosw0 + 2 * sqrtf(A) * alpha ), ( (A + 1) - (A - 1) * cosw0 + 2 * sqrtf(A) * alpha ) );
}
