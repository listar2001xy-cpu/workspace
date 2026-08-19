#pragma once

// 硬件引脚定义（ESP32 DevKit + ADS1220 默认接线）。
// [待确认] 若你的实际接线不同，只改这里，其余代码不感知具体引脚。
namespace board {

// SPI 走默认 VSPI：SCK=GPIO18, MISO=GPIO19, MOSI=GPIO23（SPI.begin() 内部固定，无需配置）
constexpr int kAds1220Cs = 5;    // ADS1220 CS（片选）
constexpr int kAds1220Drdy = 4;  // ADS1220 DOUT/DRDY（开漏输出，内部上拉）

}  // namespace board
