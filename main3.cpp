#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

// Налаштування мережі
const char *ssid = "miron_shtyrm";
const char *password = "miron2019";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws"); // WebSocket ендпоінт [cite: 164]

// Функція зчитування датчика та розсилки даних [cite: 177, 183]
void broadcastSensorData()
{
  int potValue = analogRead(34); // Пін ADC для потенціометра

  StaticJsonDocument<128> doc;
  doc["value"] = potValue;

  String jsonString;
  serializeJson(doc, jsonString);
  ws.textAll(jsonString); // Відправка всім підключеним клієнтам [cite: 183]
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len)
{
  if (type == WS_EVT_CONNECT)
  {
    Serial.printf("WebSocket client #%u connected\n", client->id());
  }
  else if (type == WS_EVT_DISCONNECT)
  {
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
  }
}

void setup()
{
  Serial.begin(115200);

  // Ініціалізація LittleFS [cite: 46]
  if (!LittleFS.begin())
  {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  // Підключення до Wi-Fi [cite: 45]
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println(WiFi.localIP()); // Вивід IP для браузера [cite: 50]

  // Налаштування WebSocket [cite: 164]
  ws.onEvent(onEvent);
  server.addHandler(&ws);

  // Маршрут для головної сторінки [cite: 49]
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.html", "text/html"); });

  server.begin();
}

void loop()
{
  static unsigned long lastTime = 0;
  // Частота оновлення (10 Гц)
  if (millis() - lastTime > 100)
  {
    broadcastSensorData();
    lastTime = millis();
  }
  ws.cleanupClients();
}
