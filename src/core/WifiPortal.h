#pragma once

#include <Arduino.h>

class WifiPortal {
 public:
  void begin(const String &deviceName);
  void loop();
  bool connected() const;
  String ip() const;
  String ssid() const;
  int8_t rssi() const;
  void resetWifiSettings();

 private:
  uint32_t lastReconnectMs_ = 0;
};
