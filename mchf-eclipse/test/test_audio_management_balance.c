#include <stdint.h>
#include <stdbool.h>
#include "test.h"

typedef float float32_t;

/* -------------------------------------------------------------------------
 * Replicated types from audio_management.h
 * ------------------------------------------------------------------------- */
#define IQ_BALANCE_OFF  -128
#define IQ_BALANCE_MAX  127

typedef struct
{
    int32_t phase;
    int32_t gain;
} iq_adjust_balance_t;

typedef enum
{
    IQ_RX_GAIN,
    IQ_RX_PHASE,
    IQ_TX_TRANS_ON_GAIN,
    IQ_TX_TRANS_OFF_GAIN,
    IQ_TX_TRANS_ON_PHASE,
    IQ_TX_TRANS_OFF_PHASE,
} iq_adjust_params_t;

typedef struct {
   iq_adjust_balance_t rx;
   iq_adjust_balance_t tx[2];
} iq_adjust_vals_t;

typedef struct
{
    int32_t freq;
    iq_adjust_vals_t adj;
} freq_adjust_point_t;

#define IQ_TRANS_ON  0
#define IQ_TRANS_OFF 1

typedef struct {
    float32_t i;
    float32_t q;
} iq_float_t;

/* -------------------------------------------------------------------------
 * Reimplemented logic under test
 * ------------------------------------------------------------------------- */
static int32_t AudioManagement_GetBalanceValFromStruct(const freq_adjust_point_t* point, iq_adjust_params_t what)
{
    int32_t retval = IQ_BALANCE_OFF;
    switch(what)
    {
    case IQ_RX_GAIN:
        retval = point->adj.rx.gain;
        break;
    case IQ_RX_PHASE:
         retval = point->adj.rx.phase;
         break;
    case IQ_TX_TRANS_ON_GAIN:
        retval = point->adj.tx[IQ_TRANS_ON].gain;
        break;
    case IQ_TX_TRANS_OFF_GAIN:
        retval = point->adj.tx[IQ_TRANS_OFF].gain;
        break;
    case IQ_TX_TRANS_ON_PHASE:
        retval = point->adj.tx[IQ_TRANS_ON].phase;
        break;
    case IQ_TX_TRANS_OFF_PHASE:
        retval = point->adj.tx[IQ_TRANS_OFF].phase;
        break;
    }
    return retval;
}

static uint32_t AudioManagement_GetNextValid(const freq_adjust_point_t* points, uint32_t idx, iq_adjust_params_t what)
{
    while (points[idx].freq != 0)
    {
        int32_t val = AudioManagement_GetBalanceValFromStruct(&points[idx], what);
        if (val != IQ_BALANCE_OFF)
        {
            break;
        }
        idx++;
    }
    return idx;
}

static void AudioManagement_FindFreqRange(const freq_adjust_point_t* points, uint32_t freq, iq_adjust_params_t what,
                                          float32_t* adj_low_ptr, float32_t* adj_high_ptr, float32_t* freq_low_ptr, float32_t* freq_high_ptr)
{
    uint32_t idx = AudioManagement_GetNextValid(points, 0, what);
    uint32_t previous_idx = idx;
    uint32_t before_previous_idx = idx;
    uint32_t valid_points = 0;
    bool found_match = false;

    while (points[idx].freq != 0)
    {
        valid_points++;
        if (freq < (uint32_t)points[idx].freq)
        {
            if (previous_idx == idx)
            {
                idx = AudioManagement_GetNextValid(points, idx + 1, what);
                if (points[idx].freq == 0)
                {
                    idx = previous_idx;
                }
            }
            found_match = true;
            break;
        }
        before_previous_idx = previous_idx;
        previous_idx = idx;
        idx = AudioManagement_GetNextValid(points, idx + 1, what);
    }

    if (found_match == false)
    {
        if (valid_points > 1)
        {
            idx = previous_idx;
            previous_idx = before_previous_idx;
        }
        else
        {
            idx = previous_idx;
        }
    }

    *adj_high_ptr = AudioManagement_GetBalanceValFromStruct(&points[idx], what);
    *adj_low_ptr = AudioManagement_GetBalanceValFromStruct(&points[previous_idx], what);
    *freq_high_ptr = points[idx].freq;
    *freq_low_ptr = points[previous_idx].freq;
}

