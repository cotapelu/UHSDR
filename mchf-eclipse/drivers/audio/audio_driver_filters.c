/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     audio_driver_filters.c                                          **
 **  Description:   Biquad filter coefficient calculations extracted from audio_driver.c **
 **  Last Modified:                                                                 **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

#include "audio_driver_filters.h"
#include "audio_driver.h"

/**
 * @brief Biquad Filter Init Helper function which copies the biquad coefficients into the filter array itself
 */
void AudioDriver_SetBiquadCoeffs(float32_t* coeffsTo,const float32_t* coeffsFrom)
{
    coeffsTo[0] = coeffsFrom[0];
    coeffsTo[1] = coeffsFrom[1];
    coeffsTo[2] = coeffsFrom[2];
    coeffsTo[3] = coeffsFrom[3];
    coeffsTo[4] = coeffsFrom[4];
}

/**
 * @brief Biquad Filter Init Helper function which copies the biquad coefficients into all filter instances of the 1 or 2 channel audio
 * @param biquad_inst_array a pointer to an either 1 or 2 element sized array of biquad filter instances. Make sure the array has the expected size!
 * @param idx of first element of stage coefficients (i.e. 0, 5, 10, ... ) since we have 5 element coeff arrays per stage
 */
void AudioDriver_SetBiquadCoeffsAllInstances(arm_biquad_casd_df1_inst_f32 biquad_inst_array[NUM_AUDIO_CHANNELS], uint32_t idx, const float32_t* coeffsFrom)
{
    for (int chan = 0; chan < NUM_AUDIO_CHANNELS; chan++)
     {
         AudioDriver_SetBiquadCoeffs(&biquad_inst_array[chan].pCoeffs[idx],coeffsFrom);
     }
}

/**
 * @brief Biquad Filter Init Helper function which applies the filter specific scaling to calculated coefficients
 */
void AudioDriver_ScaleBiquadCoeffs(float32_t coeffs[5],const float32_t scalingA, const float32_t scalingB)
{
    coeffs[A1] = coeffs[A1] / scalingA;
    coeffs[A2] = coeffs[A2] / scalingA;

    coeffs[B0] = coeffs[B0] / scalingB;
    coeffs[B1] = coeffs[B1] / scalingB;
    coeffs[B2] = coeffs[B2] / scalingB;
}

/**
 * @brief Biquad Filter Init Helper function to calculate a notch filter aka narrow bandstop filter
 */
void AudioDriver_CalcBandstop(float32_t coeffs[5], float32_t f0, float32_t FS)
{
     float32_t Q = 10; // larger Q gives narrower notch
     float32_t w0 = 2 * PI * f0 / FS;
     float32_t alpha = sinf(w0) / (2 * Q);

     coeffs[B0] = 1;
     coeffs[B1] = - 2 * cosf(w0);
     coeffs[B2] = 1;
     float32_t scaling = 1 + alpha;
     coeffs[A1] = 2 * cosf(w0); // already negated!
     coeffs[A2] = alpha - 1; // already negated!

     AudioDriver_ScaleBiquadCoeffs(coeffs,scaling, scaling);
}

/**
 * @brief Biquad Filter Init Helper function to calculate a peak filter aka a narrow bandpass filter
 */
void AudioDriver_CalcBandpass(float32_t coeffs[5], float32_t f0, float32_t FS)
{
    /*       // peak filter = peaking EQ
    f0 = ts.dsp.peak_frequency;
    //Q = 15; //
    // bandwidth in octaves between midpoint (Gain / 2) gain frequencies
    float32_t BW = 0.05;
    w0 = 2 * PI * f0 / FSdec;
    //alpha = sin(w0) / (2 * Q);
    alpha = sin (w0) * sinh( log(2) / 2 * BW * w0 / sin(w0) );
    float32_t Gain = 12;
    A = powf(10.0, (Gain/40.0));
    coeffs[B0] = 1 + (alpha * A);
    coeffs[B1] = - 2 * cos(w0);
    coeffs[B2] = 1 - (alpha * A);
    a0 = 1 + (alpha / A);
    coeffs[A1] = 2 * cos(w0); // already negated!
    coeffs[A2] = (alpha/A) - 1; // already negated!
     */
    /*        // test the BPF coefficients, because actually we want a "peak" filter without gain!
    // Bandpass filter constant 0dB peak gain
    // this filter was tested: "should have more gain and less Q"
    f0 = ts.dsp.peak_frequency;
    Q = 20; //
    w0 = 2 * PI * f0 / FSdec;
    alpha = sinf(w0) / (2 * Q);
//        A = 1; // gain = 1
    //        A = 3; // 10^(10/40); 15dB gain

    coeffs[B0] = alpha;
    coeffs[B1] = 0;
    coeffs[B2] = - alpha;
    a0 = 1 + alpha;
    coeffs[A1] = 2 * cosf(w0); // already negated!
    coeffs[A2] = alpha - 1; // already negated!
     */
    // BPF: constant skirt gain, peak gain = Q
    float32_t Q = 4; //
    float32_t BW = 0.03;
    float32_t w0 = 2 * PI * f0 / FS;
    float32_t alpha = sinf (w0) * sinhf( log(2) / 2 * BW * w0 / sinf(w0) ); //

    coeffs[B0] = Q * alpha;
    coeffs[B1] = 0;
    coeffs[B2] = - Q * alpha;
    float32_t scaling = 1 + alpha;
    coeffs[A1] = 2 * cosf(w0); // already negated!
    coeffs[A2] = alpha - 1; // already negated!

    AudioDriver_ScaleBiquadCoeffs(coeffs,scaling, scaling);

}

