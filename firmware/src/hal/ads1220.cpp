#include "hal/ads1220.h"

#include <Arduino.h>

namespace {

// 命令
constexpr uint8_t kCmdReset = 0x06;
constexpr uint8_t kCmdStart = 0x08;  // START/SYNC：启动连续转换
constexpr uint8_t kCmdRdata = 0x10;
constexpr uint8_t kCmdRreg = 0x20;  // | (addr << 2)
constexpr uint8_t kCmdWreg = 0x40;  // | (addr << 2)

// 寄存器地址
constexpr uint8_t kRegConfig0 = 0x00;  // MUX / GAIN / PGA_BYPASS
constexpr uint8_t kRegConfig1 = 0x01;  // 数据率 / 模式 / 基准
constexpr uint8_t kRegConfig2 = 0x02;  // IDAC 电流 / 其他
constexpr uint8_t kRegConfig3 = 0x03;  // IDAC 路由

// CONFIG1：DR=2000SPS(0111) + 普通模式(0) + 连续转换(1) + 内部 2.048V 基准(00)
constexpr uint8_t kCfg1Cont2000 = 0x74;  // 0b0111_0_1_00

// CONFIG3：IDAC1 路由到 AIN0（恒流源输出），IDAC2 关闭
constexpr uint8_t kCfg3Idac1Ain0 = 0x20;  // 0b001_000_00

// 档位表：{IDAC 电流字段(CONFIG2[2:0]), PGA 增益字段(CONFIG0[3:1], 0=1x 1=2x 2=4x)}
// 满量程 Rx_fs = 2.048V / (I × 2^gain)。相邻档约 2× 间隔，配合 Autorange 的
// 80/20 双阈值滞回不会在边界来回抖动（>4× 间隔会与 80/20 冲突导致震荡）。
// [待确认] 电流/PGA 取值按实际传感器阻值范围与精度目标校准。
struct RangeCfg {
    uint8_t idac;  // 0x01=10µA 0x02=50µA 0x03=100µA 0x04=250µA 0x05=500µA 0x06=1000µA
    uint8_t gain;  // GAIN 字段：0=1x 1=2x 2=4x
};
constexpr RangeCfg kRanges[] = {
    {0x06, 0},  // 1000µA × 1 → Rx_fs ≈ 2.0kΩ
    {0x05, 0},  //  500µA × 1 → ≈ 4.1kΩ
    {0x04, 0},  //  250µA × 1 → ≈ 8.2kΩ
    {0x03, 0},  //  100µA × 1 → ≈ 20.5kΩ
    {0x02, 0},  //   50µA × 1 → ≈ 41kΩ
    {0x01, 2},  //   10µA × 4 → ≈ 51kΩ
    {0x01, 1},  //   10µA × 2 → ≈ 102kΩ
    {0x01, 0},  //   10µA × 1 → ≈ 205kΩ
};
static_assert(Ads1220Adc::kNumRanges == sizeof(kRanges) / sizeof(kRanges[0]),
              "档位数与档位表不匹配");

}  // namespace

Ads1220Adc::Ads1220Adc(int csPin, int drdyPin, uint32_t spiClk)
    : cs_(csPin), drdy_(drdyPin), spiSettings_(spiClk, MSBFIRST, SPI_MODE1) {}

bool Ads1220Adc::init() {
    SPI.begin();  // 默认 VSPI：SCK=18, MISO=19, MOSI=23
    pinMode(cs_, OUTPUT);
    digitalWrite(cs_, HIGH);
    pinMode(drdy_, INPUT_PULLUP);  // DOUT/DRDY 开漏，需上拉

    sendCmd(kCmdReset);
    delay(10);  // 复位后等待内部稳定

    writeReg(kRegConfig1, kCfg1Cont2000);   // 2000SPS 连续、内部基准
    writeReg(kRegConfig3, kCfg3Idac1Ain0);  // IDAC1 → AIN0
    setRange(0);                            // 默认最低阻值档（最大电流）

    return true;
}

bool Ads1220Adc::selftest() {
    // SPI 回读自检：写测试值到 CONFIG2，读回比对，再恢复原值
    uint8_t saved = readReg(kRegConfig2);
    writeReg(kRegConfig2, 0x07);  // IDAC=1500µA 测试值
    uint8_t readback = readReg(kRegConfig2);
    writeReg(kRegConfig2, saved);
    return readback == 0x07;
}

bool Ads1220Adc::setRange(uint8_t rangeIdx) {
    if (rangeIdx >= kNumRanges) return false;
    const RangeCfg& r = kRanges[rangeIdx];
    writeReg(kRegConfig0, r.gain << 1);  // MUX=AIN0/AIN1(0000)、PGA 使能
    writeReg(kRegConfig2, r.idac);       // IDAC 电流
    return true;
}

bool Ads1220Adc::startContinuous() {
    sendCmd(kCmdStart);
    return true;
}

bool Ads1220Adc::dataReady() {
    return digitalRead(drdy_) == LOW;  // CS 为高时 DOUT/DRDY 作 DRDY 用
}

int32_t Ads1220Adc::readRaw() {
    SPI.beginTransaction(spiSettings_);
    digitalWrite(cs_, LOW);
    SPI.transfer(kCmdRdata);
    uint8_t b0 = SPI.transfer(0x00);
    uint8_t b1 = SPI.transfer(0x00);
    uint8_t b2 = SPI.transfer(0x00);
    digitalWrite(cs_, HIGH);
    SPI.endTransaction();

    uint32_t u = (uint32_t(b0) << 16) | (uint32_t(b1) << 8) | uint32_t(b2);
    if (u & 0x00800000) u |= 0xFF000000;  // 24 位有符号扩展
    return int32_t(u);
}

void Ads1220Adc::setDrdyCallback(void (*cb)()) {
    (void)cb;
    // MVP 用 dataReady() 轮询。ADS1220 的 DOUT/DRDY 是共享引脚，SPI 读时
    // 会抖动产生假下降沿；后续若换独立 DRDY 引脚的板子，再在此 attachInterrupt。
}

void Ads1220Adc::sendCmd(uint8_t cmd) {
    SPI.beginTransaction(spiSettings_);
    digitalWrite(cs_, LOW);
    SPI.transfer(cmd);
    digitalWrite(cs_, HIGH);
    SPI.endTransaction();
}

void Ads1220Adc::writeReg(uint8_t addr, uint8_t val) {
    SPI.beginTransaction(spiSettings_);
    digitalWrite(cs_, LOW);
    SPI.transfer(kCmdWreg | (addr << 2));
    SPI.transfer(val);
    digitalWrite(cs_, HIGH);
    SPI.endTransaction();
}

uint8_t Ads1220Adc::readReg(uint8_t addr) {
    SPI.beginTransaction(spiSettings_);
    digitalWrite(cs_, LOW);
    SPI.transfer(kCmdRreg | (addr << 2));
    uint8_t val = SPI.transfer(0x00);
    digitalWrite(cs_, HIGH);
    SPI.endTransaction();
    return val;
}
