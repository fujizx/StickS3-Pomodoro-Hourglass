#include "AppConfig.h"

namespace {
constexpr const char *kNamespace = "stick-app";
}

bool AppConfig::begin() {
  if (!prefs_.begin(kNamespace, false)) return false;

  settings_.deviceName = prefs_.getString("name", settings_.deviceName);
  settings_.timezone = prefs_.getString("tz", settings_.timezone);
  settings_.ntpServer1 = prefs_.getString("ntp1", settings_.ntpServer1);
  settings_.ntpServer2 = prefs_.getString("ntp2", settings_.ntpServer2);
  settings_.httpBaseUrl = prefs_.getString("http", settings_.httpBaseUrl);
  settings_.wsHost = prefs_.getString("wshost", settings_.wsHost);
  settings_.wsPort = prefs_.getUShort("wsport", settings_.wsPort);
  settings_.wsPath = prefs_.getString("wspath", settings_.wsPath);
  return true;
}

void AppConfig::end() {
  prefs_.end();
}

const AppSettings &AppConfig::settings() const {
  return settings_;
}

void AppConfig::save(const AppSettings &settings) {
  settings_ = settings;
  prefs_.putString("name", settings_.deviceName);
  prefs_.putString("tz", settings_.timezone);
  prefs_.putString("ntp1", settings_.ntpServer1);
  prefs_.putString("ntp2", settings_.ntpServer2);
  prefs_.putString("http", settings_.httpBaseUrl);
  prefs_.putString("wshost", settings_.wsHost);
  prefs_.putUShort("wsport", settings_.wsPort);
  prefs_.putString("wspath", settings_.wsPath);
}

void AppConfig::clear() {
  prefs_.clear();
  settings_ = AppSettings{};
}
