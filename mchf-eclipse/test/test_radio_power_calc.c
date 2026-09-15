#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "test.h"

typedef float float32_t;

/* -------------------------------------------------------------------------
 * Replicated constants from radio_management.c
 * ------------------------------------------------------------------------- */
#define SWR_ADC_FULL_SCALE          4095
#define SWR_ADC_VOLT_REFERENCE      3.3
#define LOW_POWER_CALC_THRESHOLD    0.05
#define LOW_RF_PWR_COEFF_A         -0.0338205168744131
#define LOW_RF_PWR_COEFF_B          5.02584652062682
#define LOW_RF_PWR_COEFF_C         -106.610490958242
#define LOW_RF_PWR_COEFF_D          853.156505329744
#define HIGH_RF_PWR_COEFF_A         0.01209
#define HIGH_RF_PWR_COEFF_B         0.8334
#define HIGH_RF_PWR_COEFF_C         1.569

/* -------------------------------------------------------------------------
 * Reimplemented logic under test
 * ------------------------------------------------------------------------- */
static void RadioManagement_PowerFromADCValue_under_test(float inval, float sensor_null, float coupling_calc,
                                                         float* pwr_ptr, float* dbm_ptr)
{
    float pwr;
    const float val = sensor_null + ((inval * SWR_ADC_VOLT_REFERENCE) / SWR_ADC_FULL_SCALE);

    if (val <= LOW_POWER_CALC_THRESHOLD)
    {
        pwr = LOW_RF_PWR_COEFF_A + (LOW_RF_PWR_COEFF_B * val) + (LOW_RF_PWR_COEFF_C * (val * val)) + (LOW_RF_PWR_COEFF_D * powf(val, 3));
    }
    else
    {
        pwr = HIGH_RF_PWR_COEFF_A + (HIGH_RF_PWR_COEFF_B * val) + (HIGH_RF_PWR_COEFF_C * (val * val));
    }

    if (pwr < 0)
    {
        pwr = 0;
    }

    const float dbm = (10 * (log10f(pwr))) + 30 + coupling_calc;
    *dbm_ptr = dbm;
    *pwr_ptr = powf(10.0f, dbm / 10) / 1000;
}

/* -------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */
static void test_low_power_calculation(void)
{
    float pwr = 0, dbm = 0;
    /* Low input -> low power path, use value that produces small positive power */
    RadioManagement_PowerFromADCValue_under_test(50, 0, 0, &pwr, &dbm);
    TEST_ASSERT(pwr >= 0);
    TEST_ASSERT(dbm > -100); /* dbm should be finite, not -inf */
}

static void test_high_power_calculation(void)
{
    float pwr = 0, dbm = 0;
    /* High input -> high power path */
    RadioManagement_PowerFromADCValue_under_test(4095, 0, 0, &pwr, &dbm);
    TEST_ASSERT(pwr > 0);
    TEST_ASSERT(dbm > 0);
}

static void test_coupling_offset_applied(void)
{
    float pwr1 = 0, dbm1 = 0;
    float pwr2 = 0, dbm2 = 0;
    /* Same input, different coupling -> different dbm */
    RadioManagement_PowerFromADCValue_under_test(2000, 0, 0, &pwr1, &dbm1);
    RadioManagement_PowerFromADCValue_under_test(2000, 0, 3, &pwr2, &dbm2);
    TEST_ASSERT(fabsf(dbm1 - dbm2) > 0.1f);
}

static void test_negative_power_clamped_to_zero(void)
{
    float pwr = 0, dbm = 0;
    /* Input that would produce negative power from low-power equation */
    RadioManagement_PowerFromADCValue_under_test(1, 0, 0, &pwr, &dbm);
    TEST_ASSERT(pwr >= 0);
}

static void test_sensor_null_offset(void)
{
    float pwr1 = 0, dbm1 = 0;
    float pwr2 = 0, dbm2 = 0;
    /* Positive sensor_null should increase val and thus power */
    RadioManagement_PowerFromADCValue_under_test(1000, 0, 0, &pwr1, &dbm1);
    RadioManagement_PowerFromADCValue_under_test(1000, 0.1f, 0, &pwr2, &dbm2);
    TEST_ASSERT(pwr2 > pwr1);
}

