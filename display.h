#include <MD_Parola.h>
#include <MD_MAX72XX.h>
#include <SPI.h>

#define DIN_PIN 13  // GPIO13 (D7)
#define CLK_PIN 14  // GPIO14 (D5)
#define CS_PIN 12   // GPIO12 (D6)

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define NUM_MATRICES 16

MD_Parola display = MD_Parola(HARDWARE_TYPE, DIN_PIN, CLK_PIN, CS_PIN, NUM_MATRICES);

bool isWiFiConnected() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    return false;
  }
  return true;
}

void initDisplay() {
  display.begin();
  display.setIntensity(8);  // Устанавливаем яркость (0-15)
  display.setSpeed(200);
  display.setFont(CyrillicLatinFont);
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