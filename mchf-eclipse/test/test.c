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
#include "test_audio_api.c"
#include "test_audio_twinpeaks.c"
#include "test_remote.c"
#include "test_ui_spectrum_ctrl.c"
#include "test_remote_queue.c"
#include "test_profiling.c"
#include "test_ui_driver_utils.c"
#include "test_cat_driver.c"
#include "test_uhsdr_math.c"
#include "test_radio_management.c"
#include "test_mock_hal.c"
#include "test_ui_configuration.c"
#include "test_ui_menu.c"
#include "test_audio_filter.c"
#include "test_goertzel.c"
#include "test_lcd_utils.c"
#include "test_ui_injection.c"
#include "test_ui_split_mode.c"
#include "test_lcd_blanking.c"
#include "test_tuning_step.c"
#include "test_demod_mode_change.c"
#include "test_tx_sidetone.c"
#include "test_power2string.c"
#include "test_ui_driver_remote.c"
#include "test_uhsdr_keypad.c"
#include "test_decayavg.c"
#include "test_ui_timer.c"

/* -------------------------------------------------------------------------
 * Forward declarations for separately compiled test modules
 * ------------------------------------------------------------------------- */
void test_hal_uart_register(void);
void test_ui_state_queries_register(void);
void test_ui_config_getentry_register(void);
void test_audio_translate_freq_register(void);
void test_cw_dial_offset_register(void);
void test_audio_convolution_buffers_register(void);
void test_ui_driver_remote_register(void);
void test_uhsdr_keypad_register(void);
void test_decayavg_register(void);
void test_ui_timer_register(void);
void test_psk_varicode_register(void);
void test_audio_management_balance_register(void);
void test_radio_power_calc_register(void);
void test_radio_sideband_register(void);
void test_ui_menu_item_change_register(void);
void test_rtty_baudot_register(void);
void test_cw_gen_helpers_register(void);
void test_cw_char_lookup_register(void);
void test_audio_nr_notch_register(void);
void test_ui_voltage_display_register(void);
void test_audio_alc_decay_register(void);
void test_audio_biquad_coeffs_register(void);
void test_cw_reverse_code_register(void);
void test_radio_freq_in_band_register(void);
void test_lcd_text_width_register(void);
void test_radio_freq_translation_mode_register(void);
void test_cw_decoder_spike_register(void);

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
    TEST_REGISTER(test_set_biquad_coeffs_all_instances);
    TEST_REGISTER(test_calc_bandstop_multiple_sample_rates);
    TEST_REGISTER(test_calc_bandpass_multiple_sample_rates);
    TEST_REGISTER(test_calc_bandstop_low_freq);
    TEST_REGISTER(test_calc_bandstop_near_nyquist);
    TEST_REGISTER(test_calc_bandpass_low_freq);
    TEST_REGISTER(test_calc_bandpass_high_freq);
    TEST_REGISTER(test_calc_high_shelf_extreme_gain);
    TEST_REGISTER(test_calc_low_shelf_extreme_gain);
    TEST_REGISTER(test_calc_high_shelf_zero_gain);
    TEST_REGISTER(test_calc_low_shelf_zero_gain);
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
    TEST_REGISTER(test_volume_active_value_greater_than_one);
    TEST_REGISTER(test_volume_negative_buffer_values);
    TEST_REGISTER(test_volume_zero_buffer_values);
    printf("\n");

    /* Audio API tests (T35) */
    printf("--- Audio API (T35) ---\n");
    TEST_REGISTER(test_translate_freq_off);
    TEST_REGISTER(test_translate_freq_p6khz);
    TEST_REGISTER(test_translate_freq_m6khz);
    TEST_REGISTER(test_translate_freq_p12khz);
    TEST_REGISTER(test_translate_freq_m12khz);
    TEST_REGISTER(test_translate_freq_invalid_mode_returns_zero);
    printf("\n");

    /* Twinpeaks logic (T35.5) */
    printf("--- Twinpeaks Logic (T35.5) ---\n");
    TEST_REGISTER(test_twinpeaks_skip_zero_teta3);
    TEST_REGISTER(test_twinpeaks_small_phase_done);
    TEST_REGISTER(test_twinpeaks_large_phase_restart);
    TEST_REGISTER(test_twinpeaks_four_restarts_uncorrectable);
    TEST_REGISTER(test_twinpeaks_boundary_below_threshold);
    printf("\n");

    /* Remote protocol tests (T46.1) */
    test_remote_register();
    printf("\n");

    /* Remote queue tests (T46.2) */
    test_remote_queue_register();
    printf("\n");

    /* UI spectrum control tests (T42.1) */
    test_ui_spectrum_ctrl_register();
    printf("\n");

    /* Profiling WCET logic tests (new) */
    test_profiling_register();

    /* ui_driver_utils clamp/wrap tests */
    test_ui_driver_utils_register();

    /* cat_driver pure helper tests */
    test_cat_driver_register();

    /* uhsdr_math pure math tests */
    test_uhsdr_math_register();

    /* radio_management pure logic tests */
    test_radio_management_register();

    /* mock HAL layer tests */
    test_mock_hal_register();

    /* hal_uart shim tests */
    test_hal_uart_register();

    /* ui_configuration logic tests */
    test_ui_configuration_register();

    /* ui_menu float2fixedstr logic tests */
    test_ui_menu_register();

    /* audio_filter mode mapping tests */
    test_audio_filter_register();

    /* goertzel pure math tests */
    test_goertzel_register();

    /* lcd string utility tests */
    test_lcd_utils_register();

    /* UI injection tests */
    test_ui_injection_register();

    /* UI split mode tests */
    test_ui_split_mode_register();

    /* LCD blanking timer tests */
    test_lcd_blanking_register();

    /* Tuning step change tests */
    test_tuning_step_register();

    /* Demod mode change detection tests */
    test_demod_mode_change_register();

    /* TX sidetone usage tests */
    test_tx_sidetone_register();

    /* Power-to-string tests */
    test_power2string_register();

    /* ui_state_queries tests */
    test_ui_state_queries_register();

    /* ui_config_getentry tests */
    test_ui_config_getentry_register();

    /* audio_translate_freq tests */
    test_audio_translate_freq_register();

    /* cw_dial_offset tests */
    test_cw_dial_offset_register();

    /* audio_convolution_buffers tests */
    test_audio_convolution_buffers_register();

    /* ui_driver_remote tests */
    test_ui_driver_remote_register();

    /* keypad tests */
    test_uhsdr_keypad_register();

    /* decayavg tests */
    test_decayavg_register();

    /* ui_timer tests */
    test_ui_timer_register();

    /* psk varicode tests */
    test_psk_varicode_register();

    /* audio management balance tests */
    test_audio_management_balance_register();

    /* radio management sideband tests */
    test_radio_sideband_register();

    /* ui_menu item change tests */
    test_ui_menu_item_change_register();

    /* radio management power calc tests */
    test_radio_power_calc_register();

    /* rtty baudot tests */
    test_rtty_baudot_register();

    /* cw_gen helper tests */
    test_cw_gen_helpers_register();

    /* cw_gen character lookup tests */
    test_cw_char_lookup_register();

    /* audio_nr notch coeff tests */
    test_audio_nr_notch_register();

    /* ui voltage display tests */
    test_ui_voltage_display_register();

    /* audio_alc_decay tests */
    test_audio_alc_decay_register();

    /* audio biquad coeff tests */
    test_audio_biquad_coeffs_register();

    /* cw reverse code tests */
    test_cw_reverse_code_register();

    /* radio freq in band tests */
    test_radio_freq_in_band_register();

    /* lcd text width tests */
    test_lcd_text_width_register();

    /* radio freq translation mode tests */
    test_radio_freq_translation_mode_register();

    /* cw decoder spike tests */
    test_cw_decoder_spike_register();

    printf("=====================\n");
    printf("Passed: %d  Failed: %d\n", test_pass_count, test_fail_count);

    return test_fail_count > 0 ? 1 : 0;
}

int main(void)
{
    return test_main();
}