static float32_t AudioManagement_CalcAdjustInFreqRangeHelperNew(const freq_adjust_point_t* points, iq_adjust_params_t what, float32_t freq, float32_t scaling)
{
    float32_t adj_low, adj_high;
    float32_t freq_low, freq_high;

    AudioManagement_FindFreqRange(points, freq, what, &adj_low, &adj_high, &freq_low, &freq_high);

    if (adj_high == IQ_BALANCE_OFF)
    {
        if (adj_low == IQ_BALANCE_OFF)
        {
            adj_high = adj_low = 0.0f;
        }
        else
        {
            adj_high = adj_low;
        }
    }
    else if (adj_low == IQ_BALANCE_OFF)
    {
        adj_low = adj_high;
    }

    float32_t adj_delta = (freq_high != freq_low) ? (adj_high - adj_low) / (freq_high - freq_low) * (freq - freq_low) : 0;
    return (adj_delta + adj_low) / scaling;
}

static void AudioManagement_CalcIqGainAdjustVarHelper(iq_float_t* var, float32_t adj)
{
    var->i = 1 + adj;
    var->q = 1 - adj;
}

/* -------------------------------------------------------------------------
 * Tests: GetBalanceValFromStruct
 * ------------------------------------------------------------------------- */
static void test_rx_gain(void)
{
    freq_adjust_point_t p = { .adj.rx = { .gain = 10, .phase = 20 } };
    TEST_ASSERT_EQUAL(10, AudioManagement_GetBalanceValFromStruct(&p, IQ_RX_GAIN));
}

static void test_rx_phase(void)
{
    freq_adjust_point_t p = { .adj.rx = { .gain = 10, .phase = 20 } };
    TEST_ASSERT_EQUAL(20, AudioManagement_GetBalanceValFromStruct(&p, IQ_RX_PHASE));
}

static void test_tx_trans_on_gain(void)
{
    freq_adjust_point_t p = { .adj.tx = { { .gain = 30, .phase = 40 }, { .gain = 50, .phase = 60 } } };
    TEST_ASSERT_EQUAL(30, AudioManagement_GetBalanceValFromStruct(&p, IQ_TX_TRANS_ON_GAIN));
}

static void test_tx_trans_off_gain(void)
{
    freq_adjust_point_t p = { .adj.tx = { { .gain = 30, .phase = 40 }, { .gain = 50, .phase = 60 } } };
    TEST_ASSERT_EQUAL(50, AudioManagement_GetBalanceValFromStruct(&p, IQ_TX_TRANS_OFF_GAIN));
}

static void test_tx_trans_on_phase(void)
{
    freq_adjust_point_t p = { .adj.tx = { { .gain = 30, .phase = 40 }, { .gain = 50, .phase = 60 } } };
    TEST_ASSERT_EQUAL(40, AudioManagement_GetBalanceValFromStruct(&p, IQ_TX_TRANS_ON_PHASE));
}

static void test_tx_trans_off_phase(void)
{
    freq_adjust_point_t p = { .adj.tx = { { .gain = 30, .phase = 40 }, { .gain = 50, .phase = 60 } } };
    TEST_ASSERT_EQUAL(60, AudioManagement_GetBalanceValFromStruct(&p, IQ_TX_TRANS_OFF_PHASE));
}

static void test_unknown_param_returns_off(void)
{
    freq_adjust_point_t p = { .adj.rx = { .gain = 10, .phase = 20 } };
    TEST_ASSERT_EQUAL(IQ_BALANCE_OFF, AudioManagement_GetBalanceValFromStruct(&p, (iq_adjust_params_t)99));
}

static void test_negative_values_preserved(void)
{
    freq_adjust_point_t p = { .adj.rx = { .gain = -50, .phase = -99 } };
    TEST_ASSERT_EQUAL(-50, AudioManagement_GetBalanceValFromStruct(&p, IQ_RX_GAIN));
    TEST_ASSERT_EQUAL(-99, AudioManagement_GetBalanceValFromStruct(&p, IQ_RX_PHASE));
}

static void test_boundary_values(void)
{
    freq_adjust_point_t p = { .adj.rx = { .gain = IQ_BALANCE_OFF, .phase = IQ_BALANCE_MAX } };
    TEST_ASSERT_EQUAL(IQ_BALANCE_OFF, AudioManagement_GetBalanceValFromStruct(&p, IQ_RX_GAIN));
    TEST_ASSERT_EQUAL(IQ_BALANCE_MAX, AudioManagement_GetBalanceValFromStruct(&p, IQ_RX_PHASE));
}

