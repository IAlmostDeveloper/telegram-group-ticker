#include <MD_Parola.h>
#include <MD_MAX72XX.h>
#include <SPI.h>
#include <Ticker.h>
#include "animations.h"

#define DIN_PIN 13  // GPIO13 (D7)
#define CLK_PIN 14  // GPIO14 (D5)
#define CS_PIN 12   // GPIO12 (D6)

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define NUM_MATRICES 16

MD_Parola display = MD_Parola(HARDWARE_TYPE, DIN_PIN, CLK_PIN, CS_PIN, NUM_MATRICES);

// === Ticker для смены статичных сообщений ===
Ticker idleTicker;
const char* staticMessages[] = {"HELLO", "WELCOME", "IDLE MODE", "ESP8266"};
const int numStaticMessages = sizeof(staticMessages) / sizeof(staticMessages[0]);
int staticMessageIndex = 0;  // Индекс текущего статичного сообщения
String newMessage = "";       // Сообщение из порта
volatile bool newMessageFlag = false;  // Флаг получения нового сообщения


// === Функция для смены статичных сообщений ===
void displayStaticMessage() {
  Serial.println("displayStaticMessage()");
  if (!newMessageFlag) {  // Меняем сообщение только если нет новых
    staticMessageIndex = (staticMessageIndex + 1) % numStaticMessages;
    Serial.println(staticMessages[staticMessageIndex]);
    display.displayClear();
    display.displayText(staticMessages[staticMessageIndex], PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
  }
}

// === Метод для немедленного отображения нового сообщения ===
void displayNewMessage(const String& message) {
  Serial.print("displayNewMessage(");
  Serial.print(message.c_str());
  Serial.println(")");

  display.displayClear();
  display.displayText(message.c_str(), PA_CENTER, 40, 500, PA_SPRITE, PA_SCROLL_LEFT);
  newMessageFlag = false;  // Сбрасываем флаг после начала отображения
  // Ждём, пока сообщение прокрутится
  while (!display.displayAnimate()) {
    yield();  // Для поддержания работы ESP8266
  }
}

void initDisplay() {
  display.begin();
  #if ENA_SPRITE
  display.setSpriteData(pacman1, W_PMAN1, F_PMAN1, pacman2, W_PMAN2, F_PMAN2);
#endif
  // display.setIntensity(8);  // Устанавливаем яркость (0-15)
  // display.setSpeed(200);
  display.setFont(CyrillicLatinFont);
  display.displayClear();
}

void enableIdleMessages() {
  Serial.println("enableIdleMessages()");
  // Запускаем Ticker для отображения статичных сообщений
  idleTicker.attach(3, displayStaticMessage);  // Смена статичных сообщений каждые 3 секунды
}

void disableIdleMessages() {
  Serial.println("disableIdleMessages()");
  idleTicker.detach();  // Останавливаем отображение статичных сообщений
  display.displayClear();
}

void displayMessage(const String& text) {
  for (int i = 0; i < 3; i++) {
    display.displayText(text.c_str(), PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
    display.displayAnimate();
    delay(300);
    display.displayClear();
    delay(300);
  }
  display.displayClear();
  display.displayText(text.c_str(), PA_CENTER, 40, 500, PA_SCROLL_LEFT, PA_SCROLL_LEFT);

  while (!display.displayAnimate()) {
    delay(1);
  }
}