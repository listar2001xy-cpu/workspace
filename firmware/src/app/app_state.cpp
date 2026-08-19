#include "app/app_state.h"

#include <Arduino.h>

#include "transport/ble.h"
#include "transport/frame.h"
#include "transport/wifi_ap.h"

namespace {

// 帧 payload 消息类型（接收端据此区分）
constexpr uint8_t kMsgHeartbeat = 0x01;
constexpr uint8_t kMsgSample = 0x02;

// BLE RX 命令：手机写 1 字节切换采样率（FR-2 软件可切换）
constexpr uint8_t kCmdRate100Hz = 0x01;   // 2000SPS ÷ 20
constexpr uint8_t kCmdRate500Hz = 0x02;   // ÷ 4
constexpr uint8_t kCmdRate1000Hz = 0x03;  // ÷ 2

}  // namespace

AppState* AppState::s_instance_ = nullptr;

void AppState::bleRxTrampoline(const uint8_t* data, size_t len) {
    if (s_instance_) s_instance_->onRxCommand(data, len);
}

void AppState::setup() {
    s_instance_ = this;

    Serial.begin(115200);
    Serial.println("flex-sensor firmware boot");

    // 无线：BLE（数据通路）+ WiFi AP（配置/调试）
    bool bleOk = transport::bleInit();
    bool wifiOk = transport::wifiApInit(nullptr, nullptr);
    Serial.printf("[BLE] init %s\n", bleOk ? "ok" : "FAIL");
    Serial.printf("[WiFi AP] init %s, ip=%s\n", wifiOk ? "ok" : "FAIL", transport::wifiApIp());
    transport::bleSetRxHandler(bleRxTrampoline);

    // 数据通路组件
    adc_.init();
    decimator_.setFactor(20);  // 默认 100Hz（2000SPS ÷ 20）
    autorange_.configure(Ads1220Adc::kFullScale, Ads1220Adc::kNumRanges);

    state_ = State::IDLE;
}

void AppState::loop() {
    switch (state_) {
        case State::IDLE:
            // M1 自检：SPI 回读验证，通过后进入采集
            state_ = adc_.selftest() ? State::ACQUIRING : State::ERROR;
            break;
        case State::ACQUIRING:
            adc_.startContinuous();
            state_ = State::STREAMING;
            break;
        case State::STREAMING:
            pumpData();
            break;
        case State::ERROR:
            // TODO: 看门狗 + 恢复（NFR-1）
            break;
    }

    // 心跳 1s（millis 计时，不阻塞数据通路）
    uint32_t now = millis();
    if (now - lastHeartbeat_ >= 1000) {
        lastHeartbeat_ = now;
        sendBleHeartbeat();
        Serial.printf("heartbeat %u ble=%d range=%u\n", ++tick_,
                      transport::bleConnected() ? 1 : 0, range_);
    }
}

void AppState::pumpData() {
    // DRDY 轮询（ADS1220 DOUT/DRDY 共享引脚，轮询比中断稳妥；2000SPS=每 500µs 一次）
    if (adc_.dataReady()) {
        rawBuf_.push(adc_.readRaw());  // 满则丢并计溢出（可查 overflowCount）
    }
    // 消费：抽取 → 换挡 → 帧编码 → BLE notify
    int32_t raw;
    while (rawBuf_.pop(raw)) {
        int32_t dec;
        if (decimator_.push(raw, dec)) {
            onDecimatedSample(dec);
        }
    }
}

void AppState::onDecimatedSample(int32_t v) {
    // 自动换挡：>80% 满量程升档（减小电流），<20% 降档（增大电流）
    uint8_t r = autorange_.decide(range_, v);
    if (r != range_) {
        range_ = r;
        adc_.setRange(range_);
    }

    // 帧 payload：[type=0x02][range][value 24bit 大端]
    uint8_t payload[5] = {
        kMsgSample,
        range_,
        uint8_t((v >> 16) & 0xFF),
        uint8_t((v >> 8) & 0xFF),
        uint8_t(v & 0xFF),
    };
    uint8_t frame[transport::kHeaderSize + sizeof(payload) + transport::kCrcSize];
    size_t n = transport::encode(frame, sizeof(frame), frameSeq_++, payload, sizeof(payload));
    if (n > 0) transport::bleNotify(frame, n);
}

void AppState::sendBleHeartbeat() {
    // 心跳帧 payload：[type=0x01][state][tick 2B 大端]
    uint8_t payload[4] = {
        kMsgHeartbeat,
        uint8_t(state_),
        uint8_t((tick_ >> 8) & 0xFF),
        uint8_t(tick_ & 0xFF),
    };
    uint8_t frame[transport::kHeaderSize + sizeof(payload) + transport::kCrcSize];
    size_t n = transport::encode(frame, sizeof(frame), frameSeq_++, payload, sizeof(payload));
    if (n > 0) transport::bleNotify(frame, n);
}

void AppState::onRxCommand(const uint8_t* data, size_t len) {
    if (len < 1) return;
    switch (data[0]) {
        case kCmdRate100Hz: decimator_.setFactor(20); break;
        case kCmdRate500Hz: decimator_.setFactor(4); break;
        case kCmdRate1000Hz: decimator_.setFactor(2); break;
        default: break;
    }
}