/**
 * @brief Biquad Filter Init Helper function to calculate a treble adjustment filter aka high shelf filter
 */
void AudioDriver_CalcHighShelf(float32_t coeffs[5], float32_t f0, float32_t S, float32_t gain, float32_t FS)
{
    float32_t w0 = 2 * PI * f0 / FS;
    float32_t A = pow10f(gain/40.0); // gain ranges from -20 to 5
    float32_t alpha = sinf(w0) / 2 * sqrtf( (A + 1/A) * (1/S - 1) + 2 );
    float32_t cosw0 = cosf(w0);
    float32_t twoAa = 2 * sqrtf(A) * alpha;
    // highShelf
    //
    coeffs[B0] = A *        ( (A + 1) + (A - 1) * cosw0 + twoAa );
    coeffs[B1] = - 2 * A *  ( (A - 1) + (A + 1) * cosw0         );
    coeffs[B2] = A *        ( (A + 1) + (A - 1) * cosw0 - twoAa );
    float32_t scaling =       (A + 1) - (A - 1) * cosw0 + twoAa ;
    coeffs[A1] = - 2 *      ( (A - 1) - (A + 1) * cosw0         ); // already negated!
    coeffs[A2] = twoAa      - (A + 1) + (A - 1) * cosw0; // already negated!


    //    DCgain = 2; //
    //    DCgain = (coeffs[B0] + coeffs[B1] + coeffs[B2]) / (1 - (- coeffs[A1] - coeffs[A2])); // takes into account that coeffs[A1] and coeffs[A2] are already negated!
    float32_t DCgain = 1.0 * scaling;

    AudioDriver_ScaleBiquadCoeffs(coeffs,scaling, DCgain);
}

/**
 * @brief Biquad Filter Init Helper function to calculate a bass adjustment filter aka low shelf filter
 */
void AudioDriver_CalcLowShelf(float32_t coeffs[5], float32_t f0, float32_t S, float32_t gain, float32_t FS)
{

    float32_t w0 = 2 * PI * f0 / FS;
    float32_t A = pow10f(gain/40.0); // gain ranges from -20 to 5

    float32_t alpha = sinf(w0) / 2 * sqrtf( (A + 1/A) * (1/S - 1) + 2 );
    float32_t cosw0 = cosf(w0);
    float32_t twoAa = 2 * sqrtf(A) * alpha;

    // lowShelf
    coeffs[B0] = A *        ( (A + 1) - (A - 1) * cosw0 + twoAa );
    coeffs[B1] = 2 * A *    ( (A - 1) - (A + 1) * cosw0         );
    coeffs[B2] = A *        ( (A + 1) - (A - 1) * cosw0 - twoAa );
    float32_t scaling =       (A + 1) + (A - 1) * cosw0 + twoAa ;
    coeffs[A1] = 2 *        ( (A - 1) + (A + 1) * cosw0         ); // already negated!
    coeffs[A2] = twoAa      - (A + 1) - (A - 1) * cosw0; // already negated!

    // scaling the feedforward coefficients for gain adjustment !
    // "DC gain of an IIR filter is the sum of the filters� feedforward coeffs divided by
    // 1 minus the sum of the filters� feedback coeffs" (Lyons 2011)
    //    float32_t DCgain = (coeffs[B0] + coeffs[B1] + coeffs[B2]) / (1 - (coeffs[A1] + coeffs[A2]));
    // does not work for some reason?
    // I take a divide by a constant instead !
    //    DCgain = (coeffs[B0] + coeffs[B1] + coeffs[B2]) / (1 - (- coeffs[A1] - coeffs[A2])); // takes into account that coeffs[A1] and coeffs[A2] are already negated!

    float32_t DCgain = 1.0 * scaling; //


    AudioDriver_ScaleBiquadCoeffs(coeffs,scaling, DCgain);

}
