#include "unity.h"
#include "voltage_sense.h"

#define VREF_MV 3300U
#define ADC_MAX 4095U

void setUp(void) {}

void tearDown(void) {}

static int32_t reference_to_mv(uint32_t r_high_ohm, uint32_t r_low_ohm, uint16_t raw) {
    const uint64_t numerator =
        (uint64_t)raw * (uint64_t)VREF_MV * ((uint64_t)r_high_ohm + (uint64_t)r_low_ohm);
    const uint64_t denominator = (uint64_t)ADC_MAX * (uint64_t)r_low_ohm;
    return (int32_t)((numerator + (denominator / 2U)) / denominator);
}

static void test_voltage_lower_bound(void) {
    const voltage_sense_t sense = {.scale_q16 =
                                       VOLTAGE_SENSE_CALC_SCALE(VREF_MV, ADC_MAX, 1000U, 1000U)};
    const int32_t voltage_mv = voltage_sense_to_mv(&sense, 0U);

    TEST_ASSERT_EQUAL_INT32(0, voltage_mv);
}

static void test_voltage_upper_bound(void) {
    const voltage_sense_t sense = {.scale_q16 =
                                       VOLTAGE_SENSE_CALC_SCALE(VREF_MV, ADC_MAX, 1000U, 1000U)};
    const int32_t voltage_mv = voltage_sense_to_mv(&sense, (uint16_t)ADC_MAX);

    TEST_ASSERT_INT32_WITHIN(2, reference_to_mv(1000U, 1000U, (uint16_t)ADC_MAX), voltage_mv);
}

static void test_voltage_12V_lower_bound(void) {
    const voltage_sense_t sense = {.scale_q16 =
                                       VOLTAGE_SENSE_CALC_SCALE(VREF_MV, ADC_MAX, 100000U, 10000U)};
    const int32_t voltage_mv = voltage_sense_to_mv(&sense, 0U);

    TEST_ASSERT_EQUAL_INT32(0, voltage_mv);
}

static void test_voltage_12V_half(void) {
    const voltage_sense_t sense = {.scale_q16 =
                                       VOLTAGE_SENSE_CALC_SCALE(VREF_MV, ADC_MAX, 100000U, 10000U)};
    const uint16_t raw = (uint16_t)(ADC_MAX / 2U);
    const int32_t voltage_mv = voltage_sense_to_mv(&sense, raw);

    TEST_ASSERT_INT32_WITHIN(2, reference_to_mv(100000U, 10000U, raw), voltage_mv);
}

static void test_voltage_12V_upper_bound(void) {
    const voltage_sense_t sense = {.scale_q16 =
                                       VOLTAGE_SENSE_CALC_SCALE(VREF_MV, ADC_MAX, 100000U, 10000U)};
    const int32_t voltage_mv = voltage_sense_to_mv(&sense, (uint16_t)ADC_MAX);

    TEST_ASSERT_INT32_WITHIN(2, reference_to_mv(100000U, 10000U, (uint16_t)ADC_MAX), voltage_mv);
}

static void test_voltage_usb_lower_bound(void) {
    const voltage_sense_t sense = {
        .scale_q16 = VOLTAGE_SENSE_CALC_SCALE(VREF_MV, ADC_MAX, 100000U, 100000U)};
    const int32_t voltage_mv = voltage_sense_to_mv(&sense, 0U);

    TEST_ASSERT_EQUAL_INT32(0, voltage_mv);
}

static void test_voltage_usb_half(void) {
    const voltage_sense_t sense = {
        .scale_q16 = VOLTAGE_SENSE_CALC_SCALE(VREF_MV, ADC_MAX, 100000U, 100000U)};
    const uint16_t raw = (uint16_t)(ADC_MAX / 2U);
    const int32_t voltage_mv = voltage_sense_to_mv(&sense, raw);

    TEST_ASSERT_INT32_WITHIN(2, reference_to_mv(100000U, 100000U, raw), voltage_mv);
}

static void test_voltage_usb_upper_bound(void) {
    const voltage_sense_t sense = {
        .scale_q16 = VOLTAGE_SENSE_CALC_SCALE(VREF_MV, ADC_MAX, 100000U, 100000U)};
    const int32_t voltage_mv = voltage_sense_to_mv(&sense, (uint16_t)ADC_MAX);

    TEST_ASSERT_INT32_WITHIN(2, reference_to_mv(100000U, 100000U, (uint16_t)ADC_MAX), voltage_mv);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_voltage_lower_bound);
    RUN_TEST(test_voltage_upper_bound);
    RUN_TEST(test_voltage_12V_lower_bound);
    RUN_TEST(test_voltage_12V_half);
    RUN_TEST(test_voltage_12V_upper_bound);
    RUN_TEST(test_voltage_usb_lower_bound);
    RUN_TEST(test_voltage_usb_half);
    RUN_TEST(test_voltage_usb_upper_bound);

    return UNITY_END();
}
