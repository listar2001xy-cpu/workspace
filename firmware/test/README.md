# 测试

纯逻辑模块（`src/core/`、`src/transport/` 的纯函数部分）在 PC 上跑 native 单测；硬件相关在真机跑 HIL。

## 运行

```bash
# PC 上跑纯逻辑单测（test/test_* 各模块目录）
pio test -d firmware -e native

# 真机 HIL（test/test_embedded，需接线 + 烧录）
pio test -d firmware -e esp32dev
```

## 目录

- `test_core/`      —— 环形缓冲（core/ring_buffer.h）
- `test_decimator/` —— 抽取器（core/decimator.h）
- `test_autorange/` —— 自动换挡（core/autorange.h）
- `test_frame/`     —— 帧编解码（transport/frame.h）
- `test_embedded/`  —— 硬件在环测试（真机，串口记录标定数据）

## 约定

- 纯逻辑模块保持头文件式（如 `core/decimator.h`），native 测试直接 `#include`。
- **每个纯逻辑模块一个 `test_<module>/` 目录**：PlatformIO 会把同一目录下所有 .cpp 链接成单个二进制，多个 `main()` 会冲突，故一个模块一个目录、目录内一个测试文件。
- 测试文件名以 `test_` 开头，PlatformIO 自动识别。
