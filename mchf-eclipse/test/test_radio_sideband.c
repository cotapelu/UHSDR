/**
 * @file test_radio_sideband.c
 * @brief Unit tests for radio_management.c sideband logic:
 *        RadioManagement_UsesBothSidebands(), RadioManagement_LSBActive(),
 *        RadioManagement_USBActive()
 *
 * Reimplements the pure logic inline to avoid hardware-dependent headers.
 */

#include "test.h"
#include <stdint.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------
 * Demod mode constants (from uhsdr_board.h)
 * ------------------------------------------------------------------------- */
#define DEMOD_USB      0
#define DEMOD_LSB      1
#define DEMOD_CW       2
#define DEMOD_AM       3
#define DEMOD_SAM      4
#define DEMOD_FM       5
#define DEMOD_DIGI     6
#ifndef USE_TWO_CHANNEL_AUDIO
#define DEMOD_SSBSTEREO 7
#define DEMOD_IQ        8
#endif

/* -------------------------------------------------------------------------
 * SAM sideband constants (from audio_driver.h)
 * ------------------------------------------------------------------------- */
#define SAM_SIDEBAND_BOTH     0
#define SAM_SIDEBAND_LSB      1
#define SAM_SIDEBAND_USB      2
#define SAM_SIDEBAND_STEREO   3

/* -------------------------------------------------------------------------
 * Reimplementation of RadioManagement_UsesBothSidebands() for host testing
 * ------------------------------------------------------------------------- */
static bool host_RadioManagement_UsesBothSidebands(uint16_t dmod_mode,
                                                   uint8_t sam_sideband,
                                                   bool use_two_channel_audio)
{
    (void)use_two_channel_audio; /* unused when USE_TWO_CHANNEL_AUDIO not defined */
    bool retval =
            (dmod_mode == DEMOD_AM) ||
            (dmod_mode == DEMOD_SAM && sam_sideband == SAM_SIDEBAND_BOTH) ||
            (dmod_mode == DEMOD_FM);

#ifdef USE_TWO_CHANNEL_AUDIO
    if (use_two_channel_audio)
    {
        retval = retval ||
                 (dmod_mode == DEMOD_SSBSTEREO) ||
                 (dmod_mode == DEMOD_IQ) ||
                 (dmod_mode == DEMOD_SAM && sam_sideband == SAM_SIDEBAND_STEREO);
    }
#endif

    return retval;
}

/* -------------------------------------------------------------------------
 * Reimplementation of RadioManagement_LSBActive() for host testing
 * ------------------------------------------------------------------------- */
static bool host_RadioManagement_LSBActive(uint16_t dmod_mode,
                                           uint8_t sam_sideband,
                                           bool cw_lsb,
                                           bool digi_lsb)
{
    bool is_lsb;

    switch (dmod_mode)
    {
    case DEMOD_SAM:
        is_lsb = (sam_sideband == SAM_SIDEBAND_LSB);
        break;
    case DEMOD_LSB:
        is_lsb = true;
        break;
    case DEMOD_CW:
        is_lsb = cw_lsb;
        break;
    case DEMOD_DIGI:
        is_lsb = digi_lsb;
        break;
    case DEMOD_USB:
    default:
        is_lsb = false;
        break;
    }

    return is_lsb;
}

/* -------------------------------------------------------------------------
 * Reimplementation of RadioManagement_USBActive() for host testing
 * ------------------------------------------------------------------------- */
static bool host_RadioManagement_USBActive(uint16_t dmod_mode,
                                           uint8_t sam_sideband,
                                           bool cw_lsb,
                                           bool digi_lsb,
                                           bool use_two_channel_audio)
{
    return host_RadioManagement_UsesBothSidebands(dmod_mode, sam_sideband, use_two_channel_audio) == false &&
           host_RadioManagement_LSBActive(dmod_mode, sam_sideband, cw_lsb, digi_lsb) == false;
}

/* =========================================================================
 * Tests: RadioManagement_UsesBothSidebands
 * ========================================================================= */

static void test_uses_both_sidebands_am(void)
{
    TEST_ASSERT(host_RadioManagement_UsesBothSidebands(DEMOD_AM, SAM_SIDEBAND_LSB, false));
}

static void test_uses_both_sidebands_fm(void)
{
    TEST_ASSERT(host_RadioManagement_UsesBothSidebands(DEMOD_FM, SAM_SIDEBAND_USB, false));
}

static void test_uses_both_sidebands_sam_both(void)
{
    TEST_ASSERT(host_RadioManagement_UsesBothSidebands(DEMOD_SAM, SAM_SIDEBAND_BOTH, false));
}

static void test_uses_both_sidebands_sam_lsb(void)
{
    TEST_ASSERT(!host_RadioManagement_UsesBothSidebands(DEMOD_SAM, SAM_SIDEBAND_LSB, false));
}

