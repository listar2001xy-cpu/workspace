#include "app/app_state.h"

#include <Arduino.h>

#include "transport/ble.h"
#include "transport/frame.h"
#include "transport/wifi_ap.h"

// 状态机主循环。当前为占位骨架 + 无线初始化验证：
//   M1: selftest() 通过后 IDLE → ACQUIRING
//   M2: DRDY 中断 + RingBuffer + 抽取 + 换挡
//   M3: frame 编码 + BLE 发送（当前以心跳 notify 预演）
void AppState::setup() {
    Serial.begin(115200);
    Serial.println("flex-sensor firmware boot");

    // 无线：BLE（数据通路）+ WiFi AP（配置/调试）
    bool bleOk = transport::bleInit();
    bool wifiOk = transport::wifiApInit(nullptr, nullptr);
    Serial.printf("[BLE] init %s\n", bleOk ? "ok" : "FAIL");
    Serial.printf("[WiFi AP] init %s, ip=%s\n", wifiOk ? "ok" : "FAIL", transport::wifiApIp());

    state_ = State::IDLE;
}

void AppState::loop() {
    switch (state_) {
        case State::IDLE:
            // TODO(M1): selftest() → 通过后进 ACQUIRING
            break;
        case State::ACQUIRING:
            // TODO(M2): DRDY 中断 + 环形缓冲 + 抽取 + 换挡
            break;
        case State::STREAMING:
            // TODO(M3): 帧编码 + BLE 发送
            break;
        case State::ERROR:
            // TODO: 看门狗 + 恢复
            break;
    }

    sendBleHeartbeat();

    Serial.printf("heartbeat %u ble=%d\n", ++tick_, transport::bleConnected() ? 1 : 0);
    delay(1000);
}

void AppState::sendBleHeartbeat() {
    // 心跳帧 payload：[state(1B) | tick(2B, 大端)]
    uint8_t payload[3] = {
        (uint8_t)state_,
        (uint8_t)((tick_ >> 8) & 0xFF),
        (uint8_t)(tick_ & 0xFF),
    };
    uint8_t frame[transport::kHeaderSize + sizeof(payload) + transport::kCrcSize];
    size_t n = transport::encode(frame, sizeof(frame), frameSeq_++, payload, sizeof(payload));
    if (n > 0) transport::bleNotify(frame, n);
}
