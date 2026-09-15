/**
 * @file test_radio_freq_in_band.c
 * @brief Unit tests for radio_management.c RadioManagement_FreqIsInBand()
 *
 * Reimplements the frequency range check logic inline to avoid
 * hardware-dependent headers and global state.
 */

#include "test.h"
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

/* -------------------------------------------------------------------------
 * Replicated BandInfo struct from radio_management.h
 * ------------------------------------------------------------------------- */

typedef struct {
    uint32_t tune;
    uint32_t size;
    uint8_t band_mode;
    const char* name;
} BandInfo;

/* -------------------------------------------------------------------------
 * Reimplementation of RadioManagement_FreqIsInBand()
 * ------------------------------------------------------------------------- */

static bool host_freq_is_in_band(const BandInfo* bandinfo, uint32_t freq)
{
    assert(bandinfo != NULL);
    return (freq >= bandinfo->tune) && (freq <= (bandinfo->tune + bandinfo->size));
}

/* =========================================================================
 * Tests: RadioManagement_FreqIsInBand
 * ========================================================================= */

static void test_freq_at_band_start(void)
{
    BandInfo band = { .tune = 3500000, .size = 300000, .name = "80m", .band_mode = 0 };
    TEST_ASSERT(host_freq_is_in_band(&band, 3500000));
}

static void test_freq_at_band_end(void)
{
    BandInfo band = { .tune = 3500000, .size = 300000, .name = "80m", .band_mode = 0 };
    TEST_ASSERT(host_freq_is_in_band(&band, 3800000));
}

static void test_freq_below_band(void)
{
    BandInfo band = { .tune = 3500000, .size = 300000, .name = "80m", .band_mode = 0 };
    TEST_ASSERT(!host_freq_is_in_band(&band, 3499999));
}

static void test_freq_above_band(void)
{
    BandInfo band = { .tune = 3500000, .size = 300000, .name = "80m", .band_mode = 0 };
    TEST_ASSERT(!host_freq_is_in_band(&band, 3800001));
}

static void test_freq_middle_of_band(void)
{
    BandInfo band = { .tune = 3500000, .size = 300000, .name = "80m", .band_mode = 0 };
    TEST_ASSERT(host_freq_is_in_band(&band, 3650000));
}

static void test_zero_size_band(void)
{
    BandInfo band = { .tune = 5000000, .size = 0, .name = "Gen", .band_mode = 0 };
    TEST_ASSERT(host_freq_is_in_band(&band, 5000000));
    TEST_ASSERT(!host_freq_is_in_band(&band, 5000001));
}

static void test_full_ham_band(void)
{
    /* 20m band: 14.0 MHz to 14.35 MHz */
    BandInfo band = { .tune = 14000000, .size = 350000, .name = "20m", .band_mode = 0 };
    TEST_ASSERT(host_freq_is_in_band(&band, 14000000));
    TEST_ASSERT(host_freq_is_in_band(&band, 14350000));
    TEST_ASSERT(!host_freq_is_in_band(&band, 13999999));
    TEST_ASSERT(!host_freq_is_in_band(&band, 14350001));
}

static void test_very_large_band(void)
{
    /* 23cm band: 1.24 GHz to 1.3 GHz */
    BandInfo band = { .tune = 1240000000, .size = 60000000, .name = "23cm", .band_mode = 0 };
    TEST_ASSERT(host_freq_is_in_band(&band, 1240000000));
    TEST_ASSERT(host_freq_is_in_band(&band, 1300000000));
    TEST_ASSERT(!host_freq_is_in_band(&band, 1239999999));
    TEST_ASSERT(!host_freq_is_in_band(&band, 1300000001));
}

static void test_very_small_band(void)
{
    BandInfo band = { .tune = 1812000, .size = 200000, .name = "160m", .band_mode = 0 };
    TEST_ASSERT(host_freq_is_in_band(&band, 1812000));
    TEST_ASSERT(host_freq_is_in_band(&band, 1812001));
    TEST_ASSERT(!host_freq_is_in_band(&band, 1811999));
}

static void test_bandinfo_name_ignored(void)
{
    /* The name field is ignored by the function */
    BandInfo band = { .tune = 7000000, .size = 200000, .name = NULL, .band_mode = 0 };
    TEST_ASSERT(host_freq_is_in_band(&band, 7100000));
    TEST_ASSERT(!host_freq_is_in_band(&band, 6999999));
}

/* =========================================================================
 * Registration
 * ========================================================================= */

void test_radio_freq_in_band_register(void)
{
    printf("--- radio_freq_in_band ---\n");

    TEST_REGISTER(test_freq_at_band_start);
    TEST_REGISTER(test_freq_at_band_end);
    TEST_REGISTER(test_freq_below_band);
    TEST_REGISTER(test_freq_above_band);
    TEST_REGISTER(test_freq_middle_of_band);
    TEST_REGISTER(test_zero_size_band);
    TEST_REGISTER(test_full_ham_band);
    TEST_REGISTER(test_very_large_band);
    TEST_REGISTER(test_very_small_band);
    TEST_REGISTER(test_bandinfo_name_ignored);

    printf("\n");
}