static void test_uses_both_sidebands_sam_usb(void)
{
    TEST_ASSERT(!host_RadioManagement_UsesBothSidebands(DEMOD_SAM, SAM_SIDEBAND_USB, false));
}

static void test_uses_both_sidebands_usb(void)
{
    TEST_ASSERT(!host_RadioManagement_UsesBothSidebands(DEMOD_USB, SAM_SIDEBAND_USB, false));
}

static void test_uses_both_sidebands_lsb(void)
{
    TEST_ASSERT(!host_RadioManagement_UsesBothSidebands(DEMOD_LSB, SAM_SIDEBAND_LSB, false));
}

static void test_uses_both_sidebands_cw(void)
{
    TEST_ASSERT(!host_RadioManagement_UsesBothSidebands(DEMOD_CW, SAM_SIDEBAND_LSB, false));
}

static void test_uses_both_sidebands_digi(void)
{
    TEST_ASSERT(!host_RadioManagement_UsesBothSidebands(DEMOD_DIGI, SAM_SIDEBAND_LSB, false));
}

#ifdef USE_TWO_CHANNEL_AUDIO
static void test_uses_both_sidebands_ssb_stereo_enabled(void)
{
    TEST_ASSERT(host_RadioManagement_UsesBothSidebands(DEMOD_SSBSTEREO, SAM_SIDEBAND_LSB, true));
}

static void test_uses_both_sidebands_iq_enabled(void)
{
    TEST_ASSERT(host_RadioManagement_UsesBothSidebands(DEMOD_IQ, SAM_SIDEBAND_USB, true));
}

static void test_uses_both_sidebands_sam_stereo_enabled(void)
{
    TEST_ASSERT(host_RadioManagement_UsesBothSidebands(DEMOD_SAM, SAM_SIDEBAND_STEREO, true));
}

static void test_uses_both_sidebands_ssb_stereo_disabled(void)
{
    TEST_ASSERT(!host_RadioManagement_UsesBothSidebands(DEMOD_SSBSTEREO, SAM_SIDEBAND_LSB, false));
}
#else
/* When two-channel audio is not available, SSBSTEREO and IQ are not valid modes */
static void test_uses_both_sidebands_ssb_stereo_disabled_build(void)
{
    TEST_ASSERT(!host_RadioManagement_UsesBothSidebands(DEMOD_SSBSTEREO, SAM_SIDEBAND_LSB, false));
}
#endif

/* =========================================================================
 * Tests: RadioManagement_LSBActive
 * ========================================================================= */

static void test_lsb_active_lsb_mode(void)
{
    TEST_ASSERT(host_RadioManagement_LSBActive(DEMOD_LSB, SAM_SIDEBAND_LSB, false, false));
}

static void test_lsb_active_usb_mode(void)
{
    TEST_ASSERT(!host_RadioManagement_LSBActive(DEMOD_USB, SAM_SIDEBAND_USB, false, false));
}

static void test_lsb_active_cw_lsb(void)
{
    TEST_ASSERT(host_RadioManagement_LSBActive(DEMOD_CW, SAM_SIDEBAND_LSB, true, false));
}

static void test_lsb_active_cw_usb(void)
{
    TEST_ASSERT(!host_RadioManagement_LSBActive(DEMOD_CW, SAM_SIDEBAND_LSB, false, false));
}

static void test_lsb_active_digi_lsb(void)
{
    TEST_ASSERT(host_RadioManagement_LSBActive(DEMOD_DIGI, SAM_SIDEBAND_USB, false, true));
}

static void test_lsb_active_digi_usb(void)
{
    TEST_ASSERT(!host_RadioManagement_LSBActive(DEMOD_DIGI, SAM_SIDEBAND_LSB, false, false));
}

static void test_lsb_active_sam_lsb(void)
{
    TEST_ASSERT(host_RadioManagement_LSBActive(DEMOD_SAM, SAM_SIDEBAND_LSB, false, false));
}

static void test_lsb_active_sam_usb(void)
{
    TEST_ASSERT(!host_RadioManagement_LSBActive(DEMOD_SAM, SAM_SIDEBAND_USB, false, false));
}

static void test_lsb_active_sam_both(void)
{
    TEST_ASSERT(!host_RadioManagement_LSBActive(DEMOD_SAM, SAM_SIDEBAND_BOTH, false, false));
}

static void test_lsb_active_am(void)
{
    TEST_ASSERT(!host_RadioManagement_LSBActive(DEMOD_AM, SAM_SIDEBAND_LSB, false, false));
}

static void test_lsb_active_fm(void)
{
    TEST_ASSERT(!host_RadioManagement_LSBActive(DEMOD_FM, SAM_SIDEBAND_USB, false, false));
}

/* =========================================================================
 * Tests: RadioManagement_USBActive
 * ========================================================================= */

static void test_usb_active_usb_mode(void)
{
    TEST_ASSERT(host_RadioManagement_USBActive(DEMOD_USB, SAM_SIDEBAND_USB, false, false, false));
}

