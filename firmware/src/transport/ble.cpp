#include "transport/ble.h"

#include <Arduino.h>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

namespace transport {
namespace {

// Nordic UART Service 标准 UUID（微信小程序据此发现服务）
constexpr const char* kServiceUuid = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
constexpr const char* kRxUuid = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";  // 手机 → 设备（写）
constexpr const char* kTxUuid = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";  // 设备 → 手机（notify）
constexpr const char* kDeviceName = "FlexSensor";

bool g_connected = false;
BLECharacteristic* g_tx = nullptr;
void (*g_rxHandler)(const uint8_t* data, size_t len) = nullptr;

// 连接/断线回调。core 3.x 中 onConnect/onDisconnect 各有 1-arg 与 2-arg 重载，
// 栈会依次调两个；这里只覆写 2-arg 版，保证每事件只触发一次。
class ServerCallbacks : public BLEServerCallbacks {
public:
    void onConnect(BLEServer* server, esp_ble_gatts_cb_param_t* param) override {
        (void)server;
        (void)param;
        g_connected = true;
        Serial.println("[BLE] connected");
    }
    void onDisconnect(BLEServer* server, esp_ble_gatts_cb_param_t* param) override {
        (void)server;
        (void)param;
        g_connected = false;
        Serial.println("[BLE] disconnected, re-advertising");
        server->startAdvertising();  // NFR-1 断线自动重连
    }
};

// 手机写入回调：转交上层处理（切采样率等命令）
class RxCallbacks : public BLECharacteristicCallbacks {
public:
    void onWrite(BLECharacteristic* ch, esp_ble_gatts_cb_param_t* param) override {
        (void)ch;
        if (g_rxHandler && param && param->write.len > 0) {
            g_rxHandler(param->write.value, param->write.len);
        }
    }
};

}  // namespace

bool bleInit() {
    BLEDevice::init(kDeviceName);

    BLEServer* server = BLEDevice::createServer();
    server->setCallbacks(new ServerCallbacks());

    BLEService* svc = server->createService(kServiceUuid);
    BLECharacteristic* rx = svc->createCharacteristic(kRxUuid, BLECharacteristic::PROPERTY_WRITE);
    rx->setCallbacks(new RxCallbacks());
    g_tx = svc->createCharacteristic(kTxUuid, BLECharacteristic::PROPERTY_NOTIFY);
    g_tx->addDescriptor(new BLE2902());  // CCCD：客户端订阅后才能 notify
    svc->start();

    BLEAdvertising* adv = BLEDevice::getAdvertising();
    adv->addServiceUUID(kServiceUuid);
    adv->setScanResponse(true);
    BLEDevice::startAdvertising();

    Serial.println("[BLE] advertising as FlexSensor");
    return true;
}

bool bleNotify(const uint8_t* data, size_t len) {
    if (!g_connected || !g_tx || !data || len == 0) return false;
    g_tx->setValue(const_cast<uint8_t*>(data), len);  // API 签名非 const，数据不会被改写
    g_tx->notify();
    return true;
}

bool bleConnected() {
    return g_connected;
}

void bleSetRxHandler(void (*handler)(const uint8_t* data, size_t len)) {
    g_rxHandler = handler;
}

}  // namespace transport
