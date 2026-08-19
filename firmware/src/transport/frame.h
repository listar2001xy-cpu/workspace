#pragma once
#include <stddef.h>
#include <stdint.h>

// transport 层：帧编解码（纯逻辑，无硬件依赖，native 可单测）。
//
// 帧格式：
//   [SEQ(2B, 大端) | LEN(1B) | PAYLOAD(N) | CRC8(1B)]
//   SEQ  —— 自增序号，接收端据此检测丢包（FR-3：丢包率 <0.1%）
//   LEN  —— 负载长度（0..kMaxPayload）
//   CRC8 —— CRC-8/MAXIM（LSB-first，poly 0x8C = 0x31 反射），覆盖 SEQ+LEN+PAYLOAD
// 接收端 CRC 校验失败或长度不符 = 该帧作废（相当于丢包）。
namespace transport {

constexpr size_t kSeqSize = 2;
constexpr size_t kLenSize = 1;
constexpr size_t kCrcSize = 1;
constexpr size_t kHeaderSize = kSeqSize + kLenSize;  // 3
constexpr size_t kMaxPayload = 240;                   // BLE MTU 约束下的安全上限
constexpr uint8_t kCrc8Poly = 0x8C;  // CRC-8/MAXIM 反射多项式（0x31 位反转，DS18B20/1-Wire 同款）

// CRC-8/MAXIM（LSB-first，poly 0x8C，init 0x00）：独立校验点 crc8("123456789") == 0xA1。
inline uint8_t crc8(const uint8_t* data, size_t len) {
    uint8_t crc = 0x00;
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; ++bit) {
            crc = (crc & 0x01) ? uint8_t((crc >> 1) ^ kCrc8Poly) : uint8_t(crc >> 1);
        }
    }
    return crc;
}

// 编码：写入完整帧，返回帧长；out 空间不足或参数非法返回 0。
inline size_t encode(uint8_t* out, size_t outCap, uint16_t seq, const uint8_t* payload,
                     uint8_t len) {
    if (!out || (len > 0 && !payload) || len > kMaxPayload) return 0;
    const size_t total = kHeaderSize + len + kCrcSize;
    if (outCap < total) return 0;

    out[0] = uint8_t((seq >> 8) & 0xFF);  // SEQ 大端
    out[1] = uint8_t(seq & 0xFF);
    out[2] = len;
    for (uint8_t i = 0; i < len; ++i) out[kHeaderSize + i] = payload[i];
    out[kHeaderSize + len] = crc8(out, kHeaderSize + len);  // 覆盖 SEQ+LEN+PAYLOAD
    return total;
}

// 解码：校验长度 + CRC，提取 seq/payload。失败返回 false（视为丢包）。
inline bool decode(const uint8_t* frame, size_t frameLen, uint16_t& seq,
                   const uint8_t*& payload, uint8_t& payloadLen) {
    if (!frame || frameLen < kHeaderSize + kCrcSize) return false;
    const uint8_t len = frame[2];
    if (frameLen != kHeaderSize + len + kCrcSize) return false;
    if (crc8(frame, kHeaderSize + len) != frame[kHeaderSize + len]) return false;
    seq = uint16_t((frame[0] << 8) | frame[1]);
    payload = &frame[kHeaderSize];
    payloadLen = len;
    return true;
}

}  // namespace transport