static void test_deterministic_output(void)
{
    float pwr1 = 0, dbm1 = 0;
    float pwr2 = 0, dbm2 = 0;
    RadioManagement_PowerFromADCValue_under_test(1500, 0.05f, 1.5f, &pwr1, &dbm1);
    RadioManagement_PowerFromADCValue_under_test(1500, 0.05f, 1.5f, &pwr2, &dbm2);
    TEST_ASSERT_FLOAT_EQ(pwr1, pwr2, 0.0001f);
    TEST_ASSERT_FLOAT_EQ(dbm1, dbm2, 0.0001f);
}

static void test_boundary_low_power_threshold(void)
{
    float pwr_at = 0, dbm_at = 0;
    float pwr_below = 0, dbm_below = 0;
    /* At threshold vs just below should use low power path */
    RadioManagement_PowerFromADCValue_under_test(60, 0, 0, &pwr_at, &dbm_at);
    RadioManagement_PowerFromADCValue_under_test(59, 0, 0, &pwr_below, &dbm_below);
    TEST_ASSERT(pwr_at >= 0);
    TEST_ASSERT(pwr_below >= 0);
}

/* -------------------------------------------------------------------------
 * Tests: RadioManagement_CalculatePowerFactorScale
 * ------------------------------------------------------------------------- */

static float32_t host_calculate_power_factor_scale(float32_t powerMw)
{
    /* reference_power = 5000.0 mW (5W) from mchf_pa config */
    const float32_t reference_power = 5000.0f;
    float32_t retval = 1.0f;
    if (powerMw > 0)
    {
        retval = sqrtf(powerMw / reference_power);
    }
    return retval;
}

static void test_power_factor_scale_zero_power(void)
{
    /* Zero or negative power should return scale 1 */
    TEST_ASSERT_FLOAT_EQ(1.0f, host_calculate_power_factor_scale(0.0f), 0.0001f);
    TEST_ASSERT_FLOAT_EQ(1.0f, host_calculate_power_factor_scale(-10.0f), 0.0001f);
}

static void test_power_factor_scale_reference_power(void)
{
    /* At reference power (5W), scale should be 1 */
    TEST_ASSERT_FLOAT_EQ(1.0f, host_calculate_power_factor_scale(5000.0f), 0.0001f);
}

static void test_power_factor_scale_half_power(void)
{
    /* Half power should give sqrt(0.5) scale */
    TEST_ASSERT_FLOAT_EQ(0.70710678f, host_calculate_power_factor_scale(2500.0f), 0.0001f);
}

static void test_power_factor_scale_double_power(void)
{
    /* Double power should give sqrt(2) scale */
    TEST_ASSERT_FLOAT_EQ(1.41421356f, host_calculate_power_factor_scale(10000.0f), 0.0001f);
}

static void test_power_factor_scale_small_power(void)
{
    /* Small power should give small scale */
    float32_t scale = host_calculate_power_factor_scale(100.0f);
    TEST_ASSERT(scale > 0.0f);
    TEST_ASSERT(scale < 0.2f);
}

static void test_power_factor_scale_monotonic(void)
{
    /* Scale should increase with power */
    float32_t prev = host_calculate_power_factor_scale(100.0f);
    for (float32_t p = 200.0f; p <= 10000.0f; p += 500.0f)
    {
        float32_t curr = host_calculate_power_factor_scale(p);
        TEST_ASSERT(curr > prev);
        prev = curr;
    }
}

/* -------------------------------------------------------------------------
 * Registration
 * ------------------------------------------------------------------------- */
void test_radio_power_calc_register(void)
{
    printf("--- radio_management power calc ---\n");
    TEST_REGISTER(test_low_power_calculation);
    TEST_REGISTER(test_high_power_calculation);
    TEST_REGISTER(test_coupling_offset_applied);
    TEST_REGISTER(test_negative_power_clamped_to_zero);
    TEST_REGISTER(test_sensor_null_offset);
    TEST_REGISTER(test_deterministic_output);
    TEST_REGISTER(test_boundary_low_power_threshold);

    /* power factor scale tests */
    TEST_REGISTER(test_power_factor_scale_zero_power);
    TEST_REGISTER(test_power_factor_scale_reference_power);
    TEST_REGISTER(test_power_factor_scale_half_power);
    TEST_REGISTER(test_power_factor_scale_double_power);
    TEST_REGISTER(test_power_factor_scale_small_power);
    TEST_REGISTER(test_power_factor_scale_monotonic);
    printf("\n");
}
