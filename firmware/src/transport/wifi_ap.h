#pragma once

// transport 层：WiFi AP（设备自建热点，用于配置/调试，非数据通路）。
// 数据通路是 BLE（FR-3）；WiFi 是为后续配置/OTA 预留的第二连接通道。
namespace transport {

// 启动 AP。ssid/pass 传 nullptr 用默认值；返回 true = AP 已启动。
bool wifiApInit(const char* ssid, const char* pass);

// AP 模式下设备本机 IP（如 192.168.4.1），未启动返回 "0.0.0.0"。
const char* wifiApIp();

}  // namespace transport
