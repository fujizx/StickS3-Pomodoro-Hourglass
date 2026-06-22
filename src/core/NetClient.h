#pragma once

#include <Arduino.h>
#include <functional>

#include <HTTPClient.h>
#include <WebSocketsClient.h>

using WebSocketTextHandler = std::function<void(const String &)>;

class HttpClient {
 public:
  explicit HttpClient(String baseUrl = "");
  void setBaseUrl(const String &baseUrl);
  bool get(const String &path, String &response);
  bool postJson(const String &path, const String &json, String &response);

 private:
  String baseUrl_;
  String urlFor(const String &path) const;
};

class WsClient {
 public:
  void begin(const String &host, uint16_t port, const String &path);
  void loop();
  void sendText(const String &text);
  void onText(WebSocketTextHandler handler);
  bool configured() const;

 private:
  WebSocketsClient client_;
  WebSocketTextHandler textHandler_;
  bool configured_ = false;
};