static void test_usb_active_lsb_mode(void)
{
    TEST_ASSERT(!host_RadioManagement_USBActive(DEMOD_LSB, SAM_SIDEBAND_LSB, false, false, false));
}

static void test_usb_active_cw_usb(void)
{
    TEST_ASSERT(host_RadioManagement_USBActive(DEMOD_CW, SAM_SIDEBAND_LSB, false, false, false));
}

static void test_usb_active_cw_lsb(void)
{
    TEST_ASSERT(!host_RadioManagement_USBActive(DEMOD_CW, SAM_SIDEBAND_LSB, true, false, false));
}

static void test_usb_active_sam_usb(void)
{
    TEST_ASSERT(host_RadioManagement_USBActive(DEMOD_SAM, SAM_SIDEBAND_USB, false, false, false));
}

static void test_usb_active_sam_lsb(void)
{
    TEST_ASSERT(!host_RadioManagement_USBActive(DEMOD_SAM, SAM_SIDEBAND_LSB, false, false, false));
}

static void test_usb_active_sam_both(void)
{
    /* SAM BOTH uses both sidebands, so USB is not active */
    TEST_ASSERT(!host_RadioManagement_USBActive(DEMOD_SAM, SAM_SIDEBAND_BOTH, false, false, false));
}

static void test_usb_active_am(void)
{
    /* AM uses both sidebands */
    TEST_ASSERT(!host_RadioManagement_USBActive(DEMOD_AM, SAM_SIDEBAND_LSB, false, false, false));
}

static void test_usb_active_fm(void)
{
    /* FM uses both sidebands */
    TEST_ASSERT(!host_RadioManagement_USBActive(DEMOD_FM, SAM_SIDEBAND_USB, false, false, false));
}

static void test_usb_active_digi_usb(void)
{
    TEST_ASSERT(host_RadioManagement_USBActive(DEMOD_DIGI, SAM_SIDEBAND_LSB, false, false, false));
}

static void test_usb_active_digi_lsb(void)
{
    TEST_ASSERT(!host_RadioManagement_USBActive(DEMOD_DIGI, SAM_SIDEBAND_USB, false, true, false));
}

/* =========================================================================
 * Registration
 * ========================================================================= */

void test_radio_sideband_register(void)
{
    printf("--- radio_management sideband ---\n");

    /* UsesBothSidebands */
    TEST_REGISTER(test_uses_both_sidebands_am);
    TEST_REGISTER(test_uses_both_sidebands_fm);
    TEST_REGISTER(test_uses_both_sidebands_sam_both);
    TEST_REGISTER(test_uses_both_sidebands_sam_lsb);
    TEST_REGISTER(test_uses_both_sidebands_sam_usb);
    TEST_REGISTER(test_uses_both_sidebands_usb);
    TEST_REGISTER(test_uses_both_sidebands_lsb);
    TEST_REGISTER(test_uses_both_sidebands_cw);
    TEST_REGISTER(test_uses_both_sidebands_digi);
#ifdef USE_TWO_CHANNEL_AUDIO
    TEST_REGISTER(test_uses_both_sidebands_ssb_stereo_enabled);
    TEST_REGISTER(test_uses_both_sidebands_iq_enabled);
    TEST_REGISTER(test_uses_both_sidebands_sam_stereo_enabled);
    TEST_REGISTER(test_uses_both_sidebands_ssb_stereo_disabled);
#else
    TEST_REGISTER(test_uses_both_sidebands_ssb_stereo_disabled_build);
#endif

    printf("\n");

    /* LSBActive */
    TEST_REGISTER(test_lsb_active_lsb_mode);
    TEST_REGISTER(test_lsb_active_usb_mode);
    TEST_REGISTER(test_lsb_active_cw_lsb);
    TEST_REGISTER(test_lsb_active_cw_usb);
    TEST_REGISTER(test_lsb_active_digi_lsb);
    TEST_REGISTER(test_lsb_active_digi_usb);
    TEST_REGISTER(test_lsb_active_sam_lsb);
    TEST_REGISTER(test_lsb_active_sam_usb);
    TEST_REGISTER(test_lsb_active_sam_both);
    TEST_REGISTER(test_lsb_active_am);
    TEST_REGISTER(test_lsb_active_fm);

    printf("\n");

    /* USBActive */
    TEST_REGISTER(test_usb_active_usb_mode);
    TEST_REGISTER(test_usb_active_lsb_mode);
    TEST_REGISTER(test_usb_active_cw_usb);
    TEST_REGISTER(test_usb_active_cw_lsb);
    TEST_REGISTER(test_usb_active_sam_usb);
    TEST_REGISTER(test_usb_active_sam_lsb);
    TEST_REGISTER(test_usb_active_sam_both);
    TEST_REGISTER(test_usb_active_am);
    TEST_REGISTER(test_usb_active_fm);
    TEST_REGISTER(test_usb_active_digi_usb);
    TEST_REGISTER(test_usb_active_digi_lsb);

    printf("\n");
}
