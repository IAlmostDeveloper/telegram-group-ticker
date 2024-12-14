#include <ESP8266HTTPClient.h>

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