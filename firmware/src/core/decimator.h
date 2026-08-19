#pragma once
#include <stdint.h>

// core 层：抽取器（纯逻辑）。ADC 定速 2000SPS，固件按 ÷2/÷4/÷20 抽取到 1000/500/100Hz。
// 每凑满 factor 个样本，输出其平均值（boxcar 低通 + 降采样，同时降噪）。
// 头文件式：目标与 native 测试都直接 include。
class Decimator {
public:
    // 设置抽取因子；仅接受 1/2/4/20，非法值按 1 处理。
    void setFactor(uint8_t factor) {
        factor_ = (factor == 2 || factor == 4 || factor == 20) ? factor : 1;
        count_ = 0;
        accum_ = 0;
    }

    // 每 push 一个样本；累计满 factor 个后返回 true 并在 out 输出平均值。
    bool push(int32_t sample, int32_t& out) {
        accum_ += sample;
        ++count_;
        if (count_ >= factor_) {
            out = int32_t(accum_ / factor_);
            count_ = 0;
            accum_ = 0;
            return true;
        }
        return false;
    }

    void reset() {
        count_ = 0;
        accum_ = 0;
    }
    uint8_t factor() const { return factor_; }

private:
    uint8_t factor_ = 1;
    uint8_t count_ = 0;
    int64_t accum_ = 0;  // 累加防溢出（24 位 × 20 远小于 int64）
};
