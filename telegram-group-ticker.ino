#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <MD_Parola.h>
#include <MD_MAX72XX.h>
#include <SPI.h>

#include "cyrillic_font.h"

// Определяем пины подключения
#define DIN_PIN 13 // GPIO13 (D7)
#define CLK_PIN 14 // GPIO14 (D5)
#define CS_PIN  12 // GPIO12 (D6)

// Указываем тип матрицы. Используем FC-16.
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define NUM_MATRICES 4 // Количество матриц

// Создаем объект Parola
MD_Parola display = MD_Parola(HARDWARE_TYPE, DIN_PIN, CLK_PIN, CS_PIN, NUM_MATRICES);

extern "C" {
  #include "user_interface.h"
  #include "wpa2_enterprise.h"
  #include "c_types.h"
}

char ssid[] = "xxxxx";
char username[] = "xxxxx";
char identity[] = "xxxxx";
char password[] = "xxxxx";

uint8_t target_esp_mac[6] = {0x24, 0x0a, 0xc4, 0x9a, 0x58, 0x28};

const char* botToken = "xxxxx";
const char* channelChatID = "xxxxx";
unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 1000; 
int lastUpdateId = 0;

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

void getTelegramUpdates() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient https;

  String url = String("https://api.telegram.org/bot") + botToken + "/getUpdates?offset=" + (lastUpdateId + 1);
  
  Serial.println("Requesting: " + url);

  if (https.begin(client, url)) {  // HTTPS
    int httpCode = https.GET();

    if (httpCode > 0) {
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK) {
        String payload = https.getString();
        Serial.println(payload);
        const size_t capacity = JSON_ARRAY_SIZE(10) + JSON_OBJECT_SIZE(10) + 1000;
        DynamicJsonDocument doc(capacity);
        DeserializationError error = deserializeJson(doc, payload);

        if (error) {
          Serial.print("deserializeJson() failed: ");
          Serial.println(error.f_str());
          https.end();
          return;
        }

        JsonArray updates = doc["result"].as<JsonArray>();

        for (JsonObject update : updates) {
          int update_id = update["update_id"];

          if (update.containsKey("message")) {
            JsonObject channel_post = update["message"];

            String text = "";
            if (channel_post.containsKey("text")) {
              text = channel_post["text"].as<String>();
            }

            String sender_chat_username = "";
            if (channel_post.containsKey("sender_chat") && channel_post["sender_chat"].containsKey("username")) {
              sender_chat_username = channel_post["sender_chat"]["username"].as<String>();
            }

            Serial.printf("New channel post from @%s: %s\n", sender_chat_username.c_str(), text.c_str());
            display.displayClear();   // Очищаем матрицу
            display.displayText(text.c_str(), PA_CENTER, 40, 500, PA_SCROLL_LEFT, PA_SCROLL_LEFT);

            while (!display.displayAnimate()) {
              // Ждем завершения анимации
              delay(40);
            }
          }
          lastUpdateId = update_id;
        }
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
    }

    https.end();
  } else {
    Serial.println("[HTTPS] Unable to connect");
  }
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

void setup() {

  // Инициализация дисплея
  display.begin();
  display.setIntensity(1);  // Устанавливаем яркость (0-15)
  display.setSpeed(100);
  display.setFont(CyrillicLatinFont);
  display.displayClear();

  connect_to_wifi();
  testInternetConnection();
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastUpdateTime >= updateInterval) {
    lastUpdateTime = currentMillis;
    getTelegramUpdates();
  }
}
