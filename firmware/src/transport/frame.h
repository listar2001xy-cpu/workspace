#pragma once
#include <stdint.h>

// transport 层：帧编解码 + BLE 发送（M3 落地）。
//
// 帧格式草案（待 M3 定稿）：
//   [SEQ(2B) | LEN(1B) | PAYLOAD(N) | CRC8(1B)]
//   SEQ  —— 自增序号，用于丢包检测（FR-3 要求 <0.1%）
//   CRC8 —— 多项式 0x31，校验头 + 负载
//
// 纯逻辑部分（SEQ/CRC8 编解码）将像 core/ring_buffer.h 一样在 native 下单测。
// 此处仅定义常量，不声明未实现的函数（避免悬空声明）。
namespace transport {

constexpr uint8_t kSeqSize = 2;      // 序号字节数
constexpr uint8_t kLenSize = 1;      // 长度字节数
constexpr uint8_t kCrcSize = 1;      // 校验字节数
constexpr uint8_t kCrc8Poly = 0x31;  // CRC-8 多项式（MAXIM）

}  // namespace transport
