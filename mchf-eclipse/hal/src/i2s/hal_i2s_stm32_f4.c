/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     hal_i2s_stm32.c                                                 **
 **  Description:   STM32 backend for hal_i2s.h                                     **
 **                 ONLY this file includes vendor HAL I2S/SAI headers.             **
 **  Last Modified: 2026-08-21                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "hal_i2s.h"

#include "uhsdr_board_config.h"
#include "uhsdr_board.h"
#include "profiling.h"

#include "audio_driver.h"

/* -------------------------------------------------------------------------
 * Local type definitions
 * ------------------------------------------------------------------------- */
typedef struct {
    void (*start)(void);
    void (*stop)(void);
    void (*clear_tx)(void);
    void (*set_bit_width)(void);
} audio_if_t;

#include "stm32f4xx_hal.h"
#include "i2s.h"

/* -------------------------------------------------------------------------
 * DMA buffer
 * ------------------------------------------------------------------------- */
typedef struct
{
    IqSample_t out[2*IQ_BLOCK_SIZE];
    IqSample_t in[2*IQ_BLOCK_SIZE];
} dma_iq_buffer_t;

typedef struct
{
    AudioSample_t out[2*AUDIO_BLOCK_SIZE];
    AudioSample_t in[2*AUDIO_BLOCK_SIZE];
} dma_audio_buffer_t;

typedef
#if CODEC_NUM == 1
    union
#else
    struct
#endif
    {
        dma_iq_buffer_t iq_buf;
        dma_audio_buffer_t audio_buf;
    } I2S_DmaBuffers_t;

static __UHSDR_DMAMEM I2S_DmaBuffers_t dma;

/* -------------------------------------------------------------------------
 * Internal helpers
 * ------------------------------------------------------------------------- */
static void UhsdrHwI2s_Codec_ClearTxDmaBuffer(void)
{
    memset((void*)&dma.iq_buf.out, 0, sizeof(dma.iq_buf.out));
}

static void UhsdrHwI2s_SetBitWidth(void);

/* -------------------------------------------------------------------------
 * Audio interface vtable (I2S on F4, SAI on F7/H7)
 * ------------------------------------------------------------------------- */
static void audio_if_start(void)
{
    UhsdrHwI2s_SetBitWidth();
    uint32_t dma_size = sizeof(dma.iq_buf.in);
    dma_size /= sizeof(dma.iq_buf.in[0].l);
    HAL_I2SEx_TransmitReceive_DMA(&hi2s3, (uint16_t*)dma.iq_buf.out, (uint16_t*)dma.iq_buf.in, dma_size);
}

static void audio_if_stop(void)
{
    HAL_I2S_DMAStop(&hi2s3);
}

static void audio_if_clear_tx(void)
{
    UhsdrHwI2s_Codec_ClearTxDmaBuffer();
}

static void audio_if_set_bit_width(void)
{
    UhsdrHwI2s_SetBitWidth();
}

static const audio_if_t audio_if = {
    .start = audio_if_start,
    .stop = audio_if_stop,
    .clear_tx = audio_if_clear_tx,
    .set_bit_width = audio_if_set_bit_width,
};


/* -------------------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------------------- */
static void UhsdrHwI2s_Codec_StartDMA(void)
{
    audio_if.start();
}

static void UhsdrHwI2s_Codec_StopDMA(void)
{
    audio_if.stop();
}

/* -------------------------------------------------------------------------
 * HAL abstract API implementation
 * ------------------------------------------------------------------------- */
hal_status_t hal_i2s_codec_start_dma(void)
{
    UhsdrHwI2s_Codec_StartDMA();
    return HAL_STATUS_OK;
}

hal_status_t hal_i2s_codec_stop_dma(void)
{
    UhsdrHwI2s_Codec_StopDMA();
    return HAL_STATUS_OK;
}

hal_status_t hal_i2s_codec_clear_tx_dma_buffer(void)
{
    UhsdrHwI2s_Codec_ClearTxDmaBuffer();
    return HAL_STATUS_OK;
}

/* -------------------------------------------------------------------------
 * DMA callback handler
 * ------------------------------------------------------------------------- */
static void MchfHw_Codec_HandleBlock(uint16_t which)
{
#ifdef PROFILE_EVENTS
    profileCycleCount_stop();
    profileTimedEventStart(ProfileAudioInterrupt);
#endif

#ifdef EXEC_PROFILING
    GPIOE->BSRRL = GPIO_Pin_10;
#endif

    ts.audio_int_counter++;

    const size_t sz = IQ_BLOCK_SIZE;
    const uint16_t offset = which == 0 ? sz : 0;

    AudioSample_t *audio;
    IqSample_t    *iq;

    if (ts.txrx_mode != TRX_MODE_TX)
    {
        iq = &dma.iq_buf.in[offset];
        audio = &dma.audio_buf.out[offset];
    }
    else
    {
        audio = &dma.audio_buf.in[offset];
        iq = &dma.iq_buf.out[offset];
    }

    AudioSample_t *audioDst = &dma.audio_buf.out[offset];

    AudioDriver_I2SCallback(audio, iq, audioDst, sz);

#ifdef EXEC_PROFILING
    GPIOE->BSRRH = GPIO_Pin_10;
#endif
#ifdef PROFILE_EVENTS
    profileCycleCount_stop();
    profileTimedEventStop(ProfileAudioInterrupt);
#endif
}

void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    MchfHw_Codec_HandleBlock(0);
}

void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    MchfHw_Codec_HandleBlock(1);
}


/* -------------------------------------------------------------------------
 * Bit width configuration
 * ------------------------------------------------------------------------- */


static void UhsdrHwI2s_SetBitWidth()
{
#if defined(USE_32_IQ_BITS)
    hi2s3.Init.DataFormat = I2S_DATAFORMAT_32B;
    HAL_I2S_Init(&hi2s3);
#endif

#if defined(USE_32_AUDIO_BITS)
#endif
}
