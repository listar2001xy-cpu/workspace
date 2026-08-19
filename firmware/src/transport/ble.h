#pragma once
#include <stddef.h>
#include <stdint.h>

// transport 层：BLE 传输（Nordic UART Service，FR-3 数据通路）。
// 硬件相关，仅 esp32dev 编译（native 的 build_src_filter 已排除 src/）。
namespace transport {

// 初始化 BLE：创建 NUS 服务（RX 写 / TX notify）并开始广播。
// 返回 true = 广播已启动。
bool bleInit();

// 通过 TX 特性 notify 一帧数据（完整帧，含 seq+CRC8，见 frame.h）。
// 无客户端连接/未订阅 notify 时返回 false，数据由上层决定是否缓冲。
bool bleNotify(const uint8_t* data, size_t len);

// 是否有客户端已连接。
bool bleConnected();

// 注册手机写入（RX 特性）处理回调；MVP 用于切换采样率命令（FR-2 软件可切换）。
void bleSetRxHandler(void (*handler)(const uint8_t* data, size_t len));

}  // namespace transport
