#include <Arduino.h>

// 状态机占位（M2 落地完整实现）
enum class State { IDLE, ACQUIRING, STREAMING, ERROR };
static State g_state = State::IDLE;

void setup() {
    Serial.begin(115200);
    Serial.println("flex-sensor firmware boot");
    g_state = State::IDLE;
}

void loop() {
    switch (g_state) {
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
    delay(1000);
}
