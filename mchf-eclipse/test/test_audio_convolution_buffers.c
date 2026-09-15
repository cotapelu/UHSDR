/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_audio_convolution_buffers.c                                **
 **  Description:   Host unit tests for audio_convolution ring buffer operations    **
 **                 (Sample_in/Sample_out buffer management)                       **
 **  Last Modified: 2026-08-29                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "test.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Test configuration
 * ------------------------------------------------------------------------- */
#define SAMPLE_BUFFER_FIFO_SIZE 8

/* -------------------------------------------------------------------------
 * Minimal Sample_Buffer placeholder
 * ------------------------------------------------------------------------- */
typedef struct {
    uint32_t id;
} Sample_Buffer;

/* -------------------------------------------------------------------------
 * Local ring buffer state
 * ------------------------------------------------------------------------- */
static int32_t g_in_head = 0;
static int32_t g_in_tail = 0;
static Sample_Buffer* g_in_buffers[SAMPLE_BUFFER_FIFO_SIZE];

static int32_t g_out_head = 0;
static int32_t g_out_tail = 0;
static Sample_Buffer* g_out_buffers[SAMPLE_BUFFER_FIFO_SIZE];

/* -------------------------------------------------------------------------
 * Reimplemented buffer operations (from audio_convolution.c)
 * ------------------------------------------------------------------------- */
static void buffer_reset(int32_t* head, int32_t* tail)
{
    *tail = *head;
}

static int8_t buffer_has_data(int32_t head, int32_t tail)
{
    int32_t len = head - tail;
    return (int8_t)(len < 0 ? len + SAMPLE_BUFFER_FIFO_SIZE : len);
}

static int32_t buffer_has_room(int32_t head, int32_t tail)
{
    return SAMPLE_BUFFER_FIFO_SIZE - 1 - buffer_has_data(head, tail);
}

static int buffer_add(Sample_Buffer** buffers, int32_t* head, int32_t tail, Sample_Buffer* item)
{
    int ret = 0;
    int32_t next_head = (*head + 1) % SAMPLE_BUFFER_FIFO_SIZE;

    if (next_head != tail)
    {
        buffers[*head] = item;
        *head = next_head;
        ret++;
    }
    return ret;
}

static int buffer_peek(Sample_Buffer** buffers, int32_t head, int32_t tail, Sample_Buffer** c_ptr)
{
    int ret = 0;

    if (head != tail)
    {
        *c_ptr = buffers[tail];
        ret++;
    }
    return ret;
}

static int buffer_remove(Sample_Buffer** buffers, int32_t* head, int32_t* tail, Sample_Buffer** c_ptr)
{
    int ret = 0;

    if (*head != *tail)
    {
        *c_ptr = buffers[*tail];
        *tail = (*tail + 1) % SAMPLE_BUFFER_FIFO_SIZE;
        ret++;
    }
    return ret;
}

/* -------------------------------------------------------------------------
 * Helpers
 * ------------------------------------------------------------------------- */
static Sample_Buffer g_pool[32];
static uint32_t g_pool_idx = 0;

static Sample_Buffer* alloc_buffer(uint32_t id)
{
    Sample_Buffer* b = &g_pool[g_pool_idx++];
    b->id = id;
    return b;
}

static void reset_in_buffer(void)
{
    g_in_head = 0;
    g_in_tail = 0;
    memset(g_in_buffers, 0, sizeof(g_in_buffers));
}

static void reset_out_buffer(void)
{
    g_out_head = 0;
    g_out_tail = 0;
    memset(g_out_buffers, 0, sizeof(g_out_buffers));
}

/* =========================================================================
 * Sample_in_buffer tests
 * ========================================================================= */

static void test_in_peek_empty_returns_zero(void)
{
    reset_in_buffer();
    Sample_Buffer* ptr = NULL;
    TEST_ASSERT_EQUAL(0, buffer_peek(g_in_buffers, g_in_head, g_in_tail, &ptr));
    TEST_ASSERT_NULL(ptr);
}

static void test_in_add_single_item(void)
{
    reset_in_buffer();
    Sample_Buffer* item = alloc_buffer(1);
    TEST_ASSERT_EQUAL(1, buffer_add(g_in_buffers, &g_in_head, g_in_tail, item));
    TEST_ASSERT_EQUAL(1, buffer_has_data(g_in_head, g_in_tail));
    TEST_ASSERT_EQUAL(SAMPLE_BUFFER_FIFO_SIZE - 2, buffer_has_room(g_in_head, g_in_tail));
}

static void test_in_remove_single_item(void)
{
    reset_in_buffer();
    Sample_Buffer* item = alloc_buffer(1);
    buffer_add(g_in_buffers, &g_in_head, g_in_tail, item);

    Sample_Buffer* out = NULL;
    TEST_ASSERT_EQUAL(1, buffer_remove(g_in_buffers, &g_in_head, &g_in_tail, &out));
    TEST_ASSERT_EQUAL_PTR(item, out);
    TEST_ASSERT_EQUAL(0, buffer_has_data(g_in_head, g_in_tail));
}

