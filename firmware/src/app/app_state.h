#pragma once
#include <stdint.h>

// app 层：状态机主循环。编排 hal → core → transport，自身不含业务算法。
// 具体逻辑随里程碑逐步落地（当前为占位骨架）。
class AppState {
public:
    enum class State { IDLE, ACQUIRING, STREAMING, ERROR };

    void setup();
    void loop();
    State state() const { return state_; }

private:
    State state_ = State::IDLE;
    uint32_t tick_ = 0;
};
