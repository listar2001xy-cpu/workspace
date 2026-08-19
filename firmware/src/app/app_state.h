#pragma once
#include <stdint.h>

// app 层：状态机主循环。编排 hal → core → transport，自身不含业务算法。
// 具体逻辑随里程碑逐步落地（当前为占位骨架 + 无线初始化验证）。
class AppState {
public:
    enum class State { IDLE, ACQUIRING, STREAMING, ERROR };

    void setup();
    void loop();
    State state() const { return state_; }

private:
    void sendBleHeartbeat();

    State state_ = State::IDLE;
    uint32_t tick_ = 0;      // 主循环计数（串口心跳显示）
    uint16_t frameSeq_ = 0;  // 帧序号（BLE 心跳 + M3 数据帧共用）
};
