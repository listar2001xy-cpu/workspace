#pragma once
#include <stddef.h>
#include <stdint.h>

#include "core/autorange.h"
#include "core/decimator.h"
#include "core/ring_buffer.h"
#include "hal/ads1220.h"
#include "hal/board_pins.h"

// app 层：状态机主循环。编排 hal → core → transport，自身不含业务算法。
// 数据通路：ADS1220 连续转换 → DRDY 轮询 → 环形缓冲 → 抽取 → 自动换挡 → 帧编码 → BLE notify。
class AppState {
public:
    enum class State { IDLE, ACQUIRING, STREAMING, ERROR };

    void setup();
    void loop();
    State state() const { return state_; }

private:
    void pumpData();
    void onDecimatedSample(int32_t v);
    void sendBleHeartbeat();
    void onRxCommand(const uint8_t* data, size_t len);

    static void bleRxTrampoline(const uint8_t* data, size_t len);  // transport C 回调 → 实例
    static AppState* s_instance_;

    static constexpr size_t kRawBufLog2 = 6;  // 64 槽原始样本缓冲

    Ads1220Adc adc_{board::kAds1220Cs, board::kAds1220Drdy};
    RingBuffer<int32_t, kRawBufLog2> rawBuf_;
    Decimator decimator_;
    Autorange autorange_;

    State state_ = State::IDLE;
    uint32_t tick_ = 0;
    uint32_t lastHeartbeat_ = 0;
    uint16_t frameSeq_ = 0;
    uint8_t range_ = 0;
};
