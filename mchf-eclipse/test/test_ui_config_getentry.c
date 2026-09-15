/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_ui_config_getentry.c                                       **
 **  Description:   Host unit tests for UiConfiguration_GetEntry()                  **
 **                 (linear search over config table)                              **
 **  Last Modified: 2026-08-29                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "test.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Constants matching production code
 * ------------------------------------------------------------------------- */
#define ConfigEntry_Stop      0
#define ConfigEntry_UInt8     1
#define ConfigEntry_UInt16    2
#define ConfigEntry_UInt32    3

/* -------------------------------------------------------------------------
 * Minimal ConfigEntryDescriptor for testing
 * ------------------------------------------------------------------------- */
typedef struct {
    int typeId;
    uint16_t id;
    volatile void* val_ptr;
    int32_t val_default;
    int32_t val_min;
    int32_t val_max;
} ConfigEntryDescriptor;

/* -------------------------------------------------------------------------
 * Test fixture: synthetic config table
 * ------------------------------------------------------------------------- */
static ConfigEntryDescriptor g_test_table[4];
static int g_table_count;

static void setup_table(const ConfigEntryDescriptor* table, int count)
{
    memcpy(g_test_table, table, count * sizeof(ConfigEntryDescriptor));
    g_table_count = count;
}

/* -------------------------------------------------------------------------
 * Reimplemented UiConfiguration_GetEntry (from ui_configuration.c)
 * ------------------------------------------------------------------------- */
static const ConfigEntryDescriptor* UiConfiguration_GetEntry(uint16_t id)
{
    int idx;
    const ConfigEntryDescriptor* retval = NULL;
    for (idx = 0; idx < g_table_count && g_test_table[idx].typeId != ConfigEntry_Stop; idx++)
    {
        if (g_test_table[idx].id == id)
        {
            retval = &g_test_table[idx];
            break;
        }
    }
    return retval;
}

/* -------------------------------------------------------------------------
 * Helpers to build test entries
 * ------------------------------------------------------------------------- */
static ConfigEntryDescriptor entry(uint16_t id, int typeId)
{
    ConfigEntryDescriptor e;
    memset(&e, 0, sizeof(e));
    e.id = id;
    e.typeId = typeId;
    return e;
}

static ConfigEntryDescriptor stop(void)
{
    ConfigEntryDescriptor e;
    memset(&e, 0, sizeof(e));
    e.typeId = ConfigEntry_Stop;
    return e;
}

/* =========================================================================
 * UiConfiguration_GetEntry tests
 * ========================================================================= */

static void test_empty_table_returns_null(void)
{
    setup_table(NULL, 0);
    TEST_ASSERT(UiConfiguration_GetEntry(0) == NULL);
    TEST_ASSERT(UiConfiguration_GetEntry(123) == NULL);
}

static void test_single_entry_found(void)
{
    ConfigEntryDescriptor table[] = { entry(0x10, ConfigEntry_UInt8), stop() };
    setup_table(table, 2);
    TEST_ASSERT((uintptr_t)&g_test_table[0] == (uintptr_t)UiConfiguration_GetEntry(0x10));
}

static void test_single_entry_not_found(void)
{
    ConfigEntryDescriptor table[] = { entry(0x10, ConfigEntry_UInt8), stop() };
    setup_table(table, 2);
    TEST_ASSERT(UiConfiguration_GetEntry(0x20) == NULL);
}

static void test_multiple_entries_finds_first(void)
{
    ConfigEntryDescriptor table[] = {
        entry(0x10, ConfigEntry_UInt8),
        entry(0x20, ConfigEntry_UInt16),
        entry(0x30, ConfigEntry_UInt32),
        stop()
    };
    setup_table(table, 4);
    TEST_ASSERT((uintptr_t)&g_test_table[0] == (uintptr_t)UiConfiguration_GetEntry(0x10));
    TEST_ASSERT((uintptr_t)&g_test_table[1] == (uintptr_t)UiConfiguration_GetEntry(0x20));
    TEST_ASSERT((uintptr_t)&g_test_table[2] == (uintptr_t)UiConfiguration_GetEntry(0x30));
}

static void test_multiple_entries_not_found(void)
{
    ConfigEntryDescriptor table[] = {
        entry(0x10, ConfigEntry_UInt8),
        entry(0x20, ConfigEntry_UInt16),
        stop()
    };
    setup_table(table, 3);
    TEST_ASSERT(UiConfiguration_GetEntry(0x99) == NULL);
}

static void test_stops_at_stop_sentinel(void)
{
    ConfigEntryDescriptor table[] = {
        entry(0x10, ConfigEntry_UInt8),
        stop(),
        entry(0x99, ConfigEntry_UInt8)  /* should be ignored */
    };
    setup_table(table, 3);
    TEST_ASSERT((uintptr_t)&g_test_table[0] == (uintptr_t)UiConfiguration_GetEntry(0x10));
    TEST_ASSERT(UiConfiguration_GetEntry(0x99) == NULL);
}

static void test_id_zero_found(void)
{
    ConfigEntryDescriptor table[] = { entry(0x00, ConfigEntry_UInt8), stop() };
    setup_table(table, 2);
    TEST_ASSERT((uintptr_t)&g_test_table[0] == (uintptr_t)UiConfiguration_GetEntry(0x00));
}

static void test_max_uint16_not_found(void)
{
    ConfigEntryDescriptor table[] = { entry(0x10, ConfigEntry_UInt8), stop() };
    setup_table(table, 2);
    TEST_ASSERT(UiConfiguration_GetEntry(0xFFFF) == NULL);
}

static void test_duplicate_ids_returns_first(void)
{
    ConfigEntryDescriptor table[] = {
        entry(0x10, ConfigEntry_UInt8),
        entry(0x10, ConfigEntry_UInt16),
        stop()
    };
    setup_table(table, 3);
    TEST_ASSERT((uintptr_t)&g_test_table[0] == (uintptr_t)UiConfiguration_GetEntry(0x10));
}

static void test_typeid_ignored_for_lookup(void)
{
    ConfigEntryDescriptor table[] = {
        entry(0x10, ConfigEntry_UInt8),
        entry(0x10, ConfigEntry_UInt16),
        stop()
    };
    setup_table(table, 3);
    /* Lookup is by ID only, not typeId */
    TEST_ASSERT((uintptr_t)&g_test_table[0] == (uintptr_t)UiConfiguration_GetEntry(0x10));
}

/* =========================================================================
 * Registration
 * ========================================================================= */

void test_ui_config_getentry_register(void)
{
    printf("--- ui_config_getentry ---\n");
    TEST_REGISTER(test_empty_table_returns_null);
    TEST_REGISTER(test_single_entry_found);
    TEST_REGISTER(test_single_entry_not_found);
    TEST_REGISTER(test_multiple_entries_finds_first);
    TEST_REGISTER(test_multiple_entries_not_found);
    TEST_REGISTER(test_stops_at_stop_sentinel);
    TEST_REGISTER(test_id_zero_found);
    TEST_REGISTER(test_max_uint16_not_found);
    TEST_REGISTER(test_duplicate_ids_returns_first);
    TEST_REGISTER(test_typeid_ignored_for_lookup);
    printf("\n");
}
