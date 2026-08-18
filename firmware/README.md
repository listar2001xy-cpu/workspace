# firmware — 柔性传感器数据采集固件（单通道 MVP）

ESP32 + ADS1220（24-bit ΣΔ，恒流法测电阻）固件。分层架构详见根目录 [docs/architecture.md](../docs/architecture.md)。

## 目录结构

```
src/
  app/        状态机主循环（编排，无业务算法）
  core/       采样引擎：环形缓冲 / 抽取 / 换挡（纯逻辑，native 可单测）
  transport/  帧编解码 + BLE 发送
  hal/        ADC 抽象接口 i_adc.h + 具体芯片实现（唯一碰寄存器的层）
test/
  test_core/      纯逻辑单测（Unity，native）
  test_embedded/  硬件在环（真机，HIL）
```

依赖方向：上层依赖下层，下层绝不依赖上层。换 ADC 芯片只改 `hal/`。

## 构建 / 烧录

```bash
pio run -d firmware              # 编译 esp32dev
pio run -d firmware -t upload    # 烧录（先装 USB 驱动 CP210x/CH340）
pio run -d firmware -t monitor   # 串口监视（115200）
```

## 测试

```bash
pio test -d firmware -e native   # PC 上跑纯逻辑单测
pio test -d firmware -e esp32dev # 真机 HIL（需接线）
```

详见 [test/README.md](test/README.md)。

## 代码规范

- 4 空格缩进，列宽 100，UTF-8 / LF（见 `.clang-format` / `.editorconfig`）
- 提交信息：`<type>(<scope>): <描述>`，type ∈ feat/fix/refactor/test/docs/chore
- 纯逻辑模块保持头文件式，便于 native 单测

## 里程碑（当前）

- [x] M0 骨架：分层目录 + 构建 + 测试闭环
- [ ] M1 自检：selftest() 通过后 IDLE → ACQUIRING
- [ ] M2 采样：DRDY 中断 + 环形缓冲 + 抽取 ÷2/÷4/÷20 + 换挡
- [ ] M3 传输：帧编码（SEQ + CRC8）+ BLE 发送
- [ ] M4/M5 标定：标准电阻验收
