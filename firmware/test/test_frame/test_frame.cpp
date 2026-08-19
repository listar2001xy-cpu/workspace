#include <unity.h>

#include "transport/frame.h"

using namespace transport;

void setUp() {}

void tearDown() {}

void test_crc8_known_value() {
    // CRC-8/MAXIM 标准校验值，独立验证算法正确性
    const char* msg = "123456789";
    TEST_ASSERT_EQUAL_UINT8(0xA1, crc8((const uint8_t*)msg, 9));
}

void test_encode_decode_roundtrip() {
    uint8_t payload[] = {0xDE, 0xAD, 0xBE, 0xEF};
    uint8_t buf[64];
    size_t n = encode(buf, sizeof(buf), 0x1234, payload, 4);
    TEST_ASSERT_EQUAL_INT(8, (int)n);  // 3 头 + 4 负载 + 1 CRC

    uint16_t seq = 0;
    const uint8_t* outPayload = nullptr;
    uint8_t outLen = 0;
    TEST_ASSERT_TRUE(decode(buf, n, seq, outPayload, outLen));
    TEST_ASSERT_EQUAL_UINT16(0x1234, seq);
    TEST_ASSERT_EQUAL_UINT8(4, outLen);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(payload, outPayload, 4);
}

void test_decode_detects_corruption() {
    uint8_t payload[] = {1, 2, 3};
    uint8_t buf[64];
    size_t n = encode(buf, sizeof(buf), 7, payload, 3);
    buf[4] ^= 0xFF;  // 破坏负载一个字节
    uint16_t seq;
    const uint8_t* p;
    uint8_t len;
    TEST_ASSERT_FALSE(decode(buf, n, seq, p, len));
}

void test_decode_rejects_bad_length() {
    uint8_t payload[] = {1, 2, 3};
    uint8_t buf[64];
    size_t n = encode(buf, sizeof(buf), 7, payload, 3);
    uint16_t seq;
    const uint8_t* p;
    uint8_t len;
    TEST_ASSERT_FALSE(decode(buf, n - 1, seq, p, len));  // 截断一字节
    TEST_ASSERT_FALSE(decode(buf, 2, seq, p, len));      // 短于帧头
}

void test_seq_endian_and_wrap() {
    uint8_t payload[] = {0};
    uint8_t buf[64];
    size_t n = encode(buf, sizeof(buf), 0xFFFF, payload, 1);
    TEST_ASSERT_EQUAL_UINT8(0xFF, buf[0]);  // 大端高字节
    TEST_ASSERT_EQUAL_UINT8(0xFF, buf[1]);  // 大端低字节
    uint16_t seq = 0;
    const uint8_t* p;
    uint8_t len;
    TEST_ASSERT_TRUE(decode(buf, n, seq, p, len));
    TEST_ASSERT_EQUAL_UINT16(0xFFFF, seq);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_crc8_known_value);
    RUN_TEST(test_encode_decode_roundtrip);
    RUN_TEST(test_decode_detects_corruption);
    RUN_TEST(test_decode_rejects_bad_length);
    RUN_TEST(test_seq_endian_and_wrap);
    return UNITY_END();
}
