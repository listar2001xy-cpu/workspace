# 柔性传感器数据采集系统（单通道 MVP）

8 通道柔性传感器（0.001Ω–1MΩ）数据采集系统的**单通道 MVP**。

- **硬件**：ESP32 单芯片 + ADS1220（24-bit ΣΔ ADC，恒流法测电阻）
- **链路**：`传感器 → ADS1220 → SPI → ESP32 → BLE → 微信小程序 → 微信云开发`
- **范围**：首期单通道电阻 1Ω–100kΩ，采样率 100/500/1000Hz 三档

## 目录结构

```
firmware/          ESP32 固件（PlatformIO）
  src/app/         状态机主循环（编排）
  src/core/        采样引擎（缓冲/抽取/换挡，纯逻辑）
  src/transport/   帧编解码 + BLE 发送
  src/hal/         ADC 抽象接口 + ADS1220 实现
miniprogram/       微信小程序
docs/              需求 / 架构 / 协议 / 标定
.github/           CI
```

## 快速开始

```bash
# 固件构建
pio run -d firmware

# 固件烧录（先装 USB 驱动：CP210x 或 CH340）
pio run -d firmware -t upload
```

## 文档

- [docs/requirements.md](docs/requirements.md) — 需求基线（可验收规格）
- [docs/architecture.md](docs/architecture.md) — 分层架构与接口
- docs/protocol.md — 帧格式（M3 落地）
- docs/calibration.md — 标定记录（M5 落地）
- docs/traceability.md — 需求-测试追溯
