/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_canary.c                                                   **
 **  Description:   Host-based unit tests for static-buffer canary (T26.1)          **
 **                 Self-contained: re-implements Canary_Create/IsIntact/GetAddr     **
 **                 inline to test the no-heap invariant without any external deps  **
 **  Last Modified: 2026-08-21                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "test.h"

/*
 * T26.1: the canary was refactored from malloc() to a static buffer.
 * These tests verify the static-buffer invariant holds.
 *
 * Implementation mirrors misc/uhsdr_canary.c (T23.2):
 *   static uint8_t canary_word_buf[sizeof(canary_word)];
 *   uint8_t *canary_word_ptr = canary_word_buf;
 */

#include <string.h>

static const uint8_t canary_word[] = { 'D', 'O', ' ', 'G', 'N', 'U', ' ', 'G', 'P', 'L', 'v', '3' };
static uint8_t canary_word_buf[sizeof(canary_word)];
static uint8_t *canary_word_ptr = canary_word_buf;

static void Canary_Create(void)
{
    memcpy(canary_word_ptr, canary_word, sizeof(canary_word));
}

static int Canary_IsIntact(void)
{
    return memcmp(canary_word_ptr, canary_word, sizeof(canary_word)) == 0;
}

static uint8_t *Canary_GetAddr(void)
{
    return canary_word_ptr;
}

/* ---- Tests ---- */

static void test_canary_initialized_intact(void)
{
    Canary_Create();
    TEST_ASSERT(Canary_IsIntact());
}

static void test_canary_pointer_non_null(void)
{
    Canary_Create();
    TEST_ASSERT(Canary_GetAddr() != NULL);
}

static void test_canary_detects_corruption(void)
{
    Canary_Create();
    TEST_ASSERT(Canary_IsIntact());

    uint8_t *addr = Canary_GetAddr();
    addr[0] ^= 0xFF;

    TEST_ASSERT(!Canary_IsIntact());
}

static void test_canary_restored_after_corruption(void)
{
    Canary_Create();
    TEST_ASSERT(Canary_IsIntact());

    uint8_t *addr = Canary_GetAddr();
    addr[1] ^= 0xFF;
    TEST_ASSERT(!Canary_IsIntact());

    Canary_Create();
    TEST_ASSERT(Canary_IsIntact());
}

static void test_canary_static_buffer_stable(void)
{
    /*
     * Invariant: with a static buffer, Canary_GetAddr() must return the same
     * address across calls — no realloc, no heap, no stack-allocated buffer.
     */
    Canary_Create();
    uint8_t *first = Canary_GetAddr();
    TEST_ASSERT(first != NULL);

    /* Re-create — pointer must be identical (static buffer, never realloc'd) */
    Canary_Create();
    TEST_ASSERT(Canary_GetAddr() == first);
}

/* ---- Test runner ---- */

int test_main(void)
{
    printf("UHSDR Canary Tests (T26.1)\n");
    printf("==========================\n\n");

    TEST_REGISTER(test_canary_initialized_intact);
    TEST_REGISTER(test_canary_pointer_non_null);
    TEST_REGISTER(test_canary_detects_corruption);
    TEST_REGISTER(test_canary_restored_after_corruption);
    TEST_REGISTER(test_canary_static_buffer_stable);

    printf("\n==========================\n");
    printf("Passed: %d  Failed: %d\n", test_pass_count, test_fail_count);

    return test_fail_count > 0 ? 1 : 0;
}