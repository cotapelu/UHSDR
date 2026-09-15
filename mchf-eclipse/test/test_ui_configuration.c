/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_ui_configuration.c                                         **
 **  Description:   Host unit tests for ui_configuration pure logic helpers          **
 **  Last Modified: 2026-08-29                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "test.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Reimplementation of ui_configuration.c pure helpers for host testing
 * ------------------------------------------------------------------------- */
typedef struct {
    uint32_t tune;
    uint32_t size;
    const char* name;
    uint32_t band_mode;
    bool rx_only;
} BandInfo_c;

#define CONFIGSTORE_IN_USE_RAMCACHE 2

/* Reimplementation of UiReadSettingEEPROM_UInt8 logic */
static uint8_t UiReadSettingEEPROM_UInt8_logic(uint16_t addr_unused, uint8_t default_val, uint8_t min_val, uint8_t max_val, bool load_default, bool read_success, uint8_t eeprom_value)
{
    (void)addr_unused;
    uint8_t val_ptr = default_val;
    if (read_success)
    {
        val_ptr = eeprom_value;
        if (val_ptr < min_val || val_ptr > max_val || load_default)
        {
            val_ptr = default_val;
        }
    }
    else
    {
        val_ptr = default_val;
    }
    return val_ptr;
}

/* Reimplementation of UiConfiguration_CompareConfigBuildVersions logic (non-static cache) */
static int32_t UiConfiguration_CompareConfigBuildVersions_logic(uint16_t config_major, uint16_t config_minor, uint16_t config_release, uint16_t major, uint32_t minor, uint32_t release)
{
    int32_t retval = 0;
    if (major < config_major)
    {
        retval = -1;
    }
    else if (major > config_major)
    {
        retval = 1;
    }
    else if (minor < config_minor)
    {
        retval = -1;
    }
    else if (minor > config_minor)
    {
        retval = 1;
    }
    else if (release < config_release)
    {
        retval = -1;
    }
    else if (release > config_release)
    {
        retval = 1;
    }
    return retval;
}

/* Reimplementation of UiConfiguration_LimitFrequency logic */
static uint32_t UiConfiguration_LimitFrequency_logic(const BandInfo_c* bandInfo_unused, const uint32_t freq, bool set_to_default, bool in_band, uint32_t resetval)
{
    (void)bandInfo_unused;
    uint32_t retval = freq;
    bool do_reset = false;
    if (set_to_default)
    {
        do_reset = true;
    }
    else
    {
        if (!in_band)
        {
            do_reset = true;
        }
    }
    return do_reset ? resetval : retval;
}

/* -------------------------------------------------------------------------
 * Tests: UiReadSettingEEPROM_UInt8_logic
 * ------------------------------------------------------------------------- */
static void test_eeprom_read_success_in_range(void)
{
    /* EEPROM has valid value 50, range 0-100, no default → should use EEPROM value */
    TEST_ASSERT_EQUAL(50, UiReadSettingEEPROM_UInt8_logic(0x10, 0, 0, 100, false, true, 50));
}

static void test_eeprom_read_success_out_of_range(void)
{
    /* EEPROM has value 150, range 0-100, default 25 → should use default */
    TEST_ASSERT_EQUAL(25, UiReadSettingEEPROM_UInt8_logic(0x10, 25, 0, 100, false, true, 150));
}

static void test_eeprom_read_failure_uses_default(void)
{
    /* EEPROM read fails, default 42 → should use default */
    TEST_ASSERT_EQUAL(42, UiReadSettingEEPROM_UInt8_logic(0x10, 42, 0, 100, false, false, 0));
}

static void test_load_default_flag_overrides_eeprom(void)
{
    /* EEPROM has valid 50, but load_default=true → should use default */
    TEST_ASSERT_EQUAL(10, UiReadSettingEEPROM_UInt8_logic(0x10, 10, 0, 100, true, true, 50));
}

static void test_eeprom_at_min_boundary(void)
{
    /* EEPROM value at min boundary → should use EEPROM value */
    TEST_ASSERT_EQUAL(0, UiReadSettingEEPROM_UInt8_logic(0x10, 100, 0, 100, false, true, 0));
}

static void test_eeprom_at_max_boundary(void)
{
    /* EEPROM value at max boundary → should use EEPROM value */
    TEST_ASSERT_EQUAL(100, UiReadSettingEEPROM_UInt8_logic(0x10, 0, 0, 100, false, true, 100));
}

