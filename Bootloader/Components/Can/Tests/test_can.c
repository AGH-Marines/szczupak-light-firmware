#include "unity.h"

#include "can.h"

#define LIGHT_BOOT_COMMAND_ID 0x600U
#define LIGHT_BOOT_DATA_ID    0x601U

#define LIGHT_BOOT_COMMAND_DLC 8U
#define LIGHT_BOOT_DATA_DLC    8U

void setUp(void) {}

void tearDown(void) {}

static void test_boot_command_decode(void) {
    const can_frame_t frame = {.id = LIGHT_BOOT_COMMAND_ID,
                               .dlc = LIGHT_BOOT_COMMAND_DLC,
                               .data = {0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    can_boot_command_t command = {0};

    TEST_ASSERT_TRUE(can_decode_boot_command(&frame, &command));
    TEST_ASSERT_EQUAL(CAN_BOOT_COMMAND_ENTER_UPDATE, command.type);
    TEST_ASSERT_EQUAL_UINT8(0x01, command.target_mask);
}

static void test_boot_command_decode_null(void) {
    const can_frame_t frame = {0};
    can_boot_command_t command = {0};

    TEST_ASSERT_FALSE(can_decode_boot_command(&frame, NULL));
    TEST_ASSERT_FALSE(can_decode_boot_command(NULL, &command));
    TEST_ASSERT_FALSE(can_decode_boot_command(NULL, NULL));
}

static void test_boot_command_header_decode(void) {
    const can_frame_t frame = {.id = LIGHT_BOOT_COMMAND_ID,
                               .dlc = LIGHT_BOOT_COMMAND_DLC,
                               .data = {0x02, 0x01, 0x00, 0x01, 0x45, 0x23, 0x01, 0x00}};
    can_boot_command_t command = {0};

    TEST_ASSERT_TRUE(can_decode_boot_command(&frame, &command));
    TEST_ASSERT_EQUAL(CAN_BOOT_COMMAND_HEADER, command.type);
    TEST_ASSERT_EQUAL_UINT8(0x01, command.target_mask);
    TEST_ASSERT_EQUAL_UINT8(0, command.part);
    TEST_ASSERT_EQUAL_UINT8(0x01, command.version);
    TEST_ASSERT_EQUAL_UINT32(0x00012345, command.value);
}

static void test_boot_command_header_crc_decode(void) {
    const can_frame_t frame = {.id = LIGHT_BOOT_COMMAND_ID,
                               .dlc = LIGHT_BOOT_COMMAND_DLC,
                               .data = {0x02, 0x01, 0x01, 0x01, 0xEF, 0xCD, 0xAB, 0x89}};
    can_boot_command_t command = {0};

    TEST_ASSERT_TRUE(can_decode_boot_command(&frame, &command));
    TEST_ASSERT_EQUAL(CAN_BOOT_COMMAND_HEADER, command.type);
    TEST_ASSERT_EQUAL_UINT8(0x01, command.target_mask);
    TEST_ASSERT_EQUAL_UINT8(0x01, command.part);
    TEST_ASSERT_EQUAL_UINT8(0x01, command.version);
    TEST_ASSERT_EQUAL_UINT32(0x89ABCDEF, command.value);
}

static void test_boot_data_decode(void) {
    const can_frame_t frame = {.id = LIGHT_BOOT_DATA_ID,
                               .dlc = LIGHT_BOOT_DATA_DLC,
                               .data = {0x05, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77}};
    can_boot_data_t data = {0};

    TEST_ASSERT_TRUE(can_decode_boot_data(&frame, &data));
    TEST_ASSERT_EQUAL_UINT8(0x05, data.frame_index);
    TEST_ASSERT_EQUAL_UINT8(0x11, data.data[0]);
    TEST_ASSERT_EQUAL_UINT8(0x22, data.data[1]);
    TEST_ASSERT_EQUAL_UINT8(0x33, data.data[2]);
    TEST_ASSERT_EQUAL_UINT8(0x44, data.data[3]);
    TEST_ASSERT_EQUAL_UINT8(0x55, data.data[4]);
    TEST_ASSERT_EQUAL_UINT8(0x66, data.data[5]);

    TEST_ASSERT_EQUAL_UINT8(0x77, data.data[6]);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_boot_command_decode);
    RUN_TEST(test_boot_command_decode_null);
    RUN_TEST(test_boot_command_header_decode);
    RUN_TEST(test_boot_command_header_crc_decode);
    RUN_TEST(test_boot_data_decode);

    return UNITY_END();
}