/* -------------------------------------------------------------------------
 * Tests: GetNextValid
 * ------------------------------------------------------------------------- */
static void test_get_next_valid_first_point_valid(void)
{
    freq_adjust_point_t points[] = {
        { 1000, { { .gain = 10, .phase = 20 }, { { 0, 0 }, { 0, 0 } } } },
        { 0,    { { .gain = 0,  .phase = 0  }, { { 0, 0 }, { 0, 0 } } } }
    };
    TEST_ASSERT_EQUAL(0, AudioManagement_GetNextValid(points, 0, IQ_RX_GAIN));
}

static void test_get_next_valid_skip_invalid(void)
{
    freq_adjust_point_t points[] = {
        { 1000, { { .gain = IQ_BALANCE_OFF, .phase = IQ_BALANCE_OFF }, { { 0, 0 }, { 0, 0 } } } },
        { 2000, { { .gain = 50, .phase = IQ_BALANCE_OFF }, { { 0, 0 }, { 0, 0 } } } },
        { 0,    { { .gain = 0,  .phase = 0  }, { { 0, 0 }, { 0, 0 } } } }
    };
    TEST_ASSERT_EQUAL(1, AudioManagement_GetNextValid(points, 0, IQ_RX_GAIN));
}

static void test_get_next_valid_all_invalid_returns_stop(void)
{
    freq_adjust_point_t points[] = {
        { 1000, { { .gain = IQ_BALANCE_OFF, .phase = IQ_BALANCE_OFF }, { { 0, 0 }, { 0, 0 } } } },
        { 2000, { { .gain = IQ_BALANCE_OFF, .phase = IQ_BALANCE_OFF }, { { 0, 0 }, { 0, 0 } } } },
        { 0,    { { .gain = 0,  .phase = 0  }, { { 0, 0 }, { 0, 0 } } } }
    };
    TEST_ASSERT_EQUAL(2, AudioManagement_GetNextValid(points, 0, IQ_RX_GAIN));
}

static void test_get_next_valid_start_from_middle(void)
{
    freq_adjust_point_t points[] = {
        { 1000, { { .gain = IQ_BALANCE_OFF, .phase = IQ_BALANCE_OFF }, { { 0, 0 }, { 0, 0 } } } },
        { 2000, { { .gain = 50, .phase = IQ_BALANCE_OFF }, { { 0, 0 }, { 0, 0 } } } },
        { 3000, { { .gain = 60, .phase = IQ_BALANCE_OFF }, { { 0, 0 }, { 0, 0 } } } },
        { 0,    { { .gain = 0,  .phase = 0  }, { { 0, 0 }, { 0, 0 } } } }
    };
    TEST_ASSERT_EQUAL(1, AudioManagement_GetNextValid(points, 1, IQ_RX_GAIN));
}

static void test_get_next_valid_empty_array(void)
{
    freq_adjust_point_t points[] = {
        { 0, { { .gain = 0, .phase = 0 }, { { 0, 0 }, { 0, 0 } } } }
    };
    TEST_ASSERT_EQUAL(0, AudioManagement_GetNextValid(points, 0, IQ_RX_GAIN));
}

static void test_get_next_valid_different_param_types(void)
{
    freq_adjust_point_t points[] = {
        { 1000, { { .gain = IQ_BALANCE_OFF, .phase = 10 }, { { .gain = 20, .phase = 30 }, { .gain = 40, .phase = 50 } } } },
        { 0,    { { .gain = 0,  .phase = 0  }, { { 0, 0 }, { 0, 0 } } } }
    };
    TEST_ASSERT_EQUAL(0, AudioManagement_GetNextValid(points, 0, IQ_RX_PHASE));
    TEST_ASSERT_EQUAL(0, AudioManagement_GetNextValid(points, 0, IQ_TX_TRANS_ON_GAIN));
    TEST_ASSERT_EQUAL(0, AudioManagement_GetNextValid(points, 0, IQ_TX_TRANS_OFF_GAIN));
}

/* -------------------------------------------------------------------------
 * Tests: CalcAdjustInFreqRangeHelperNew
 * ------------------------------------------------------------------------- */
