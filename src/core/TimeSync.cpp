#include "TimeSync.h"

#include "AppLog.h"

namespace {
constexpr uint32_t kCheckIntervalMs = 5000;
}

void TimeSync::begin(const AppSettings &settings) {
  configTzTime(settings.timezone.c_str(),
               settings.ntpServer1.c_str(),
               settings.ntpServer2.c_str());
  configured_ = true;
  ready_ = false;
  LOGI("time", "ntp configured tz=%s", settings.timezone.c_str());
}

void TimeSync::loop(bool wifiConnected) {
  if (!configured_ || !wifiConnected) return;

  const uint32_t now = millis();
  if (now - lastCheckMs_ < kCheckIntervalMs) return;
  lastCheckMs_ = now;

  struct tm info;
  if (getLocalTime(&info, 50)) {
    if (!ready_) LOGI("time", "ntp synced");
    ready_ = true;
  }
}

bool TimeSync::ready() const {
  return ready_;
}

String TimeSync::timeText() const {
  struct tm info;
  if (!getLocalTime(&info, 10)) return "--:--:--";

  char buffer[16];
  strftime(buffer, sizeof(buffer), "%H:%M:%S", &info);
  return String(buffer);
}

String TimeSync::dateText() const {
  struct tm info;
  if (!getLocalTime(&info, 10)) return "NTP waiting";

  char buffer[24];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d", &info);
  return String(buffer);
}
