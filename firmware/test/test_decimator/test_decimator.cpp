#include <unity.h>

#include "core/decimator.h"

void setUp() {}

void tearDown() {}

void test_factor2_average() {
    Decimator d;
    d.setFactor(2);
    int32_t out = 0;
    TEST_ASSERT_FALSE(d.push(10, out));  // 第 1 个，未满
    TEST_ASSERT_TRUE(d.push(20, out));   // 第 2 个，满
    TEST_ASSERT_EQUAL_INT(15, out);      // (10+20)/2
}

void test_factor4() {
    Decimator d;
    d.setFactor(4);
    int32_t out = 0;
    int32_t samples[] = {10, 20, 30, 40};
    for (int i = 0; i < 3; ++i) TEST_ASSERT_FALSE(d.push(samples[i], out));
    TEST_ASSERT_TRUE(d.push(samples[3], out));
    TEST_ASSERT_EQUAL_INT(25, out);  // (10+20+30+40)/4
}

void test_factor20() {
    Decimator d;
    d.setFactor(20);
    int32_t out = 0;
    for (int i = 0; i < 19; ++i) TEST_ASSERT_FALSE(d.push(5, out));
    TEST_ASSERT_TRUE(d.push(5, out));
    TEST_ASSERT_EQUAL_INT(5, out);  // 20×5/20 = 5
}

void test_continues_after_output() {
    Decimator d;
    d.setFactor(2);
    int32_t out = 0;
    d.push(100, out);
    d.push(200, out);  // 输出 150
    TEST_ASSERT_EQUAL_INT(150, out);
    // 新一轮
    TEST_ASSERT_FALSE(d.push(1000, out));
    TEST_ASSERT_TRUE(d.push(2000, out));
    TEST_ASSERT_EQUAL_INT(1500, out);
}

void test_invalid_factor_treated_as_1() {
    Decimator d;
    d.setFactor(99);
    int32_t out = 0;
    TEST_ASSERT_TRUE(d.push(42, out));
    TEST_ASSERT_EQUAL_INT(42, out);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_factor2_average);
    RUN_TEST(test_factor4);
    RUN_TEST(test_factor20);
    RUN_TEST(test_continues_after_output);
    RUN_TEST(test_invalid_factor_treated_as_1);
    return UNITY_END();
}
