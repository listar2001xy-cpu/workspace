#pragma once
#include <SPI.h>

#include "hal/i_adc.h"

// hal 层：ADS1220 24-bit ΣΔ ADC 实现（IResistanceAdc 接口）。
// 恒流法测电阻：IDAC 恒流源流经传感器，ADC 测压差，Rx = V / I。
// 档位通过切换 IDAC 电流 + PGA 增益覆盖 1Ω~100kΩ（见 .cpp 内 kRanges 档位表）。
// 数据流：连续转换 2000SPS → DRDY 低电平 → RDATA 读 24bit，详见 docs/architecture.md。
class Ads1220Adc : public IResistanceAdc {
public:
    // csPin/drdyPin 见 board_pins.h；spiClk 默认 4MHz（ADS1220 上限）
    Ads1220Adc(int csPin, int drdyPin, uint32_t spiClk = 4000000);

    bool init() override;
    bool selftest() override;
    bool setRange(uint8_t rangeIdx) override;
    bool startContinuous() override;
    bool dataReady() override;
    int32_t readRaw() override;
    void setDrdyCallback(void (*cb)()) override;

    // ADC 满量程幅值（24 位有符号，2^23）；供 Autorange::configure 用
    static constexpr int32_t kFullScale = 1 << 23;
    static constexpr uint8_t kNumRanges = 8;

private:
    void sendCmd(uint8_t cmd);
    void writeReg(uint8_t addr, uint8_t val);
    uint8_t readReg(uint8_t addr);

    int cs_;
    int drdy_;
    SPISettings spiSettings_;
};
