#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#include "display.h"
#include "http.h"
#include "connection.h"

String buildTelegramApiUrl() {
  return String("https://api.telegram.org/bot") + botToken 
         + "/getUpdates?offset=" + (lastUpdateId + 1) 
         + "&allowed_updates=[\"message\",\"edited_message\", \"message_reaction\"]";
}

// Получение имени отправителя
String getSenderUsername(JsonObject message) {
  return message.containsKey("from") && message["from"].containsKey("username")
           ? message["from"]["username"].as<String>() :
           message.containsKey("user") && message["user"].containsKey("username")
           ? message["user"]["username"].as<String>()
           : "unknown";
}

// Определение типа вложения
String getAttachmentType(JsonObject message) {
  if (message.containsKey("photo")) return "photo";
  if (message.containsKey("video")) return "video";
  if (message.containsKey("audio")) return "audio";
  if (message.containsKey("document")) return "document";
  return "unknown attachment";
}

// Обработка реакции на сообщение
void handleReaction(JsonObject message) {
  String emoji = message["new_reaction"][0]["emoji"].as<String>();
  int messageId = message["new_reaction"][0]["message_id"];
  String senderUsername = getSenderUsername(message);

  Serial.printf("Reaction '%s' added by @%s to message ID %d\n", emoji.c_str(), senderUsername.c_str(), messageId);

  // Логика обработки реакций
  displayMessage("Reaction: " + emoji + " from @" + senderUsername);
}

// Проверка наличия вложений
bool containsAttachment(JsonObject message) {
  return message.containsKey("photo") || message.containsKey("video") || 
         message.containsKey("audio") || message.containsKey("document");
}

// Обработка текстовых сообщений
void handleTextMessage(JsonObject message, bool isEdited) {
  String text = message["text"].as<String>();
  String senderChatUsername = getSenderUsername(message);

  if (isEdited) {
    Serial.printf("Edited text message from @%s: %s\n", senderChatUsername.c_str(), text.c_str());
  } else {
    Serial.printf("New text message from @%s: %s\n", senderChatUsername.c_str(), text.c_str());
  }

  displayMessage(text);
}

// Обработка сообщений с подписью
void handleCaptionMessage(JsonObject message, bool isEdited) {
  String caption = message["caption"].as<String>();
  String senderChatUsername = getSenderUsername(message);

  if (isEdited) {
    Serial.printf("Edited caption message from @%s: %s\n", senderChatUsername.c_str(), caption.c_str());
  } else {
    Serial.printf("New caption message from @%s: %s\n", senderChatUsername.c_str(), caption.c_str());
  }

  displayMessage(caption);
}

// Обработка сообщений с вложениями
void handleAttachmentMessage(JsonObject message, bool isEdited) {
  String attachmentType = getAttachmentType(message);
  String senderChatUsername = getSenderUsername(message);

  if (isEdited) {
    Serial.printf("Edited message with %s from @%s\n", attachmentType.c_str(), senderChatUsername.c_str());
  } else {
    Serial.printf("New message with %s from @%s\n", attachmentType.c_str(), senderChatUsername.c_str());
  }

  displayMessage("Received attachment: " + attachmentType);
}

// Обработка нового сообщения
void handleNewMessage(JsonObject message) {
  if (message.containsKey("text")) {
    handleTextMessage(message, false);
  } else if (message.containsKey("caption")) {
    handleCaptionMessage(message, false);
  } else if (containsAttachment(message)) {
    handleAttachmentMessage(message, false);
  }
}

// Обработка редактированного сообщения
void handleEditedMessage(JsonObject editedMessage) {
  if (editedMessage.containsKey("text")) {
    handleTextMessage(editedMessage, true);
  } else if (editedMessage.containsKey("caption")) {
    handleCaptionMessage(editedMessage, true);
  } else if (containsAttachment(editedMessage)) {
    handleAttachmentMessage(editedMessage, true);
  }
}

void handleTelegramUpdate(JsonObject update) {
  // Сохраняем последний обработанный update_id
  int update_id = update["update_id"];
  lastUpdateId = update_id;

  // Обработка нового сообщения
  if (update.containsKey("message")) {
    JsonObject message = update["message"];
    handleNewMessage(message);
  }

  // Обработка редактированного сообщения
  if (update.containsKey("edited_message")) {
    JsonObject editedMessage = update["edited_message"];
    handleEditedMessage(editedMessage);
  }

  // Обработка реакции на сообщение
  if (update.containsKey("message_reaction")) {
    JsonObject message = update["message_reaction"];
    handleReaction(message);
  }
}

void processTelegramUpdates(const String& payload) {
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  
  const size_t capacity = JSON_ARRAY_SIZE(10) + JSON_OBJECT_SIZE(10) + 1000;
  
  #pragma GCC diagnostic pop

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
  Serial.println(payload);
  processTelegramUpdates(payload);
}