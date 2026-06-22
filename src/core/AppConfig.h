#pragma once

#include <Arduino.h>
#include <Preferences.h>

struct AppSettings {
  String deviceName = "StickS3";
  String timezone = "CST-8";
  String ntpServer1 = "pool.ntp.org";
  String ntpServer2 = "ntp.aliyun.com";
  String httpBaseUrl = "";
  String wsHost = "";
  uint16_t wsPort = 80;
  String wsPath = "/";
};

class AppConfig {
 public:
  bool begin();
  void end();
  const AppSettings &settings() const;
  void save(const AppSettings &settings);
  void clear();

 private:
  Preferences prefs_;
  AppSettings settings_;
};
