#include "unity.h"

#include "led_brightness.h"

void setUp(void) {}

void tearDown(void) {}

static void test_led_brightness_min(void) {
    uint8_t percent = 0;
    TEST_ASSERT_EQUAL_UINT8(0U, led_brightness_clamp_percent(percent));
    TEST_ASSERT_EQUAL_UINT16(0U, led_brightness_to_pulse(percent));
}

static void test_led_brightness_max(void) {
    uint8_t percent = 100;
    TEST_ASSERT_EQUAL_UINT8(100U, led_brightness_clamp_percent(percent));
    TEST_ASSERT_EQUAL_UINT16(7999U, led_brightness_to_pulse(percent));
}

static void test_led_brightness_range(void) {
    uint8_t percent = 200;
    TEST_ASSERT_EQUAL_UINT16(7999U, led_brightness_to_pulse(percent));
}

static void test_led_brightness_clamp(void) {
    uint8_t percent = 0;
    TEST_ASSERT_EQUAL_UINT8(0U, led_brightness_clamp_percent(percent));
    percent = 50;
    TEST_ASSERT_EQUAL_UINT8(50U, led_brightness_clamp_percent(percent));
    percent = 100;
    TEST_ASSERT_EQUAL_UINT8(100U, led_brightness_clamp_percent(percent));
    percent = 150;
    TEST_ASSERT_EQUAL_UINT8(100U, led_brightness_clamp_percent(percent));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_led_brightness_min);
    RUN_TEST(test_led_brightness_max);
    RUN_TEST(test_led_brightness_range);
    RUN_TEST(test_led_brightness_clamp);

    return UNITY_END();
}
