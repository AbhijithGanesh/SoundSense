#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <i2s.h>
#include <WebSocketsServer.h>

#ifndef STASSID
#define STASSID "SentinelAG"
#define STAPSK  "SentinelChaz@14"
#endif

const char *SSID = STASSID;
const char *PASS = STAPSK;

WiFiUDP udp;
WebSocketsServer webSocket = WebSocketsServer(81);  // Create a WebSocket server on port 81
const IPAddress listener = { 192, 168, 1, 40 };

const int port = 16500;

int16_t buffer[100][2]; // Determine the Sampling time

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch(type) {
    case WSop_close:
      Serial.printf("[%u] WebSocket Closed\n", num);
    case WStype_CONNECTED:
      Serial.printf("[%u] WebSocket Connected\n", num);
      break;
    case WStype_DISCONNECTED:
      Serial.printf("[%u] WebSocket Disconnected\n", num);
      break;
    default:
      break;
  }
}

void setup() {
  
  Serial.begin(9600);

  // Connect to WiFi network
  WiFi.begin(SSID, PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("My IP: ");
  Serial.println(WiFi.localIP());
  
  i2s_rxtx_begin(true, false); // Enable I2S RX
  i2s_set_rate(16000);

  Serial.print("\nStart the listener on ");
  Serial.print(listener);
  Serial.print(":");
  Serial.println(port);
  Serial.println("Under Linux for listener: netcat -u -p 16500 -l | play -t raw -r 16000 -b 16 -c 2 -e signed-integer -");
  Serial.println("Under Linux for recorder: netcat -u -p 16500 -l | rec -t raw -r 16000 -b 16 -c 2 -e signed-integer - file.mp3");
  udp.beginPacket(listener, port);
  udp.write("I2S Receiver\r\n");
  udp.endPacket();

  webSocket.begin();
  Serial.println("WebSocketsServer started.");
  webSocket.onEvent(onWebSocketEvent);
}

void loop() {
  webSocket.loop();  // Handle WebSocket events

  static int cnt = 0;

  // Each loop will send 100 raw samples (400 bytes)
  // UDP needs to be < TCP_MSS which can be 500 bytes in LWIP V2
  for (int i = 0; i < 100; i++) {
    i2s_read_sample(&buffer[i][0], &buffer[i][1], true);
    Serial.println(i2s_read_sample(&buffer[i][0], &buffer[i][1], true));
  }

  udp.beginPacket(listener, port);
  udp.write((uint8_t*)buffer, sizeof(buffer));
  udp.endPacket();

  webSocket.broadcastTXT((char*)buffer, sizeof(buffer));  // Broadcast the data to all connected clients

  cnt++;
  if ((cnt % 100) == 0) {
    Serial.printf("%d\n", cnt);
  }
}
