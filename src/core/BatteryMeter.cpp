#include "BatteryMeter.h"

namespace {
constexpr uint32_t kBatteryReadIntervalMs = 2000;
}

void BatteryMeter::begin() {
  loop();
}

void BatteryMeter::loop() {
  const uint32_t now = millis();
  if (now - lastReadMs_ < kBatteryReadIntervalMs && level_ >= 0) return;
  lastReadMs_ = now;
  level_ = M5.Power.getBatteryLevel();
}

int BatteryMeter::level() const {
  return level_;
}

String BatteryMeter::text() const {
  if (level_ < 0) return "BAT --";
  return "BAT " + String(level_) + "%";
}
