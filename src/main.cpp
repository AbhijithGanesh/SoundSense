/*
  SoundSense Project
  Author: Abhijith Ganesh
  Year: 2023
  License: AGPL v3.0
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <I2S.h>

const char *SSID = "SentinelAG";
const char *PASS = "SentinelChaz@14";
WebSocketsServer webSocket = WebSocketsServer(81);

int16_t buffer[100][2];

void handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_BIN:
    // Binary data received
    if (length == sizeof(buffer))
    {
      memcpy(buffer, payload, length);
      // Process audio data if needed
    }
    break;
  case WStype_CONNECTED:
    // New WebSocket connection
    Serial.printf("[%u] Connected\n", num);
    break;
  case WStype_DISCONNECTED:
    // WebSocket disconnected
    Serial.printf("[%u] Disconnected\n", num);
    break;
  case WStype_ERROR:
    // WebSocket error
    Serial.printf("[%u] Error\n", num);
    break;
  default:
    break;
  }
}

void sendBinaryToAllClients(const uint8_t *data, size_t length)
{
  for (uint8_t i = 0; i < webSocket.connectedClients(); i++)
  {
    webSocket.sendBIN(i, data, length);
  }
}

void setup()
{
  Serial.begin(115200);

  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  webSocket.begin();
  webSocket.onEvent(handleWebSocketEvent);

  i2s_rxtx_begin(true, false);
  i2s_set_rate(16000);

  Serial.println("WebSocket server started");
}

void loop()
{
  webSocket.loop();

  for (int i = 0; i < 100; i++)
  {
    i2s_read_sample(&buffer[i][0], &buffer[i][1], true);
  }

  // Send audio data over WebSocket to all connected clients
  sendBinaryToAllClients((uint8_t *)buffer, sizeof(buffer));

  delay(10); // Adjust the delay as needed
}
