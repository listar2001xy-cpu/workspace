#include <unity.h>

#include "core/autorange.h"

// fullScale = 1000，3 档（0/1/2），阈值 80%/20% = 800/200
Autorange makeRanger() {
    Autorange a;
    a.configure(1000, 3);
    return a;
}

void setUp() {}

void tearDown() {}

void test_keep_middle() {
    auto a = makeRanger();
    TEST_ASSERT_EQUAL_UINT8(1, a.decide(1, 500));   // 20%~80% 之间
    TEST_ASSERT_EQUAL_UINT8(1, a.decide(1, -500));  // 负值同样
}

void test_switch_up_when_near_full() {
    auto a = makeRanger();
    TEST_ASSERT_EQUAL_UINT8(2, a.decide(1, 900));   // > 800
    TEST_ASSERT_EQUAL_UINT8(2, a.decide(1, -900));  // 负值 |x|>800
}

void test_switch_down_when_too_small() {
    auto a = makeRanger();
    TEST_ASSERT_EQUAL_UINT8(0, a.decide(1, 100));  // < 200
}

void test_range_limits_no_overshoot() {
    auto a = makeRanger();
    TEST_ASSERT_EQUAL_UINT8(2, a.decide(2, 900));  // 最高档仍超量程 → 保持
    TEST_ASSERT_EQUAL_UINT8(0, a.decide(0, 50));   // 最低档仍过小 → 保持
}

void test_hysteresis_boundary() {
    auto a = makeRanger();
    TEST_ASSERT_EQUAL_UINT8(1, a.decide(1, 800));  // == 上阈值，不换（> 才换）
    TEST_ASSERT_EQUAL_UINT8(2, a.decide(1, 801));  // 超一点 → 换
    TEST_ASSERT_EQUAL_UINT8(1, a.decide(1, 200));  // == 下阈值，不换（< 才换）
    TEST_ASSERT_EQUAL_UINT8(0, a.decide(1, 199));  // 低一点 → 换
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_keep_middle);
    RUN_TEST(test_switch_up_when_near_full);
    RUN_TEST(test_switch_down_when_too_small);
    RUN_TEST(test_range_limits_no_overshoot);
    RUN_TEST(test_hysteresis_boundary);
    return UNITY_END();
}
