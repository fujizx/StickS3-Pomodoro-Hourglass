#pragma once

#include <Arduino.h>
#include <M5Unified.h>

class BatteryMeter {
 public:
  void begin();
  void loop();
  int level() const;
  String text() const;

 private:
  int level_ = -1;
  uint32_t lastReadMs_ = 0;
};
