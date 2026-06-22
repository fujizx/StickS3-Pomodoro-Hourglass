#pragma once

#include <Arduino.h>

enum class LogLevel {
  Debug,
  Info,
  Warn,
  Error,
};

class AppLog {
 public:
  static void begin(uint32_t baud = 115200);
  static void setLevel(LogLevel level);
  static void write(LogLevel level, const char *tag, const char *message);
  static void printf(LogLevel level, const char *tag, const char *fmt, ...);

 private:
  static LogLevel minLevel_;
  static const char *levelName(LogLevel level);
};

#define LOGD(tag, fmt, ...) AppLog::printf(LogLevel::Debug, tag, fmt, ##__VA_ARGS__)
#define LOGI(tag, fmt, ...) AppLog::printf(LogLevel::Info, tag, fmt, ##__VA_ARGS__)
#define LOGW(tag, fmt, ...) AppLog::printf(LogLevel::Warn, tag, fmt, ##__VA_ARGS__)
#define LOGE(tag, fmt, ...) AppLog::printf(LogLevel::Error, tag, fmt, ##__VA_ARGS__)
