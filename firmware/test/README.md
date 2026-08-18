# 测试

纯逻辑模块（`src/core/`、`src/transport/` 的纯函数部分）在 PC 上跑 native 单测；硬件相关在真机跑 HIL。

## 运行

```bash
# PC 上跑纯逻辑单测（test/test_core）
pio test -d firmware -e native

# 真机 HIL（test/test_embedded，需接线 + 烧录）
pio test -d firmware -e esp32dev
```

## 目录

- `test_core/`     —— 纯逻辑单测（Unity，native 环境，无硬件依赖）
- `test_embedded/` —— 硬件在环测试（真机，串口记录标定数据）

## 约定

- 纯逻辑模块保持头文件式（如 `core/ring_buffer.h`），native 测试直接 `#include`。
- 新增纯逻辑模块时，同步在 `test_core/` 加一个 `test_<module>.cpp`。
- 测试文件名以 `test_` 开头，PlatformIO 自动识别。
