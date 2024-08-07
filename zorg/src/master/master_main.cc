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
#include "master/serial.h"
#include "master/wall.h"

void OnDataSent(const uint8_t* mac_addr, esp_now_send_status_t status) {
  if (status != ESP_NOW_SEND_SUCCESS) {
    serial::Debug("Failed to send data.");
  }
}

Cube cube;

void OnDataReceived(const uint8_t* raw_addr, const uint8_t* data,
                    int data_len) {
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
  serial::Debug("Master MAC address: %s", WiFi.macAddress());

  esp_now_register_send_cb(&OnDataSent);
  esp_now_register_recv_cb(&OnDataReceived);

// Actual wall.
#ifdef ACTUAL_WALL
  cube.AddWall(Wall({0x0C, 0x8B, 0x95, 0x96, 0xC6, 0x70}));
  cube.AddWall(Wall({0x0C, 0x8B, 0x95, 0x96, 0xB2, 0xF4}));
#else
  // Test walls.
  cube.AddWall(Wall({0x0C, 0x8B, 0x95, 0x94, 0xB4, 0xDC}));
  cube.AddWall(Wall({0x0C, 0x8B, 0x95, 0x93, 0x60, 0xF8}));
  cube.AddWall(Wall({0x0C, 0x8B, 0x95, 0x96, 0x41, 0xF4}));
  cube.AddWall(Wall({0x0C, 0x8B, 0x95, 0x93, 0x5A, 0xB4}));
#endif

  cube.Connect();
}

void loop() { cube.Update(); }
