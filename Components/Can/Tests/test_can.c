#include "unity.h"

#include "can.h"

#define LIGHT_BOOT_COMMAND_ID  0x600U
#define LIGHT_BOOT_COMMAND_DLC 8U

void setUp(void) {}

void tearDown(void) {}

static void test_command_null(void) {
    can_frame_t frame = {0};
    can_command_t out = {0};

    TEST_ASSERT_FALSE(can_decode_command(NULL, &out));
    TEST_ASSERT_FALSE(can_decode_command(&frame, NULL));
    TEST_ASSERT_FALSE(can_decode_command(NULL, NULL));
}

static void test_power_status_null(void) {
    can_power_status_t in = {0};
    can_frame_t frame = {0};

    TEST_ASSERT_FALSE(can_encode_power_status(NULL, &frame));
    TEST_ASSERT_FALSE(can_encode_power_status(&in, NULL));
    TEST_ASSERT_FALSE(can_encode_power_status(NULL, NULL));
}

static void test_power_status_ranges(void) {
    can_power_status_t in = {
        .usb_voltage_mv = 4000,
        .power_voltage_mv = 18010,
        .led1_percent = 50,
        .led2_percent = 100,
        .led3_percent = 0,
        .enable = 1,
    };

    can_frame_t frame = {0};

    TEST_ASSERT_FALSE(can_encode_power_status(&in, &frame));

    in.usb_voltage_mv = 10109;
    in.power_voltage_mv = 0;
    TEST_ASSERT_FALSE(can_encode_power_status(&in, &frame));

    in.usb_voltage_mv = 10;
    in.led1_percent = 101;
    TEST_ASSERT_FALSE(can_encode_power_status(&in, &frame));

    in.led1_percent = 100;
    in.led2_percent = 101;
    TEST_ASSERT_FALSE(can_encode_power_status(&in, &frame));

    in.led2_percent = 100;
    in.led3_percent = 101;
    TEST_ASSERT_FALSE(can_encode_power_status(&in, &frame));
}

static void test_current_status_null(void) {
    can_current_status_t in = {0};
    can_frame_t frame = {0};

    TEST_ASSERT_FALSE(can_encode_current_status(NULL, &frame));
    TEST_ASSERT_FALSE(can_encode_current_status(&in, NULL));
    TEST_ASSERT_FALSE(can_encode_current_status(NULL, NULL));
}

static void test_current_status_ranges(void) {
    can_current_status_t in = {
        .led1_current_ma = 5001,
        .led2_current_ma = 0,
        .led3_current_ma = 0,
    };
    can_frame_t frame = {0};
    TEST_ASSERT_FALSE(can_encode_current_status(&in, &frame));

    in.led1_current_ma = 0;
    in.led2_current_ma = 6000;
    TEST_ASSERT_FALSE(can_encode_current_status(&in, &frame));

    in.led2_current_ma = 0;
    in.led3_current_ma = 10000;
    TEST_ASSERT_FALSE(can_encode_current_status(&in, &frame));
}

static void test_fault_status_null(void) {
    can_fault_status_t in = {0};
    can_frame_t frame = {0};

    TEST_ASSERT_FALSE(can_encode_fault_status(NULL, &frame));
    TEST_ASSERT_FALSE(can_encode_fault_status(&in, NULL));
    TEST_ASSERT_FALSE(can_encode_fault_status(NULL, NULL));
}

static void test_boot_command_decode_null(void) {
    const can_frame_t frame = {0};
    can_boot_command_t command = {0};

    TEST_ASSERT_FALSE(can_decode_boot_command(&frame, NULL));
    TEST_ASSERT_FALSE(can_decode_boot_command(NULL, &command));
    TEST_ASSERT_FALSE(can_decode_boot_command(NULL, NULL));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_command_null);

    RUN_TEST(test_power_status_null);
    RUN_TEST(test_power_status_ranges);

    RUN_TEST(test_current_status_null);
    RUN_TEST(test_current_status_ranges);

    RUN_TEST(test_fault_status_null);

    RUN_TEST(test_boot_command_decode_null);
    return UNITY_END();
}