static void test_in_fill_to_capacity(void)
{
    reset_in_buffer();
    int i;
    for (i = 0; i < SAMPLE_BUFFER_FIFO_SIZE - 1; i++)
    {
        TEST_ASSERT_EQUAL(1, buffer_add(g_in_buffers, &g_in_head, g_in_tail, alloc_buffer(i)));
    }
    TEST_ASSERT_EQUAL(0, buffer_has_room(g_in_head, g_in_tail));
    TEST_ASSERT_EQUAL(0, buffer_add(g_in_buffers, &g_in_head, g_in_tail, alloc_buffer(99)));
}

static void test_in_wraparound(void)
{
    reset_in_buffer();
    int i;
    for (i = 0; i < SAMPLE_BUFFER_FIFO_SIZE - 1; i++)
    {
        buffer_add(g_in_buffers, &g_in_head, g_in_tail, alloc_buffer(i));
    }

    /* Remove half */
    for (i = 0; i < (SAMPLE_BUFFER_FIFO_SIZE - 1) / 2; i++)
    {
        Sample_Buffer* out = NULL;
        buffer_remove(g_in_buffers, &g_in_head, &g_in_tail, &out);
    }

    /* Add more - should wrap around */
    for (i = 0; i < (SAMPLE_BUFFER_FIFO_SIZE - 1) / 2; i++)
    {
        TEST_ASSERT_EQUAL(1, buffer_add(g_in_buffers, &g_in_head, g_in_tail, alloc_buffer(100 + i)));
    }
    TEST_ASSERT_EQUAL(0, buffer_has_room(g_in_head, g_in_tail));
}

static void test_in_peek_does_not_remove(void)
{
    reset_in_buffer();
    Sample_Buffer* item = alloc_buffer(1);
    buffer_add(g_in_buffers, &g_in_head, g_in_tail, item);

    Sample_Buffer* peeked = NULL;
    TEST_ASSERT_EQUAL(1, buffer_peek(g_in_buffers, g_in_head, g_in_tail, &peeked));
    TEST_ASSERT_EQUAL_PTR(item, peeked);
    TEST_ASSERT_EQUAL(1, buffer_has_data(g_in_head, g_in_tail));
}

static void test_in_reset_clears_state(void)
{
    reset_in_buffer();
    buffer_add(g_in_buffers, &g_in_head, g_in_tail, alloc_buffer(1));
    buffer_add(g_in_buffers, &g_in_head, g_in_tail, alloc_buffer(2));
    buffer_reset(&g_in_head, &g_in_tail);
    TEST_ASSERT_EQUAL(0, buffer_has_data(g_in_head, g_in_tail));
    TEST_ASSERT_EQUAL(SAMPLE_BUFFER_FIFO_SIZE - 1, buffer_has_room(g_in_head, g_in_tail));
}

static void test_in_remove_from_empty_returns_zero(void)
{
    reset_in_buffer();
    Sample_Buffer* out = NULL;
    TEST_ASSERT_EQUAL(0, buffer_remove(g_in_buffers, &g_in_head, &g_in_tail, &out));
    TEST_ASSERT_NULL(out);
}

/* =========================================================================
 * Sample_out_buffer tests (same logic as input)
 * ========================================================================= */

static void test_out_add_and_peek(void)
{
    reset_out_buffer();
    Sample_Buffer* item = alloc_buffer(42);
    TEST_ASSERT_EQUAL(1, buffer_add(g_out_buffers, &g_out_head, g_out_tail, item));

    Sample_Buffer* peeked = NULL;
    TEST_ASSERT_EQUAL(1, buffer_peek(g_out_buffers, g_out_head, g_out_tail, &peeked));
    TEST_ASSERT_EQUAL_PTR(item, peeked);
}

static void test_out_fifo_order(void)
{
    reset_out_buffer();
    Sample_Buffer* items[3] = { alloc_buffer(1), alloc_buffer(2), alloc_buffer(3) };
    buffer_add(g_out_buffers, &g_out_head, g_out_tail, items[0]);
    buffer_add(g_out_buffers, &g_out_head, g_out_tail, items[1]);
    buffer_add(g_out_buffers, &g_out_head, g_out_tail, items[2]);

    Sample_Buffer* out = NULL;
    TEST_ASSERT_EQUAL(1, buffer_remove(g_out_buffers, &g_out_head, &g_out_tail, &out));
    TEST_ASSERT_EQUAL_PTR(items[0], out);

    TEST_ASSERT_EQUAL(1, buffer_remove(g_out_buffers, &g_out_head, &g_out_tail, &out));
    TEST_ASSERT_EQUAL_PTR(items[1], out);

    TEST_ASSERT_EQUAL(1, buffer_remove(g_out_buffers, &g_out_head, &g_out_tail, &out));
    TEST_ASSERT_EQUAL_PTR(items[2], out);
}

/* =========================================================================
 * Registration
 * ========================================================================= */

void test_audio_convolution_buffers_register(void)
{
    printf("--- audio_convolution_buffers ---\n");
    TEST_REGISTER(test_in_peek_empty_returns_zero);
    TEST_REGISTER(test_in_add_single_item);
    TEST_REGISTER(test_in_remove_single_item);
    TEST_REGISTER(test_in_fill_to_capacity);
    TEST_REGISTER(test_in_wraparound);
    TEST_REGISTER(test_in_peek_does_not_remove);
    TEST_REGISTER(test_in_reset_clears_state);
    TEST_REGISTER(test_in_remove_from_empty_returns_zero);
    TEST_REGISTER(test_out_add_and_peek);
    TEST_REGISTER(test_out_fifo_order);
    printf("\n");
}
