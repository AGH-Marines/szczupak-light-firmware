#include "unity.h"

#include "can.h"

void setUp(void) {}

void tearDown(void) {}

static void test_command_encode_decode(void) {
    const can_command_t in = {
        .led1_percent = 50,
        .led2_percent = 0,
        .led3_percent = 25,
        .enable = true,
        .fault_reset = false,
    };

    can_frame_t frame = {0};
    can_command_t out = {0};

    TEST_ASSERT_TRUE(can_encode_command(&in, &frame));

    TEST_ASSERT_EQUAL_UINT32(0x100U, frame.id);
    TEST_ASSERT_EQUAL_UINT8(4U, frame.dlc);

    TEST_ASSERT_TRUE(can_decode_command(&frame, &out));
    TEST_ASSERT_EQUAL_UINT8(in.led1_percent, out.led1_percent);
    TEST_ASSERT_EQUAL_UINT8(in.led2_percent, out.led2_percent);
    TEST_ASSERT_EQUAL_UINT8(in.led3_percent, out.led3_percent);
    TEST_ASSERT_TRUE(out.enable);
    TEST_ASSERT_FALSE(out.fault_reset);
}

static void test_command_null(void) {
    can_command_t in = {0};
    can_frame_t frame = {0};
    can_command_t out = {0};

    TEST_ASSERT_FALSE(can_encode_command(NULL, &frame));
    TEST_ASSERT_FALSE(can_encode_command(&in, NULL));
    TEST_ASSERT_FALSE(can_encode_command(NULL, NULL));

    TEST_ASSERT_FALSE(can_decode_command(NULL, &out));
    TEST_ASSERT_FALSE(can_decode_command(&frame, NULL));
    TEST_ASSERT_FALSE(can_decode_command(NULL, NULL));
}

static void test_command_ranges(void) {
    can_command_t in = {
        .led1_percent = 101,
        .led2_percent = 0,
        .led3_percent = 25,
        .enable = true,
        .fault_reset = false,
    };
    can_frame_t frame = {0};

    TEST_ASSERT_FALSE(can_encode_command(&in, &frame));

    in.led1_percent = 0;
    in.led2_percent = 150;
    TEST_ASSERT_FALSE(can_encode_command(&in, &frame));

    in.led2_percent = 0;
    in.led3_percent = 125;
    TEST_ASSERT_FALSE(can_encode_command(&in, &frame));
}

static void test_power_status_encode_decode(void) {
    const can_power_status_t in = {
        .usb_voltage_mv = 4500,
        .power_voltage_mv = 12010,
        .led1_percent = 50,
        .led2_percent = 100,
        .led3_percent = 0,
        .enable = 1,
    };

    can_frame_t frame = {0};
    can_power_status_t out = {0};

    TEST_ASSERT_TRUE(can_encode_power_status(&in, &frame));

    TEST_ASSERT_EQUAL_UINT32(0x110U, frame.id);
    TEST_ASSERT_EQUAL_UINT8(8U, frame.dlc);

    TEST_ASSERT_TRUE(can_decode_power_status(&frame, &out));

    TEST_ASSERT_EQUAL_UINT16(in.usb_voltage_mv, out.usb_voltage_mv);
    TEST_ASSERT_EQUAL_UINT16(in.power_voltage_mv, out.power_voltage_mv);
    TEST_ASSERT_EQUAL_UINT8(in.led1_percent, out.led1_percent);
    TEST_ASSERT_EQUAL_UINT8(in.led2_percent, out.led2_percent);
    TEST_ASSERT_EQUAL_UINT8(in.led3_percent, out.led3_percent);
    TEST_ASSERT_TRUE(out.enable);
}