static void test_interp_exact_match_low(void)
{
    freq_adjust_point_t points[] = {
        { 1000, { { .gain = 10, .phase = 20 }, { { 0, 0 }, { 0, 0 } } } },
        { 2000, { { .gain = 30, .phase = 40 }, { { 0, 0 }, { 0, 0 } } } },
        { 0,    { { .gain = 0,  .phase = 0  }, { { 0, 0 }, { 0, 0 } } } }
    };
    TEST_ASSERT_FLOAT_EQ(10.0f / 2.0f, AudioManagement_CalcAdjustInFreqRangeHelperNew(points, IQ_RX_GAIN, 1000, 2.0f), 0.001f);
}

static void test_interp_exact_match_high(void)
{
    freq_adjust_point_t points[] = {
        { 1000, { { .gain = 10, .phase = 20 }, { { 0, 0 }, { 0, 0 } } } },
        { 2000, { { .gain = 30, .phase = 40 }, { { 0, 0 }, { 0, 0 } } } },
        { 0,    { { .gain = 0,  .phase = 0  }, { { 0, 0 }, { 0, 0 } } } }
    };
    TEST_ASSERT_FLOAT_EQ(30.0f / 2.0f, AudioManagement_CalcAdjustInFreqRangeHelperNew(points, IQ_RX_GAIN, 2000, 2.0f), 0.001f);
}

static void test_interp_midpoint(void)
{
    freq_adjust_point_t points[] = {
        { 1000, { { .gain = 10, .phase = 20 }, { { 0, 0 }, { 0, 0 } } } },
        { 2000, { { .gain = 30, .phase = 40 }, { { 0, 0 }, { 0, 0 } } } },
        { 0,    { { .gain = 0,  .phase = 0  }, { { 0, 0 }, { 0, 0 } } } }
    };
    TEST_ASSERT_FLOAT_EQ(10.0f, AudioManagement_CalcAdjustInFreqRangeHelperNew(points, IQ_RX_GAIN, 1500, 2.0f), 0.001f);
}

static void test_interp_below_range(void)
{
    freq_adjust_point_t points[] = {
        { 1000, { { .gain = 10, .phase = 20 }, { { 0, 0 }, { 0, 0 } } } },
        { 2000, { { .gain = 30, .phase = 40 }, { { 0, 0 }, { 0, 0 } } } },
        { 0,    { { .gain = 0,  .phase = 0  }, { { 0, 0 }, { 0, 0 } } } }
    };
    TEST_ASSERT_FLOAT_EQ(0.0f, AudioManagement_CalcAdjustInFreqRangeHelperNew(points, IQ_RX_GAIN, 500, 2.0f), 0.001f);
}

static void test_interp_above_range(void)
{
    freq_adjust_point_t points[] = {
        { 1000, { { .gain = 10, .phase = 20 }, { { 0, 0 }, { 0, 0 } } } },
        { 2000, { { .gain = 30, .phase = 40 }, { { 0, 0 }, { 0, 0 } } } },
        { 0,    { { .gain = 0,  .phase = 0  }, { { 0, 0 }, { 0, 0 } } } }
    };
    TEST_ASSERT_FLOAT_EQ(20.0f, AudioManagement_CalcAdjustInFreqRangeHelperNew(points, IQ_RX_GAIN, 2500, 2.0f), 0.001f);
}

static void test_interp_off_returns_zero(void)
{
    freq_adjust_point_t points[] = {
        { 1000, { { .gain = IQ_BALANCE_OFF, .phase = IQ_BALANCE_OFF }, { { 0, 0 }, { 0, 0 } } } },
        { 0,    { { .gain = 0,  .phase = 0  }, { { 0, 0 }, { 0, 0 } } } }
    };
    TEST_ASSERT_FLOAT_EQ(0.0f, AudioManagement_CalcAdjustInFreqRangeHelperNew(points, IQ_RX_GAIN, 1000, 2.0f), 0.001f);
}

