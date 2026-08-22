/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------** **                                                                                 **
 **  File name:       audio_driver_ctx.h                                            **
 **  Description:     Internal audio driver context (T31.2 split)                  **
 **  Last Modified:   2026-08-22                                                     **
 **  Licence:         GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __AUDIO_DRIVER_CTX_H
#define __AUDIO_DRIVER_CTX_H

#include "audio_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Context struct and macros - shared between audio_driver.c and audio_rx.c */

#define NR_INTERPOLATE_NO_TAPS 40
#define IIR_RX_STATE_ARRAY_SIZE    (IIR_RXAUDIO_BLOCK_SIZE + IIR_RXAUDIO_NUM_STAGES_MAX)
#define AUDIO_DRIVER_FIR_RX_HILBERT_STATE_SIZE (IQ_RX_NUM_TAPS_MAX + IQ_RX_BLOCK_SIZE)

typedef struct AudioDriverContext
{
    // FIR Decimate instances
    arm_fir_decimate_instance_f32 decimate_zoom_fft_i;
    arm_fir_decimate_instance_f32 decimate_zoom_fft_q;
    arm_fir_decimate_instance_f32 decimate_nr;
    
    // FIR Interpolate instances
    arm_fir_interpolate_instance_f32 interpolate_rx[NUM_AUDIO_CHANNELS];
    arm_fir_interpolate_instance_f32 interpolate_nr;
    
    // FIR state arrays
    float32_t decim_zoom_fft_i_state[FIR_RXAUDIO_BLOCK_SIZE + FIR_RXAUDIO_NUM_TAPS];
    float32_t decim_zoom_fft_q_state[FIR_RXAUDIO_BLOCK_SIZE + FIR_RXAUDIO_NUM_TAPS];
    float32_t interp_rx_state[NUM_AUDIO_CHANNELS][FIR_RXAUDIO_BLOCK_SIZE + FIR_RXAUDIO_NUM_TAPS];
    float32_t decim_nr_state[FIR_RXAUDIO_BLOCK_SIZE + 4];
    float32_t interp_nr_state[FIR_RXAUDIO_BLOCK_SIZE + NR_INTERPOLATE_NO_TAPS];
    
    // IIR Lattice instances
    arm_iir_lattice_instance_f32 iir_pre_filter[NUM_AUDIO_CHANNELS];
    arm_iir_lattice_instance_f32 iir_anti_alias[NUM_AUDIO_CHANNELS];
    arm_iir_lattice_instance_f32 iir_squelch_hpf;
    
    // IIR state arrays
    float32_t iir_rx_state[NUM_AUDIO_CHANNELS][IIR_RX_STATE_ARRAY_SIZE];
    float32_t iir_aa_state[NUM_AUDIO_CHANNELS][IIR_RX_STATE_ARRAY_SIZE];
    float32_t iir_squelch_rx_state[IIR_RX_STATE_ARRAY_SIZE];
    
    // Biquad instances
    arm_biquad_casd_df1_inst_f32 iir_biquad_1[NUM_AUDIO_CHANNELS];
    arm_biquad_casd_df1_inst_f32 iir_biquad_2[NUM_AUDIO_CHANNELS];
    arm_biquad_casd_df1_inst_f32 iir_biquad_zoom_fft_i;
    arm_biquad_casd_df1_inst_f32 iir_biquad_zoom_fft_q;
    arm_biquad_casd_df1_inst_f32 iir_biquad_freedv_i;
    arm_biquad_casd_df1_inst_f32 iir_biquad_freedv_q;
    
    // Biquad state arrays
    float32_t iir_biquad_1_state[NUM_AUDIO_CHANNELS][4 * 4];
    float32_t iir_biquad_2_state[NUM_AUDIO_CHANNELS][4 * 4];
    float32_t iir_biquad_zoom_fft_i_state[4 * 4];
    float32_t iir_biquad_zoom_fft_q_state[4 * 4];
    float32_t iir_biquad_freedv_i_state[2 * 4];
    float32_t iir_biquad_freedv_q_state[2 * 4];
    
    // FreeDV FIR
    arm_fir_instance_f32 fir_freedv_rx_hilbert_i;
    arm_fir_instance_f32 fir_freedv_rx_hilbert_q;
    float32_t fir_freedv_rx_hilbert_state_i[AUDIO_DRIVER_FIR_RX_HILBERT_STATE_SIZE];
    float32_t fir_freedv_rx_hilbert_state_q[AUDIO_DRIVER_FIR_RX_HILBERT_STATE_SIZE];
    
    // Coefficient arrays
    float32_t nr_decimate_coeffs[4];
    float32_t nr_interpolate_coeffs[NR_INTERPOLATE_NO_TAPS];
    float32_t biquad_passthrough[5];
    
    // Coefficient pointer arrays
    float32_t* mag_coeffs[MAGNIFY_NUM];
    float32_t* freedv_coeffs[1];
} AudioDriver_Context_t;

