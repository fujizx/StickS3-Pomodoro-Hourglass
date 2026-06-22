#include <Arduino.h>
#include <M5Unified.h>

#include "core/AppConfig.h"
#include "core/AppLog.h"
#include "core/BatteryMeter.h"
#include "core/NetClient.h"
#include "core/TimeSync.h"
#include "core/WifiPortal.h"

namespace {
constexpr uint32_t kRollCooldownMs = 650;
constexpr uint32_t kClockRefreshMs = 1000;
constexpr uint32_t kUiFrameDelayMs = 20;
constexpr uint8_t kDisplayBrightness = 160;
constexpr float kShakeThreshold = 1.6f;

enum class Screen {
  Menu,
  Dice,
  Clock,
};

constexpr const char *kMenuItems[] = {
    "Clock",
    "Dice",
};
constexpr int kMenuCount = sizeof(kMenuItems) / sizeof(kMenuItems[0]);

Screen screen = Screen::Menu;
int menuIndex = 0;
uint32_t lastRollMs = 0;
uint32_t lastClockDrawMs = 0;
uint32_t bootMs = 0;
int dieValue = 1;
bool imuReady = false;
String lastClockText;
AppConfig appConfig;
WifiPortal wifiPortal;
TimeSync timeSync;
BatteryMeter battery;
HttpClient httpClient;
WsClient wsClient;

void drawFooter(const char *left, const char *right) {
  auto &display = M5.Display;
  const int w = display.width();
  const int h = display.height();

  display.setTextDatum(bottom_left);
  display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  display.setTextSize(1);
  display.drawString(left, 6, h - 4);

  display.setTextDatum(bottom_right);
  display.drawString(right, w - 6, h - 4);
}

void drawStatusBar() {
  auto &display = M5.Display;
  const int w = display.width();

  battery.loop();
  display.fillRect(0, 0, w, 13, TFT_BLACK);
  display.setTextSize(1);
  display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  display.setTextDatum(top_left);
  display.drawString(wifiPortal.connected() ? "WiFi" : "Offline", 4, 2);
  display.setTextDatum(top_right);
  display.drawString(battery.text(), w - 4, 2);
}

void drawMenu() {
  auto &display = M5.Display;
  const int w = display.width();

  display.fillScreen(TFT_BLACK);
  drawStatusBar();
  display.setTextDatum(top_center);
  display.setTextColor(TFT_WHITE, TFT_BLACK);
  display.setTextSize(2);
  display.drawString("STICK S3", w / 2, 16);

  display.setTextSize(1);
  display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  display.drawString("Menu", w / 2, 42);

  for (int i = 0; i < kMenuCount; ++i) {
    const int y = 68 + i * 38;
    const bool selected = i == menuIndex;
    const uint16_t bg = selected ? TFT_WHITE : TFT_BLACK;
    const uint16_t fg = selected ? TFT_BLACK : TFT_WHITE;

    display.fillRoundRect(18, y, w - 36, 28, 6, bg);
    display.setTextDatum(middle_center);
    display.setTextColor(fg, bg);
    display.setTextSize(2);
    display.drawString(kMenuItems[i], w / 2, y + 14);
  }

  drawFooter("B: Next", "A: OK");
}

void drawDieFace(int n) {
  auto &display = M5.Display;
  const int w = display.width();
  const int h = display.height();
  const int size = min(w, h) - 34;
  const int x = (w - size) / 2;
  const int y = (h - size) / 2 + 6;
  const int r = max(5, size / 12);
  const int inset = size / 4;
  const int mid = size / 2;

  display.fillScreen(TFT_BLACK);
  drawStatusBar();
  display.setTextDatum(top_center);
  display.setTextColor(TFT_WHITE, TFT_BLACK);
  display.setTextSize(1);
  display.drawString("Shake or press A", w / 2, 16);

  display.fillRoundRect(x, y, size, size, 10, TFT_WHITE);

  auto pip = [&](int px, int py) {
    display.fillCircle(x + px, y + py, r, TFT_BLACK);
  };

  if (n == 1 || n == 3 || n == 5) pip(mid, mid);
  if (n >= 2) {
    pip(inset, inset);
    pip(size - inset, size - inset);
  }
  if (n >= 4) {
    pip(size - inset, inset);
    pip(inset, size - inset);
  }
  if (n == 6) {
    pip(inset, mid);
    pip(size - inset, mid);
  }

  drawFooter("B: Menu", "");
}

void rollDie() {
  const uint32_t now = millis();
  if (now - lastRollMs < kRollCooldownMs) return;

  lastRollMs = now;
  dieValue = random(1, 7);
  LOGI("dice", "roll=%d", dieValue);
  drawDieFace(dieValue);
}

bool didShake() {
  if (!imuReady) return false;
  if (!M5.Imu.update()) return false;

  const auto data = M5.Imu.getImuData();
  const float magnitude = sqrtf(data.accel.x * data.accel.x +
                                data.accel.y * data.accel.y +
                                data.accel.z * data.accel.z);
  return fabsf(magnitude - 1.0f) > kShakeThreshold;
}

void drawFlipCard(int x, int y, int width, int height, const char *label, const char *value) {
  auto &display = M5.Display;
  constexpr uint16_t kCardTop = 0x3186;
  constexpr uint16_t kCardBottom = 0x18C3;
  constexpr uint16_t kCardBorder = 0x5AEB;

  display.fillRoundRect(x, y, width, height, 6, kCardBottom);
  display.fillRoundRect(x, y, width, height / 2, 6, kCardTop);
  display.drawRoundRect(x, y, width, height, 6, kCardBorder);
  display.drawFastHLine(x + 4, y + height / 2, width - 8, TFT_BLACK);
  display.drawFastHLine(x + 4, y + height / 2 + 1, width - 8, kCardBorder);

  display.setTextDatum(top_center);
  display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  display.setTextSize(1);
  display.drawString(label, x + width / 2, y - 13);

  display.setTextDatum(middle_center);
  display.setTextColor(TFT_WHITE, kCardTop);
  display.setTextSize(3);
  display.drawString(value, x + width / 2, y + height / 2 + 1);
}

void animateFlipCard(int x, int y, int width, int height, const char *label,
                     const String &fromValue, const char *toValue) {
  auto &display = M5.Display;

  drawFlipCard(x, y, width, height, label, fromValue.c_str());
  for (int i = 0; i < 4; ++i) {
    const int fold = (height / 2) - i * 6;
    display.fillRect(x + 4, y + 4, width - 8, height / 2 - fold, TFT_BLACK);
    display.drawFastHLine(x + 4, y + height / 2 - i * 2, width - 8, TFT_DARKGREY);
    delay(18);
  }

  drawFlipCard(x, y, width, height, label, toValue);
  for (int i = 0; i < 3; ++i) {
    display.drawFastHLine(x + 4, y + height / 2 + i * 2, width - 8, TFT_DARKGREY);
    delay(16);
  }
}

void drawClockChrome(const String &subtitle) {
  auto &display = M5.Display;
  const int w = display.width();

  display.fillScreen(TFT_BLACK);
  drawStatusBar();
  display.setTextDatum(top_center);
  display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  display.setTextSize(1);
  display.drawString(subtitle, w / 2, 18);
  drawFooter("B: Menu", "");
}

void drawClock(bool force = false) {
  const uint32_t now = millis();
  if (!force && now - lastClockDrawMs < kClockRefreshMs) return;
  lastClockDrawMs = now;

  char hhText[3];
  char mmText[3];
  char ssText[3];
  String subtitle;
  if (timeSync.ready()) {
    const String ntpTime = timeSync.timeText();
    strlcpy(hhText, ntpTime.substring(0, 2).c_str(), sizeof(hhText));
    strlcpy(mmText, ntpTime.substring(3, 5).c_str(), sizeof(mmText));
    strlcpy(ssText, ntpTime.substring(6, 8).c_str(), sizeof(ssText));
    subtitle = timeSync.dateText();
  } else {
    const uint32_t seconds = (now - bootMs) / 1000;
    const uint32_t hh = seconds / 3600;
    const uint32_t mm = (seconds / 60) % 60;
    const uint32_t ss = seconds % 60;
    snprintf(hhText, sizeof(hhText), "%02lu", static_cast<unsigned long>(hh));
    snprintf(mmText, sizeof(mmText), "%02lu", static_cast<unsigned long>(mm));
    snprintf(ssText, sizeof(ssText), "%02lu", static_cast<unsigned long>(ss));
    subtitle = wifiPortal.connected() ? "NTP waiting" : "Offline uptime";
  }

  const String clockText = String(hhText) + ":" + mmText + ":" + ssText;
  if (!force && clockText == lastClockText) return;

  const String oldClockText = lastClockText;
  const bool canAnimate = !force && oldClockText.length() == 8;
  lastClockText = clockText;

  drawClockChrome(subtitle);

  if (!canAnimate) {
    drawFlipCard(12, 52, 64, 56, "HOUR", hhText);
    drawFlipCard(88, 52, 64, 56, "MIN", mmText);
    drawFlipCard(164, 52, 64, 56, "SEC", ssText);
    return;
  }

  const String oldHh = oldClockText.substring(0, 2);
  const String oldMm = oldClockText.substring(3, 5);
  const String oldSs = oldClockText.substring(6, 8);

  if (oldHh != hhText) {
    animateFlipCard(12, 52, 64, 56, "HOUR", oldHh, hhText);
  } else {
    drawFlipCard(12, 52, 64, 56, "HOUR", hhText);
  }

  if (oldMm != mmText) {
    animateFlipCard(88, 52, 64, 56, "MIN", oldMm, mmText);
  } else {
    drawFlipCard(88, 52, 64, 56, "MIN", mmText);
  }

  if (oldSs != ssText) {
    animateFlipCard(164, 52, 64, 56, "SEC", oldSs, ssText);
  } else {
    drawFlipCard(164, 52, 64, 56, "SEC", ssText);
  }
}

void enterSelectedApp() {
  if (menuIndex == 0) {
    screen = Screen::Clock;
    lastClockText = "";
    drawClock(true);
    return;
  }

  screen = Screen::Dice;
  drawDieFace(dieValue);
}

void returnToMenu() {
  screen = Screen::Menu;
  drawMenu();
}
}  // namespace