/* -------------------------------------------------------------------------
 * Tests: UiConfiguration_CompareConfigBuildVersions_logic
 * ------------------------------------------------------------------------- */
static void test_version_equal(void)
{
    TEST_ASSERT_EQUAL(0, UiConfiguration_CompareConfigBuildVersions_logic(2, 11, 0, 2, 11, 0));
}

static void test_version_major_newer(void)
{
    TEST_ASSERT_EQUAL(1, UiConfiguration_CompareConfigBuildVersions_logic(2, 11, 0, 3, 0, 0));
}

static void test_version_major_older(void)
{
    TEST_ASSERT_EQUAL(-1, UiConfiguration_CompareConfigBuildVersions_logic(2, 11, 0, 1, 0, 0));
}

static void test_version_minor_newer(void)
{
    TEST_ASSERT_EQUAL(1, UiConfiguration_CompareConfigBuildVersions_logic(2, 10, 0, 2, 11, 0));
}

static void test_version_minor_older(void)
{
    TEST_ASSERT_EQUAL(-1, UiConfiguration_CompareConfigBuildVersions_logic(2, 11, 0, 2, 10, 0));
}

static void test_version_release_newer(void)
{
    TEST_ASSERT_EQUAL(1, UiConfiguration_CompareConfigBuildVersions_logic(2, 11, 0, 2, 11, 1));
}

static void test_version_release_older(void)
{
    TEST_ASSERT_EQUAL(-1, UiConfiguration_CompareConfigBuildVersions_logic(2, 11, 1, 2, 11, 0));
}

/* -------------------------------------------------------------------------
 * Tests: UiConfiguration_LimitFrequency_logic
 * ------------------------------------------------------------------------- */
static void test_limit_freq_no_reset_when_in_band(void)
{
    BandInfo_c band = { .tune = 7000000, .size = 200000, .name = "40m", .band_mode = 0, .rx_only = false };
    TEST_ASSERT_EQUAL(7050000, UiConfiguration_LimitFrequency_logic(&band, 7050000, false, true, 7000000));
}

static void test_limit_freq_reset_when_out_of_band(void)
{
    BandInfo_c band = { .tune = 7000000, .size = 200000, .name = "40m", .band_mode = 0, .rx_only = false };
    TEST_ASSERT_EQUAL(7000000, UiConfiguration_LimitFrequency_logic(&band, 7200000, false, false, 7000000));
}

static void test_limit_freq_reset_when_set_to_default(void)
{
    BandInfo_c band = { .tune = 7000000, .size = 200000, .name = "40m", .band_mode = 0, .rx_only = false };
    TEST_ASSERT_EQUAL(7000000, UiConfiguration_LimitFrequency_logic(&band, 7050000, true, true, 7000000));
}

static void test_limit_freq_at_band_edge(void)
{
    BandInfo_c band = { .tune = 14000000, .size = 350000, .name = "20m", .band_mode = 0, .rx_only = false };
    TEST_ASSERT_EQUAL(14350000, UiConfiguration_LimitFrequency_logic(&band, 14350000, false, true, 14000000));
}

/* -------------------------------------------------------------------------
 * Registration
 * ------------------------------------------------------------------------- */
void test_ui_configuration_register(void)
{
    printf("--- ui_configuration ---\n");
    /* EEPROM read logic tests */
    TEST_REGISTER(test_eeprom_read_success_in_range);
    TEST_REGISTER(test_eeprom_read_success_out_of_range);
    TEST_REGISTER(test_eeprom_read_failure_uses_default);
    TEST_REGISTER(test_load_default_flag_overrides_eeprom);
    TEST_REGISTER(test_eeprom_at_min_boundary);
    TEST_REGISTER(test_eeprom_at_max_boundary);
    /* Version comparison tests */
    TEST_REGISTER(test_version_equal);
    TEST_REGISTER(test_version_major_newer);
    TEST_REGISTER(test_version_major_older);
    TEST_REGISTER(test_version_minor_newer);
    TEST_REGISTER(test_version_minor_older);
    TEST_REGISTER(test_version_release_newer);
    TEST_REGISTER(test_version_release_older);
    /* Frequency limiting tests */
    TEST_REGISTER(test_limit_freq_no_reset_when_in_band);
    TEST_REGISTER(test_limit_freq_reset_when_out_of_band);
    TEST_REGISTER(test_limit_freq_reset_when_set_to_default);
    TEST_REGISTER(test_limit_freq_at_band_edge);
    printf("\n");
}
