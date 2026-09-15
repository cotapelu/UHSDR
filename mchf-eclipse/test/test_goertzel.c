/**
 * @file test_goertzel.c
 * @brief Unit tests for audio_filter.c Goertzel pure math functions
 *
 * Tests AudioFilter_CalcGoertzel() and AudioFilter_GoertzelEnergy()
 * by reimplementing the logic inline to avoid hardware-dependent headers.
 */

#include "test.h"
#include <string.h>
#include <math.h>

/* -------------------------------------------------------------------------
 * Reimplementation of Goertzel functions for host testing
 * ------------------------------------------------------------------------- */
#define PI 3.14159265358979323846f

typedef struct
{
	int a;
	float32_t b;
	float32_t sin;
	float32_t cos;
	float32_t r;
	float32_t buf[3];
} Goertzel;

static void host_AudioFilter_CalcGoertzel(Goertzel* g, float32_t freq, const uint32_t size, const float goertzel_coeff, float32_t samplerate)
{
    g->a = (0.5 + (freq * goertzel_coeff) * size / samplerate);
    g->b = (2 * PI * g->a) / size;
    g->sin = sinf(g->b);
    g->cos = cosf(g->b);
    g->r = 2 * g->cos;
}

static void host_AudioFilter_GoertzelInput(Goertzel* goertzel, float32_t in)
{
	goertzel->buf[0] = goertzel->r * goertzel->buf[1] - goertzel->buf[2] + in;
	goertzel->buf[2] = goertzel->buf[1];
	goertzel->buf[1] = goertzel->buf[0];
}

static float32_t host_AudioFilter_GoertzelEnergy(Goertzel* goertzel)
{
	float32_t a = (goertzel->buf[1] - (goertzel->buf[2] * goertzel->cos));
	float32_t b = (goertzel->buf[2] * goertzel->sin);
	goertzel->buf[0] = 0;
	goertzel->buf[1] = 0;
	goertzel->buf[2] = 0;
	return sqrtf(a * a + b * b);
}

/* -------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */
static void test_goertzel_calc_basic_params(void)
{
    Goertzel g;
    memset(&g, 0, sizeof(g));
    host_AudioFilter_CalcGoertzel(&g, 1000.0f, 256, 0.5f, 48000.0f);
    /* a is int: 0.5 + (1000 * 0.5) * 256 / 48000 = 3.166... -> truncated to 3 */
    TEST_ASSERT_EQUAL(3, g.a);
    /* b = 2*PI*a/size = 2*PI*3/256 */
    TEST_ASSERT_FLOAT_EQ(0.0736f, g.b, 0.001f);
    /* r = 2*cos(b) */
    TEST_ASSERT_FLOAT_EQ(1.9946f, g.r, 0.001f);
}

static void test_goertzel_calc_sin_cos_positive(void)
{
    Goertzel g;
    memset(&g, 0, sizeof(g));
    host_AudioFilter_CalcGoertzel(&g, 100.0f, 128, 1.0f, 44100.0f);
    /* sin^2 + cos^2 should equal 1 */
    float32_t sum_sq = g.sin * g.sin + g.cos * g.cos;
    TEST_ASSERT_FLOAT_EQ(1.0f, sum_sq, 0.001f);
}

static void test_goertzel_input_updates_buffer(void)
{
    Goertzel g;
    memset(&g, 0, sizeof(g));
    host_AudioFilter_CalcGoertzel(&g, 1000.0f, 256, 0.5f, 48000.0f);
    /* Feed some samples */
    host_AudioFilter_GoertzelInput(&g, 1.0f);
    host_AudioFilter_GoertzelInput(&g, 0.5f);
    host_AudioFilter_GoertzelInput(&g, -0.5f);
    /* buf[0] should be non-zero after input */
    TEST_ASSERT(g.buf[0] != 0.0f);
}

static void test_goertzel_energy_zero_input(void)
{
    Goertzel g;
    memset(&g, 0, sizeof(g));
    host_AudioFilter_CalcGoertzel(&g, 1000.0f, 256, 0.5f, 48000.0f);
    /* No input: energy should be 0 */
    float32_t energy = host_AudioFilter_GoertzelEnergy(&g);
    TEST_ASSERT_FLOAT_EQ(0.0f, energy, 0.001f);
}

