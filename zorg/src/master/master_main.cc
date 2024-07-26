// Master controller.
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

#include <array>
#include <optional>
#include <vector>

#include "common/common.h"
#include "common/messages.h"
#include "master/cube.h"
#include "master/wall.h"

void OnDataSent(const uint8_t* mac_addr, esp_now_send_status_t status) {
  // Debug outgoing data.
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success"
                                                : "Delivery Fail");
}

Cube cube;

void OnDataReceived(const uint8_t* raw_addr, const uint8_t* data,
                    int data_len) {
  // Debug info.
  Serial.print("\r\nLast Packet Received:\t");
  for (int i = 0; i < data_len; i++) {
    Serial.printf("%#x", data[i]);
  }
  Serial.println();

  // Parse the address and event.
  MacAddress address = MacAddressFromArray(raw_addr);
  HandEvent event;
  memcpy(&event, data, sizeof(event));

  // Let the cube handle the event.
  cube.OnHandEvent(address, event);
}

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  // Master always has the LED turned on.
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.begin(115200);
  InitEspNow();
  Serial.println("Master MAC address: " + WiFi.macAddress());

  esp_now_register_send_cb(&OnDataSent);
  esp_now_register_recv_cb(&OnDataReceived);

  cube.AddWall(Wall({0xE8, 0x9F, 0x6D, 0x1F, 0x84, 0x44}));
  cube.AddWall(Wall({0x0C, 0x8B, 0x95, 0x93, 0x60, 0xF8}));

  cube.Connect();
}

constexpr uint64_t kGlitchTimeoutMillis = 5000;

void loop() { cube.Update(); }
