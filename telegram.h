#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#include "display.h"
#include "connection.h"

String buildTelegramApiUrl() {
  return String("https://api.telegram.org/bot") + botToken + "/getUpdates?offset=" + (lastUpdateId + 1);
}

bool sendHttpRequest(const String& url, String& payload) {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient https;
  if (!https.begin(client, url)) {
    Serial.println("[HTTPS] Unable to connect");
    return false;
  }

  int httpCode = https.GET();
  if (httpCode > 0) {
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    if (httpCode == HTTP_CODE_OK) {
      payload = https.getString();
      https.end();
      return true;
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
  }

  https.end();
  return false;
}

void handleTelegramUpdate(JsonObject update) {
  int update_id = update["update_id"];
  if (!update.containsKey("message")) return;

  JsonObject message = update["message"];
  String text = message.containsKey("text") ? message["text"].as<String>() : "";
  String senderChatUsername = message.containsKey("sender_chat") && message["sender_chat"].containsKey("username")
                                ? message["sender_chat"]["username"].as<String>()
                                : "";

  Serial.printf("New channel post from @%s: %s\n", senderChatUsername.c_str(), text.c_str());
  displayMessage(text);
  lastUpdateId = update_id;
}

void processTelegramUpdates(const String& payload) {
  const size_t capacity = JSON_ARRAY_SIZE(10) + JSON_OBJECT_SIZE(10) + 1000;
  DynamicJsonDocument doc(capacity);
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
    return;
  }

  JsonArray updates = doc["result"].as<JsonArray>();
  for (JsonObject update : updates) {
    handleTelegramUpdate(update);
  }
}

void getTelegramUpdates() {
  if (!isWiFiConnected()) return;

  String url = buildTelegramApiUrl();
  Serial.println("Requesting: " + url);

  String payload;
  if (!sendHttpRequest(url, payload)) return;

  processTelegramUpdates(payload);
}