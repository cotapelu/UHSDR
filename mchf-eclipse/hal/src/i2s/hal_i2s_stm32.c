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

#if defined(STM32F407xx)
#include "stm32f4xx_hal.h"
#include "i2s.h"
#elif defined(STM32F767xx)
#include "stm32f7xx_hal.h"
#include "sai.h"
#elif defined(STM32H743xx)
#include "sai.h"
#endif

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
void UhsdrHwI2s_Codec_ClearTxDmaBuffer(void)
{
    memset((void*)&dma.iq_buf.out, 0, sizeof(dma.iq_buf.out));
}

static void UhsdrHwI2s_SetBitWidth(void);

/* -------------------------------------------------------------------------
 * Audio interface vtable (I2S on F4, SAI on F7/H7)
 * ------------------------------------------------------------------------- */
#if defined(STM32F407xx)
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
#elif defined(STM32F767xx) || defined(STM32H743xx)
static void audio_if_start(void)
{
    UhsdrHwI2s_SetBitWidth();
    memset((void*)&dma.audio_buf, 0, sizeof(dma.audio_buf));
    memset((void*)&dma.iq_buf, 0, sizeof(dma.iq_buf));

    uint32_t audio_in_size = sizeof(dma.audio_buf.in);
    audio_in_size /= sizeof(dma.audio_buf.in[0].l);
    uint32_t audio_out_size = sizeof(dma.audio_buf.out);
    audio_out_size /= sizeof(dma.audio_buf.out[0].l);
    uint32_t iq_in_size = sizeof(dma.iq_buf.in);
    iq_in_size /= sizeof(dma.iq_buf.in[0].l);
    uint32_t iq_out_size = sizeof(dma.iq_buf.out);
    iq_out_size /= sizeof(dma.iq_buf.out[0].l);

    HAL_SAI_Receive_DMA(&hsai_BlockA1, (uint8_t*)dma.audio_buf.in, audio_in_size);
    HAL_SAI_Transmit_DMA(&hsai_BlockB1, (uint8_t*)dma.audio_buf.out, audio_out_size);

    HAL_SAI_Receive_DMA(&hsai_BlockA2, (uint8_t*)dma.iq_buf.in, iq_in_size);
    HAL_SAI_Transmit_DMA(&hsai_BlockB2, (uint8_t*)dma.iq_buf.out, iq_out_size);
}

static void audio_if_stop(void)
{
    HAL_SAI_DMAStop(&hsai_BlockA1);
    HAL_SAI_DMAStop(&hsai_BlockB1);
    HAL_SAI_DMAStop(&hsai_BlockA2);
    HAL_SAI_DMAStop(&hsai_BlockB2);
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
#endif

/* -------------------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------------------- */
void UhsdrHwI2s_Codec_StartDMA(void)
{
    audio_if.start();
}

void UhsdrHwI2s_Codec_StopDMA(void)
{
    audio_if.stop();
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

#if defined(STM32F407xx)
void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    MchfHw_Codec_HandleBlock(0);
}

void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    MchfHw_Codec_HandleBlock(1);
}
#elif defined(STM32F767xx) || defined(STM32H743xx)
void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hi2s)
{
    if (hi2s == &hsai_BlockA2)
    {
        MchfHw_Codec_HandleBlock(0);
    }
}

void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hi2s)
{
    if (hi2s == &hsai_BlockA2)
    {
        MchfHw_Codec_HandleBlock(1);
    }
}
#endif

/* -------------------------------------------------------------------------
 * Bit width configuration
 * ------------------------------------------------------------------------- */
#if defined(STM32F767xx) || defined(STM32H743xx)
static void UhsdrHWI2s_Sai32Bits(SAI_HandleTypeDef* hsai)
{
    hsai->hdmarx->Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hsai->hdmarx->Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    HAL_DMA_Init(hsai->hdmarx);

    HAL_SAI_InitProtocol(hsai, SAI_I2S_STANDARD, SAI_PROTOCOL_DATASIZE_32BIT, 2);
}
#endif

static void UhsdrHwI2s_SetBitWidth()
{
#if defined(USE_32_IQ_BITS)
    #if defined(STM32F407xx)
    hi2s3.Init.DataFormat = I2S_DATAFORMAT_32B;
    HAL_I2S_Init(&hi2s3);
    #endif
    #if defined(STM32F767xx) || defined(STM32H743xx)
    UhsdrHWI2s_Sai32Bits(&hsai_BlockA2);
    UhsdrHWI2s_Sai32Bits(&hsai_BlockB2);
    #endif
#endif

#if defined(USE_32_AUDIO_BITS)
    #if defined(STM32F767xx) || defined(STM32H743xx)
    UhsdrHWI2s_Sai32Bits(&hsai_BlockA1);
    UhsdrHWI2s_Sai32Bits(&hsai_BlockB1);
    #endif
#endif
}
