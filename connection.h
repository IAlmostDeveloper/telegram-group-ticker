#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

extern "C" {
#include "user_interface.h"
#include "wpa2_enterprise.h"
#include "c_types.h"
}

bool isWiFiConnected() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    return false;
  }
  return true;
}

void connect_to_wifi() {
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);
  delay(1000);
  Serial.setDebugOutput(true);
  Serial.printf("SDK version: %s\n", system_get_sdk_version());
  Serial.printf("Free Heap: %4d\n", ESP.getFreeHeap());
  wifi_set_opmode(STATION_MODE);
  struct station_config wifi_config;
  memset(&wifi_config, 0, sizeof(wifi_config));
  strcpy((char*)wifi_config.ssid, ssid);
  strcpy((char*)wifi_config.password, password);
  wifi_station_set_config(&wifi_config);
  wifi_set_macaddr(STATION_IF, target_esp_mac);
  wifi_station_set_wpa2_enterprise_auth(1);
  wifi_station_clear_cert_key();
  wifi_station_clear_enterprise_ca_cert();
  wifi_station_clear_enterprise_identity();
  wifi_station_clear_enterprise_username();
  wifi_station_clear_enterprise_password();
  wifi_station_clear_enterprise_new_password();

  wifi_station_set_enterprise_identity((uint8*)identity, strlen(identity));
  wifi_station_set_enterprise_username((uint8*)username, strlen(username));
  wifi_station_set_enterprise_password((uint8*)password, strlen((char*)password));

  wifi_station_connect();
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void testInternetConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient https;

  Serial.println("Testing internet connection...");

  if (https.begin(client, "https://www.google.com")) {
    int httpCode = https.GET();
    if (httpCode > 0) {
      Serial.printf("[HTTP] GET... to Google, code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK) {
        Serial.println("Internet connection is working.");
      }
    } else {
      Serial.printf("[HTTP] GET... to Google failed, error: %s\n", https.errorToString(httpCode).c_str());
    }
    https.end();
  } else {
    Serial.println("[HTTPS] Unable to connect to Google");
  }
}