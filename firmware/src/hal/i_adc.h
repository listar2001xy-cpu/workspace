#pragma once
#include <stdint.h>

// ADC 抽象接口 —— 上层（core/app）只依赖此接口，不依赖具体芯片型号。
// 换芯片（ADS1220 → ADS1256 → AD7124-8）只需新增实现文件，上层零改动。
class IResistanceAdc {
public:
    virtual ~IResistanceAdc() = default;

    virtual bool init() = 0;                       // 上电初始化
    virtual bool selftest() = 0;                   // 自检：写读回寄存器验证 SPI
    virtual bool setRange(uint8_t rangeIdx) = 0;   // 换挡：改恒流 IDAC + PGA
    virtual bool startContinuous() = 0;            // 进入连续转换
    virtual bool dataReady() = 0;                  // DRDY 查询（或接中断）
    virtual int32_t readRaw() = 0;                 // 读原始 ADC 值
    virtual void setDrdyCallback(void (*cb)()) = 0;// 注册 DRDY 中断回调
};
