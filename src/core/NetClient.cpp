#include "NetClient.h"

#include <utility>

#include "AppLog.h"

HttpClient::HttpClient(String baseUrl) : baseUrl_(std::move(baseUrl)) {}

void HttpClient::setBaseUrl(const String &baseUrl) {
  baseUrl_ = baseUrl;
}

bool HttpClient::get(const String &path, String &response) {
  HTTPClient http;
  const String url = urlFor(path);
  if (!http.begin(url)) return false;

  const int code = http.GET();
  response = http.getString();
  http.end();

  LOGI("http", "GET %s -> %d", url.c_str(), code);
  return code >= 200 && code < 300;
}

bool HttpClient::postJson(const String &path, const String &json, String &response) {
  HTTPClient http;
  const String url = urlFor(path);
  if (!http.begin(url)) return false;

  http.addHeader("Content-Type", "application/json");
  const int code = http.POST(json);
  response = http.getString();
  http.end();

  LOGI("http", "POST %s -> %d", url.c_str(), code);
  return code >= 200 && code < 300;
}

String HttpClient::urlFor(const String &path) const {
  if (path.startsWith("http://") || path.startsWith("https://")) return path;
  if (baseUrl_.endsWith("/") && path.startsWith("/")) return baseUrl_ + path.substring(1);
  if (!baseUrl_.endsWith("/") && !path.startsWith("/")) return baseUrl_ + "/" + path;
  return baseUrl_ + path;
}

void WsClient::begin(const String &host, uint16_t port, const String &path) {
  if (host.length() == 0) return;

  client_.begin(host, port, path);
  client_.setReconnectInterval(5000);
  client_.onEvent([this](WStype_t type, uint8_t *payload, size_t length) {
    switch (type) {
      case WStype_CONNECTED:
        LOGI("ws", "connected");
        break;
      case WStype_DISCONNECTED:
        LOGW("ws", "disconnected");
        break;
      case WStype_TEXT:
        if (textHandler_) {
          String text;
          text.reserve(length);
          for (size_t i = 0; i < length; ++i) {
            text += static_cast<char>(payload[i]);
          }
          textHandler_(text);
        }
        break;
      default:
        break;
    }
  });
  configured_ = true;
}

void WsClient::loop() {
  if (configured_) client_.loop();
}

void WsClient::sendText(const String &text) {
  if (!configured_) return;
  String payload = text;
  client_.sendTXT(payload);
}

void WsClient::onText(WebSocketTextHandler handler) {
  textHandler_ = std::move(handler);
}

bool WsClient::configured() const {
  return configured_;
}