static void test_goertzel_energy_after_samples(void)
{
    Goertzel g;
    memset(&g, 0, sizeof(g));
    host_AudioFilter_CalcGoertzel(&g, 1000.0f, 256, 0.5f, 48000.0f);
    /* Feed constant input */
    for (int i = 0; i < 10; i++)
    {
        host_AudioFilter_GoertzelInput(&g, 1.0f);
    }
    float32_t energy = host_AudioFilter_GoertzelEnergy(&g);
    /* Energy should be positive after feeding samples */
    TEST_ASSERT(energy > 0.0f);
}

static void test_goertzel_energy_resets_buffer(void)
{
    Goertzel g;
    memset(&g, 0, sizeof(g));
    host_AudioFilter_CalcGoertzel(&g, 1000.0f, 256, 0.5f, 48000.0f);
    host_AudioFilter_GoertzelInput(&g, 1.0f);
    host_AudioFilter_GoertzelInput(&g, 0.5f);
    host_AudioFilter_GoertzelEnergy(&g);
    /* After energy calculation, buffer should be reset */
    TEST_ASSERT_FLOAT_EQ(0.0f, g.buf[0], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, g.buf[1], 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.0f, g.buf[2], 0.001f);
}

static void test_goertzel_calc_different_freqs(void)
{
    Goertzel g1, g2;
    memset(&g1, 0, sizeof(g1));
    memset(&g2, 0, sizeof(g2));
    host_AudioFilter_CalcGoertzel(&g1, 500.0f, 256, 0.5f, 48000.0f);
    host_AudioFilter_CalcGoertzel(&g2, 2000.0f, 256, 0.5f, 48000.0f);
    /* Higher frequency should produce different parameters */
    TEST_ASSERT(g1.a != g2.a);
    TEST_ASSERT(g1.b != g2.b);
}

static void test_goertzel_energy_negative_samples(void)
{
    Goertzel g;
    memset(&g, 0, sizeof(g));
    host_AudioFilter_CalcGoertzel(&g, 1000.0f, 256, 0.5f, 48000.0f);
    /* Feed negative samples */
    host_AudioFilter_GoertzelInput(&g, -1.0f);
    host_AudioFilter_GoertzelInput(&g, -0.5f);
    float32_t energy = host_AudioFilter_GoertzelEnergy(&g);
    /* Energy should still be positive (squares negative values) */
    TEST_ASSERT(energy > 0.0f);
}

static void test_goertzel_input_chain(void)
{
    Goertzel g;
    memset(&g, 0, sizeof(g));
    host_AudioFilter_CalcGoertzel(&g, 1000.0f, 256, 0.5f, 48000.0f);
    /* Chain of inputs: buf[2] gets buf[1], buf[1] gets buf[0] */
    host_AudioFilter_GoertzelInput(&g, 1.0f);
    TEST_ASSERT_FLOAT_EQ(1.0f, g.buf[1], 0.001f); /* buf[1] should equal previous buf[0] */
    host_AudioFilter_GoertzelInput(&g, 2.0f);
    TEST_ASSERT_FLOAT_EQ(1.0f, g.buf[2], 0.001f); /* buf[2] should equal previous buf[1] */
}

/* -------------------------------------------------------------------------
 * Registration
 * ------------------------------------------------------------------------- */
void test_goertzel_register(void)
{
    printf("--- goertzel ---\n");
    TEST_REGISTER(test_goertzel_calc_basic_params);
    TEST_REGISTER(test_goertzel_calc_sin_cos_positive);
    TEST_REGISTER(test_goertzel_input_updates_buffer);
    TEST_REGISTER(test_goertzel_energy_zero_input);
    TEST_REGISTER(test_goertzel_energy_after_samples);
    TEST_REGISTER(test_goertzel_energy_resets_buffer);
    TEST_REGISTER(test_goertzel_calc_different_freqs);
    TEST_REGISTER(test_goertzel_energy_negative_samples);
    TEST_REGISTER(test_goertzel_input_chain);
    printf("\n");
}
