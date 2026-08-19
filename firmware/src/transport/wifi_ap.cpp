#include "transport/wifi_ap.h"

#include <Arduino.h>
#include <WiFi.h>

namespace transport {
namespace {

constexpr const char* kDefaultSsid = "FlexSensor";
constexpr const char* kDefaultPass = "12345678";  // [待确认] 生产应从 NVS 读取，不硬编码

bool g_started = false;

}  // namespace

bool wifiApInit(const char* ssid, const char* pass) {
    const char* s = ssid ? ssid : kDefaultSsid;
    const char* p = pass ? pass : kDefaultPass;
    g_started = WiFi.softAP(s, p);
    if (g_started) {
        Serial.printf("[WiFi AP] ssid=%s ip=%s\n", s, WiFi.softAPIP().toString().c_str());
    }
    return g_started;
}

const char* wifiApIp() {
    static String ip;
    ip = g_started ? WiFi.softAPIP().toString() : String("0.0.0.0");
    return ip.c_str();
}

}  // namespace transport