extern __MCHF_SPECIALMEM AudioDriver_Context_t g_audio_driver_ctx;

// Backward-compatible aliases for internal use
#define DECIMATE_ZOOM_FFT_I g_audio_driver_ctx.decimate_zoom_fft_i
#define DECIMATE_ZOOM_FFT_Q g_audio_driver_ctx.decimate_zoom_fft_q
#define INTERPOLATE_RX g_audio_driver_ctx.interpolate_rx
#define DECIMATE_NR g_audio_driver_ctx.decimate_nr
#define INTERPOLATE_NR g_audio_driver_ctx.interpolate_nr
#define IIR_PreFilter g_audio_driver_ctx.iir_pre_filter
#define IIR_AntiAlias g_audio_driver_ctx.iir_anti_alias
#define IIR_biquad_1 g_audio_driver_ctx.iir_biquad_1
#define IIR_biquad_2 g_audio_driver_ctx.iir_biquad_2
#define IIR_biquad_Zoom_FFT_I g_audio_driver_ctx.iir_biquad_zoom_fft_i
#define IIR_biquad_Zoom_FFT_Q g_audio_driver_ctx.iir_biquad_zoom_fft_q
#define IIR_biquad_FreeDV_I g_audio_driver_ctx.iir_biquad_freedv_i
#define IIR_biquad_FreeDV_Q g_audio_driver_ctx.iir_biquad_freedv_q
#define IIR_Squelch_HPF g_audio_driver_ctx.iir_squelch_hpf
#define Fir_FreeDV_Rx_Hilbert_I g_audio_driver_ctx.fir_freedv_rx_hilbert_i
#define Fir_FreeDV_Rx_Hilbert_Q g_audio_driver_ctx.fir_freedv_rx_hilbert_q
#define NR_decimate_coeffs g_audio_driver_ctx.nr_decimate_coeffs
#define NR_interpolate_coeffs g_audio_driver_ctx.nr_interpolate_coeffs
#define mag_coeffs g_audio_driver_ctx.mag_coeffs
#define FreeDV_coeffs g_audio_driver_ctx.freedv_coeffs
#define biquad_passthrough g_audio_driver_ctx.biquad_passthrough
#define decimZoomFFTIState g_audio_driver_ctx.decim_zoom_fft_i_state
#define decimZoomFFTQState g_audio_driver_ctx.decim_zoom_fft_q_state
#define interpState g_audio_driver_ctx.interp_rx_state
#define decimNRState g_audio_driver_ctx.decim_nr_state
#define interplNRState g_audio_driver_ctx.interp_nr_state
#define iir_rx_state g_audio_driver_ctx.iir_rx_state
#define iir_aa_state g_audio_driver_ctx.iir_aa_state
#define iir_squelch_rx_state g_audio_driver_ctx.iir_squelch_rx_state
#define Fir_FreeDV_Rx_Hilbert_State_I g_audio_driver_ctx.fir_freedv_rx_hilbert_state_i
#define Fir_FreeDV_Rx_Hilbert_State_Q g_audio_driver_ctx.fir_freedv_rx_hilbert_state_q

#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_DRIVER_CTX_H */
