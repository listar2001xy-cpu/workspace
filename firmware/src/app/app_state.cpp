#include "app/app_state.h"

#include <Arduino.h>

// 状态机主循环。当前为占位骨架，业务逻辑随里程碑落地：
//   M1: selftest() 通过后 IDLE → ACQUIRING
//   M2: DRDY 中断 + RingBuffer + 抽取 + 换挡
//   M3: frame 编码 + BLE 发送
void AppState::setup() {
    Serial.begin(115200);
    Serial.println("flex-sensor firmware boot");
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
    Serial.printf("heartbeat %u\n", ++tick_);  // M0 验证用，后续删
    delay(1000);
}
