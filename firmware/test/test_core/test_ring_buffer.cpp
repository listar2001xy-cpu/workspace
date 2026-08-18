#include <unity.h>

#include "core/ring_buffer.h"

// 用 RingBuffer<int, 4>：原始 16 槽，有效容量 15。
using Buffer = RingBuffer<int, 4>;

void setUp() {}

void tearDown() {}

void test_push_pop_roundtrip() {
    Buffer buf;
    TEST_ASSERT_TRUE(buf.push(10));
    TEST_ASSERT_TRUE(buf.push(20));
    TEST_ASSERT_EQUAL_INT(2, (int)buf.count());

    int out = 0;
    TEST_ASSERT_TRUE(buf.pop(out));
    TEST_ASSERT_EQUAL_INT(10, out);
    TEST_ASSERT_TRUE(buf.pop(out));
    TEST_ASSERT_EQUAL_INT(20, out);
    TEST_ASSERT_TRUE(buf.empty());
}

void test_full_and_overflow_counter() {
    Buffer buf;
    for (int i = 0; i < (int)buf.capacity(); ++i) {
        TEST_ASSERT_TRUE(buf.push(i));
    }
    TEST_ASSERT_TRUE(buf.full());
    TEST_ASSERT_EQUAL_INT(0, (int)buf.overflowCount());

    // 再写入应失败并计数溢出（数据完整性：绝不静默丢）
    TEST_ASSERT_FALSE(buf.push(999));
    TEST_ASSERT_EQUAL_INT(1, (int)buf.overflowCount());
    TEST_ASSERT_FALSE(buf.push(999));
    TEST_ASSERT_EQUAL_INT(2, (int)buf.overflowCount());
}

void test_wrap_around() {
    Buffer buf;
    for (int i = 0; i < 10; ++i) {
        TEST_ASSERT_TRUE(buf.push(i));
    }
    int out = 0;
    for (int i = 0; i < 10; ++i) {
        TEST_ASSERT_TRUE(buf.pop(out));
        TEST_ASSERT_EQUAL_INT(i, out);
    }
    TEST_ASSERT_TRUE(buf.empty());

    // 环绕后再写入读回，值应对
    for (int i = 100; i < 110; ++i) {
        TEST_ASSERT_TRUE(buf.push(i));
    }
    for (int i = 100; i < 110; ++i) {
        TEST_ASSERT_TRUE(buf.pop(out));
        TEST_ASSERT_EQUAL_INT(i, out);
    }
}

void test_pop_empty_fails() {
    Buffer buf;
    int out = 42;
    TEST_ASSERT_FALSE(buf.pop(out));
    TEST_ASSERT_EQUAL_INT(42, out);  // 空时不得修改 out
}

void test_reset() {
    Buffer buf;
    buf.push(1);
    buf.push(2);
    buf.reset();
    TEST_ASSERT_TRUE(buf.empty());
    TEST_ASSERT_EQUAL_INT(0, (int)buf.count());
    TEST_ASSERT_EQUAL_INT(0, (int)buf.overflowCount());
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_push_pop_roundtrip);
    RUN_TEST(test_full_and_overflow_counter);
    RUN_TEST(test_wrap_around);
    RUN_TEST(test_pop_empty_fails);
    RUN_TEST(test_reset);
    return UNITY_END();
}
