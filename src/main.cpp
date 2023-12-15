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

static int flag = 0;
const char *SSID = "SentinelAG";
const char *PASS = "";
WebSocketsServer webSocket = WebSocketsServer(81);

int16_t buffer[100][2];

void handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_BIN:
    if (length == sizeof(buffer))
    {
      memcpy(buffer, payload, length);
    }
    break;
  case WStype_CONNECTED:
    // New WebSocket connection
    Serial.printf("[%u] Connected\n", num);
    break;
  case WStype_DISCONNECTED:
    Serial.printf("[%u] Disconnected\n", num);
    break;
  case WStype_ERROR:
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

  if (flag == 0){
    Serial.println(WiFi.localIP());
    flag = 1;
  }

  webSocket.loop();

  for (int i = 0; i < 100; i++)
  {
    i2s_read_sample(&buffer[i][0], &buffer[i][1], true);
  }

  sendBinaryToAllClients((uint8_t *)buffer, sizeof(buffer));

  delay(10);
}
