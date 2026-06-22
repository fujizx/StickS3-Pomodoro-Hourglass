#include "AppLog.h"

#include <stdarg.h>

LogLevel AppLog::minLevel_ = LogLevel::Debug;

void AppLog::begin(uint32_t baud) {
  Serial.begin(baud);
  delay(80);
}

void AppLog::setLevel(LogLevel level) {
  minLevel_ = level;
}

void AppLog::write(LogLevel level, const char *tag, const char *message) {
  if (static_cast<int>(level) < static_cast<int>(minLevel_)) return;
  Serial.printf("[%10lu] %-5s %-10s %s\n", millis(), levelName(level), tag, message);
}

void AppLog::printf(LogLevel level, const char *tag, const char *fmt, ...) {
  if (static_cast<int>(level) < static_cast<int>(minLevel_)) return;

  char buffer[192];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  write(level, tag, buffer);
}

const char *AppLog::levelName(LogLevel level) {
  switch (level) {
    case LogLevel::Debug:
      return "DEBUG";
    case LogLevel::Info:
      return "INFO";
    case LogLevel::Warn:
      return "WARN";
    case LogLevel::Error:
      return "ERROR";
  }
  return "LOG";
}
