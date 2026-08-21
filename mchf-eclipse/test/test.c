/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test.c                                                          **
 **  Description:   Minimal unit test framework implementation                      **
 **  Last Modified:                                                                 **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

#include "test.h"

int test_pass_count = 0;
int test_fail_count = 0;

/* -------------------------------------------------------------------------
 * Include all test modules
 * ------------------------------------------------------------------------- */
#include "test_audio_filters.c"
#include "test_canary.c"
#include "test_volume_workaround.c"

/* -------------------------------------------------------------------------
 * Unified test runner
 * ------------------------------------------------------------------------- */
int test_main(void)
{
    printf("UHSDR Host Unit Tests\n");
    printf("=====================\n\n");

    /* Audio filter tests (T26.2 reference coverage) */
    printf("--- audio_driver_filters ---\n");
    TEST_REGISTER(test_set_biquad_coeffs);
    TEST_REGISTER(test_scale_biquad_coeffs);
    TEST_REGISTER(test_calc_bandstop);
    TEST_REGISTER(test_calc_bandpass);
    TEST_REGISTER(test_calc_shelf_filters);
    printf("\n");

    /* Canary tests (T26.1) */
    printf("--- canary (T26.1) ---\n");
    TEST_REGISTER(test_canary_initialized_intact);
    TEST_REGISTER(test_canary_pointer_non_null);
    TEST_REGISTER(test_canary_detects_corruption);
    TEST_REGISTER(test_canary_restored_after_corruption);
    TEST_REGISTER(test_canary_static_buffer_stable);
    printf("\n");

    /* Volume workaround tests (T26.2) */
    printf("--- AudioDriver_MchfVolumeWorkaround (T26.2) ---\n");
    TEST_REGISTER(test_volume_no_scaling_when_gain_within_limit);
    TEST_REGISTER(test_volume_scaled_when_gain_exceeds_max);
    TEST_REGISTER(test_volume_no_scaling_at_boundary);
    TEST_REGISTER(test_volume_zero_length_buffer);
    TEST_REGISTER(test_volume_repeated_scaling);
    TEST_REGISTER(test_volume_active_value_less_than_one);
    printf("\n");

    printf("=====================\n");
    printf("Passed: %d  Failed: %d\n", test_pass_count, test_fail_count);

    return test_fail_count > 0 ? 1 : 0;
}

int main(void)
{
    return test_main();
}
