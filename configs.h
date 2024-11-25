char ssid[] = "";
char username[] = "";
char identity[] = "";
char password[] = "";

uint8_t target_esp_mac[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

const char* botToken = "";
const char* channelChatID = "";
unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 1000;
int lastUpdateId = 0;