static void test_power_status_null(void) {
    can_power_status_t in = {0};
    can_frame_t frame = {0};
    can_power_status_t out = {0};

    TEST_ASSERT_FALSE(can_encode_power_status(NULL, &frame));
    TEST_ASSERT_FALSE(can_encode_power_status(&in, NULL));
    TEST_ASSERT_FALSE(can_encode_power_status(NULL, NULL));

    TEST_ASSERT_FALSE(can_decode_power_status(NULL, &out));
    TEST_ASSERT_FALSE(can_decode_power_status(&frame, NULL));
    TEST_ASSERT_FALSE(can_decode_power_status(NULL, NULL));
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

static void test_current_status_encode_decode(void) {
    const can_current_status_t in = {
        .led1_current_ma = 1000,
        .led2_current_ma = 331,
        .led3_current_ma = 12,
    };

    can_frame_t frame = {0};
    can_current_status_t out = {0};

    TEST_ASSERT_TRUE(can_encode_current_status(&in, &frame));
    TEST_ASSERT_EQUAL_UINT32(0x111U, frame.id);
    TEST_ASSERT_EQUAL_UINT8(6U, frame.dlc);

    TEST_ASSERT_TRUE(can_decode_current_status(&frame, &out));
    TEST_ASSERT_EQUAL_UINT16(in.led1_current_ma, out.led1_current_ma);
    TEST_ASSERT_EQUAL_UINT16(in.led2_current_ma, out.led2_current_ma);
    TEST_ASSERT_EQUAL_UINT16(in.led3_current_ma, out.led3_current_ma);
}

static void test_current_status_null(void) {
    can_current_status_t in = {0};
    can_frame_t frame = {0};
    can_current_status_t out = {0};

    TEST_ASSERT_FALSE(can_encode_current_status(NULL, &frame));
    TEST_ASSERT_FALSE(can_encode_current_status(&in, NULL));
    TEST_ASSERT_FALSE(can_encode_current_status(NULL, NULL));

    TEST_ASSERT_FALSE(can_decode_current_status(NULL, &out));
    TEST_ASSERT_FALSE(can_decode_current_status(&frame, NULL));
    TEST_ASSERT_FALSE(can_decode_current_status(NULL, NULL));
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

static void test_fault_status_encode_decode(void) {
    const can_fault_status_t in = {
        .led1_overcurrent = false,
        .led2_overcurrent = false,
        .led3_overcurrent = true,
        .led1_no_load = true,
        .led2_no_load = false,
        .led3_no_load = true,
        .usb_undervoltage = false,
        .power_undervoltage = true,
    };
    can_frame_t frame = {0};
    can_fault_status_t out = {0};

    TEST_ASSERT_TRUE(can_encode_fault_status(&in, &frame));
    TEST_ASSERT_EQUAL_UINT32(0x112, frame.id);
    TEST_ASSERT_EQUAL_UINT8(1U, frame.dlc);

    TEST_ASSERT_TRUE(can_decode_fault_status(&frame, &out));
    TEST_ASSERT_FALSE(out.led1_overcurrent);
    TEST_ASSERT_FALSE(out.led2_overcurrent);
    TEST_ASSERT_TRUE(out.led3_overcurrent);
    TEST_ASSERT_TRUE(out.led1_no_load);
    TEST_ASSERT_FALSE(out.led2_no_load);
    TEST_ASSERT_TRUE(out.led3_no_load);

    TEST_ASSERT_FALSE(out.usb_undervoltage);
    TEST_ASSERT_TRUE(out.power_undervoltage);
}

static void test_fault_status_null(void) {
    can_fault_status_t in = {0};
    can_frame_t frame = {0};
    can_fault_status_t out = {0};

    TEST_ASSERT_FALSE(can_encode_fault_status(NULL, &frame));
    TEST_ASSERT_FALSE(can_encode_fault_status(&in, NULL));
    TEST_ASSERT_FALSE(can_encode_fault_status(NULL, NULL));

    TEST_ASSERT_FALSE(can_decode_fault_status(NULL, &out));
    TEST_ASSERT_FALSE(can_decode_fault_status(&frame, NULL));
    TEST_ASSERT_FALSE(can_decode_fault_status(NULL, NULL));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_command_encode_decode);
    RUN_TEST(test_command_null);
    RUN_TEST(test_command_ranges);

    RUN_TEST(test_power_status_encode_decode);
    RUN_TEST(test_power_status_null);
    RUN_TEST(test_power_status_ranges);

    RUN_TEST(test_current_status_encode_decode);
    RUN_TEST(test_current_status_null);
    RUN_TEST(test_current_status_ranges);

    RUN_TEST(test_fault_status_encode_decode);
    RUN_TEST(test_fault_status_null);

    return UNITY_END();
}