void setup() {
  AppLog::begin(115200);
  LOGI("boot", "starting");

  auto cfg = M5.config();
  M5.begin(cfg);

  randomSeed(esp_random());
  bootMs = millis();
  M5.Display.setRotation(1);
  M5.Display.setBrightness(kDisplayBrightness);
  LOGI("display", "brightness=%u ui_loop=%luHz clock_redraw=1Hz",
       M5.Display.getBrightness(),
       static_cast<unsigned long>(1000 / kUiFrameDelayMs));
  imuReady = M5.Imu.getType() != m5::imu_none;
  battery.begin();

  if (!appConfig.begin()) {
    LOGE("config", "preferences init failed");
  }
  const AppSettings &settings = appConfig.settings();
  httpClient.setBaseUrl(settings.httpBaseUrl);
  wsClient.onText([](const String &text) {
    LOGI("ws", "text=%s", text.c_str());
  });

  drawMenu();
  wifiPortal.begin(settings.deviceName);
  timeSync.begin(settings);
  wsClient.begin(settings.wsHost, settings.wsPort, settings.wsPath);

  drawMenu();
}

void loop() {
  M5.update();
  wifiPortal.loop();
  timeSync.loop(wifiPortal.connected());
  battery.loop();
  wsClient.loop();

  if (screen == Screen::Menu) {
    if (didShake()) {
      menuIndex = 0;
      drawMenu();
    }
    if (M5.BtnB.wasPressed()) {
      menuIndex = (menuIndex + 1) % kMenuCount;
      drawMenu();
    }
    if (M5.BtnA.wasPressed()) {
      enterSelectedApp();
    }
  } else {
    if (M5.BtnB.wasPressed()) {
      returnToMenu();
    } else if (screen == Screen::Dice) {
      if (M5.BtnA.wasPressed() || didShake()) {
        rollDie();
      }
    } else if (screen == Screen::Clock) {
      drawClock();
    }
  }

  delay(kUiFrameDelayMs);
}
