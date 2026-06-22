#include "WifiPortal.h"

#include <WiFi.h>
#include <WiFiManager.h>

#include "AppLog.h"

namespace {
constexpr uint32_t kReconnectIntervalMs = 10000;
constexpr uint16_t kConfigPortalTimeoutSec = 180;
}

void WifiPortal::begin(const String &deviceName) {
  WiFi.mode(WIFI_STA);
  WiFi.persistent(true);
  WiFi.setAutoReconnect(true);

  WiFiManager wm;
  wm.setConfigPortalTimeout(kConfigPortalTimeoutSec);
  wm.setConnectTimeout(15);

  const String apName = deviceName + "-Setup";
  LOGI("wifi", "connecting, portal ap=%s", apName.c_str());

  if (wm.autoConnect(apName.c_str())) {
    LOGI("wifi", "connected ssid=%s ip=%s", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
    return;
  }

  LOGW("wifi", "not connected, continue offline");
}

void WifiPortal::loop() {
  if (WiFi.status() == WL_CONNECTED) return;

  const uint32_t now = millis();
  if (now - lastReconnectMs_ < kReconnectIntervalMs) return;
  lastReconnectMs_ = now;

  LOGW("wifi", "reconnect");
  WiFi.reconnect();
}

bool WifiPortal::connected() const {
  return WiFi.status() == WL_CONNECTED;
}

String WifiPortal::ip() const {
  return connected() ? WiFi.localIP().toString() : String("-");
}

String WifiPortal::ssid() const {
  return connected() ? WiFi.SSID() : String("-");
}

int8_t WifiPortal::rssi() const {
  return connected() ? WiFi.RSSI() : 0;
}

void WifiPortal::resetWifiSettings() {
  WiFiManager wm;
  wm.resetSettings();
  WiFi.disconnect(true, true);
  LOGW("wifi", "saved wifi settings cleared");
}
