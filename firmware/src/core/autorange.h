#pragma once
#include <stdint.h>

// core 层：自动换挡决策（纯逻辑）。
// 恒流法测电阻 Rx = V/I，1Ω~100kΩ 靠多档 IDAC 电流 + PGA 增益覆盖。
// 档位 0..numRanges-1：0 = 最低阻值档（最大电流），越高 = 越高阻值档。
// 读数超上阈值(80% 满量程) → 换更高阻值档（减小电流）；低于下阈值(20%) → 换更低阻值档。
// 双阈值自带滞回，避免在边界来回抖动。
class Autorange {
public:
    static constexpr int32_t kUpperPercent = 80;  // 换高档阈值（满量程 %）
    static constexpr int32_t kLowerPercent = 20;  // 换低档阈值（满量程 %）

    // fullScale = ADC 满量程幅值（24 位 = 2^23）。numRanges = 档位数。
    void configure(int32_t fullScale, uint8_t numRanges) {
        fullScale_ = fullScale > 0 ? fullScale : 1;
        numRanges_ = numRanges > 0 ? numRanges : 1;
        upper_ = fullScale_ * kUpperPercent / 100;
        lower_ = fullScale_ * kLowerPercent / 100;
    }

    // 输入当前档位 + 原始读数，返回新档位（可能相同，不越界）。
    uint8_t decide(uint8_t range, int32_t raw) const {
        if (range >= numRanges_) range = numRanges_ - 1;
        int32_t mag = raw < 0 ? -raw : raw;  // |raw|
        if (mag > upper_) {
            return (range + 1 < numRanges_) ? range + 1 : range;
        }
        if (mag < lower_ && range > 0) {
            return range - 1;
        }
        return range;
    }

    uint8_t numRanges() const { return numRanges_; }

private:
    int32_t fullScale_ = 1;
    int32_t upper_ = 0;
    int32_t lower_ = 0;
    uint8_t numRanges_ = 1;
};
