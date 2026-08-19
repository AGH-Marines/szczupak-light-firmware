#include "unity.h"
#include "current_sense.h"

#define VREF_MV 3300U
#define ADC_MAX 4095U

void setUp(void) {}

void tearDown(void) {}

static int32_t reference_to_ma(uint32_t r_shunt_mohm, uint32_t gain, uint16_t raw) {
    const uint64_t numerator = (uint64_t)raw * (uint64_t)VREF_MV * 1000ULL;
    const uint64_t denominator = (uint64_t)ADC_MAX * (uint64_t)gain * (uint64_t)r_shunt_mohm;
    return (int32_t)((numerator + (denominator / 2U)) / denominator);
}

static void test_current_sense_lower_bound(void) {
    const current_sense_t sense = {.scale_q16 =
                                       CURRENT_SENSE_CALC_SCALE(VREF_MV, ADC_MAX, 10U, 100U)};
    const int32_t current_ma = current_sense_to_ma(&sense, 0U);

    TEST_ASSERT_EQUAL_INT32(0, current_ma);
}

static void test_current_sense_half(void) {
    const current_sense_t sense = {.scale_q16 =
                                       CURRENT_SENSE_CALC_SCALE(VREF_MV, ADC_MAX, 10U, 100U)};
    const uint16_t raw = (uint16_t)(ADC_MAX / 2U);
    const int32_t current_ma = current_sense_to_ma(&sense, raw);

    TEST_ASSERT_INT32_WITHIN(2, reference_to_ma(10U, 100U, raw), current_ma);
}

static void test_current_sense_upper_bound(void) {
    const current_sense_t sense = {.scale_q16 =
                                       CURRENT_SENSE_CALC_SCALE(VREF_MV, ADC_MAX, 10U, 100U)};
    const int32_t current_ma = current_sense_to_ma(&sense, (uint16_t)ADC_MAX);

    TEST_ASSERT_INT32_WITHIN(2, reference_to_ma(10U, 100U, (uint16_t)ADC_MAX), current_ma);
}

static void test_current_sense_20mohm_lower_bound(void) {
    const current_sense_t sense = {.scale_q16 =
                                       CURRENT_SENSE_CALC_SCALE(VREF_MV, ADC_MAX, 20U, 100U)};
    const int32_t current_ma = current_sense_to_ma(&sense, 0U);

    TEST_ASSERT_EQUAL_INT32(0, current_ma);
}

static void test_current_sense_20mohm_half(void) {
    const current_sense_t sense = {.scale_q16 =
                                       CURRENT_SENSE_CALC_SCALE(VREF_MV, ADC_MAX, 20U, 100U)};
    const uint16_t raw = (uint16_t)(ADC_MAX / 2U);
    const int32_t current_ma = current_sense_to_ma(&sense, raw);

    TEST_ASSERT_INT32_WITHIN(2, reference_to_ma(20U, 100U, raw), current_ma);
}

static void test_current_sense_20mohm_upper_bound(void) {
    const current_sense_t sense = {.scale_q16 =
                                       CURRENT_SENSE_CALC_SCALE(VREF_MV, ADC_MAX, 20U, 100U)};
    const int32_t current_ma = current_sense_to_ma(&sense, (uint16_t)ADC_MAX);

    TEST_ASSERT_INT32_WITHIN(2, reference_to_ma(20U, 100U, (uint16_t)ADC_MAX), current_ma);
}

static void test_current_sense_1v(void) {
    const current_sense_t sense = {.scale_q16 =
                                       CURRENT_SENSE_CALC_SCALE(VREF_MV, ADC_MAX, 10U, 100U)};
    const uint16_t raw = (uint16_t)(1000 * ADC_MAX / VREF_MV);
    const int32_t current_ma = current_sense_to_ma(&sense, raw);

    TEST_ASSERT_INT32_WITHIN(2, reference_to_ma(10U, 100U, raw), current_ma);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_current_sense_lower_bound);
    RUN_TEST(test_current_sense_half);
    RUN_TEST(test_current_sense_upper_bound);

    RUN_TEST(test_current_sense_20mohm_lower_bound);
    RUN_TEST(test_current_sense_20mohm_half);
    RUN_TEST(test_current_sense_20mohm_upper_bound);

    RUN_TEST(test_current_sense_1v);

    return UNITY_END();
}
