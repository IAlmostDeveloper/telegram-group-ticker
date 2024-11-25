#include "configs.h"
#include "cyrillic_font.h"
#include "telegram.h"

void setup() {
  initDisplay();
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