static void test_interp_scaling_affects_result(void)
{
    freq_adjust_point_t points[] = {
        { 1000, { { .gain = 10, .phase = 20 }, { { 0, 0 }, { 0, 0 } } } },
        { 2000, { { .gain = 30, .phase = 40 }, { { 0, 0 }, { 0, 0 } } } },
        { 0,    { { .gain = 0,  .phase = 0  }, { { 0, 0 }, { 0, 0 } } } }
    };
    TEST_ASSERT(AudioManagement_CalcAdjustInFreqRangeHelperNew(points, IQ_RX_GAIN, 1500, 1.0f) !=
                 AudioManagement_CalcAdjustInFreqRangeHelperNew(points, IQ_RX_GAIN, 1500, 2.0f));
}

/* -------------------------------------------------------------------------
 * Tests: CalcIqGainAdjustVarHelper
 * ------------------------------------------------------------------------- */
static void test_gain_adjust_zero_adj(void)
{
    iq_float_t var = {0};
    AudioManagement_CalcIqGainAdjustVarHelper(&var, 0.0f);
    TEST_ASSERT_FLOAT_EQ(1.0f, var.i, 0.0001f);
    TEST_ASSERT_FLOAT_EQ(1.0f, var.q, 0.0001f);
}

static void test_gain_adjust_positive_adj(void)
{
    iq_float_t var = {0};
    AudioManagement_CalcIqGainAdjustVarHelper(&var, 0.1f);
    TEST_ASSERT_FLOAT_EQ(1.1f, var.i, 0.0001f);
    TEST_ASSERT_FLOAT_EQ(0.9f, var.q, 0.0001f);
}

static void test_gain_adjust_negative_adj(void)
{
    iq_float_t var = {0};
    AudioManagement_CalcIqGainAdjustVarHelper(&var, -0.2f);
    TEST_ASSERT_FLOAT_EQ(0.8f, var.i, 0.0001f);
    TEST_ASSERT_FLOAT_EQ(1.2f, var.q, 0.0001f);
}

static void test_gain_adjust_symmetric(void)
{
    iq_float_t var = {0};
    AudioManagement_CalcIqGainAdjustVarHelper(&var, 0.5f);
    TEST_ASSERT_FLOAT_EQ(1.5f, var.i, 0.0001f);
    TEST_ASSERT_FLOAT_EQ(0.5f, var.q, 0.0001f);
}

static void test_gain_adjust_preserves_sum(void)
{
    iq_float_t var = {0};
    float32_t adj = 0.3f;
    AudioManagement_CalcIqGainAdjustVarHelper(&var, adj);
    TEST_ASSERT_FLOAT_EQ(2.0f, var.i + var.q, 0.0001f);
}

/* -------------------------------------------------------------------------
 * Registration
 * ------------------------------------------------------------------------- */
void test_audio_management_balance_register(void)
{
    printf("--- audio_management balance ---\n");
    TEST_REGISTER(test_rx_gain);
    TEST_REGISTER(test_rx_phase);
    TEST_REGISTER(test_tx_trans_on_gain);
    TEST_REGISTER(test_tx_trans_off_gain);
    TEST_REGISTER(test_tx_trans_on_phase);
    TEST_REGISTER(test_tx_trans_off_phase);
    TEST_REGISTER(test_unknown_param_returns_off);
    TEST_REGISTER(test_negative_values_preserved);
    TEST_REGISTER(test_boundary_values);
    TEST_REGISTER(test_get_next_valid_first_point_valid);
    TEST_REGISTER(test_get_next_valid_skip_invalid);
    TEST_REGISTER(test_get_next_valid_all_invalid_returns_stop);
    TEST_REGISTER(test_get_next_valid_start_from_middle);
    TEST_REGISTER(test_get_next_valid_empty_array);
    TEST_REGISTER(test_get_next_valid_different_param_types);
    TEST_REGISTER(test_interp_exact_match_low);
    TEST_REGISTER(test_interp_exact_match_high);
    TEST_REGISTER(test_interp_midpoint);
    TEST_REGISTER(test_interp_below_range);
    TEST_REGISTER(test_interp_above_range);
    TEST_REGISTER(test_interp_off_returns_zero);
    TEST_REGISTER(test_interp_scaling_affects_result);
    TEST_REGISTER(test_gain_adjust_zero_adj);
    TEST_REGISTER(test_gain_adjust_positive_adj);
    TEST_REGISTER(test_gain_adjust_negative_adj);
    TEST_REGISTER(test_gain_adjust_symmetric);
    TEST_REGISTER(test_gain_adjust_preserves_sum);
    printf("\n");
}
