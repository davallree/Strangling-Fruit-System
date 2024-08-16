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

Cube cube;

void OnDataSent(const uint8_t* mac_addr, esp_now_send_status_t status) {
  // Parse the address and event.
  MacAddress address = MacAddressFromArray(mac_addr);
  Wall* wall = cube.GetWall(address);
  if (wall == nullptr) return;
  if (status == ESP_NOW_SEND_SUCCESS) {
    wall->set_last_delivery_status(DeliveryStatus::kSuccess);
  } else {
    wall->set_last_delivery_status(DeliveryStatus::kFailure);
    serial::Debug("Failed to send data.");
  }
}

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
  cube.AddWall(Wall({0x0C, 0x8B, 0x95, 0x94, 0xB4, 0xDC}));
  cube.AddWall(Wall({0x0C, 0x8B, 0x95, 0x96, 0xBA, 0xFC}));
#else
  // Test walls.
  cube.AddWall(Wall({0x0C, 0x8B, 0x95, 0x93, 0x60, 0xF8}));
  cube.AddWall(Wall({0x0C, 0x8B, 0x95, 0x96, 0x41, 0xF4}));
  cube.AddWall(Wall({0x0C, 0x8B, 0x95, 0x93, 0x5A, 0xB4}));
#endif

  cube.Connect();
}

uint64_t last_update_time_millis = 0;
int update_delay_millis = 1000;

void loop() {
  // Check if the Serial has any data to read.
  if (Serial.available() > 0) {
    // Parse the incoming JSON message.
    ArduinoJson::JsonDocument doc;
    ArduinoJson::deserializeJson(doc, Serial);
    const std::string& method = doc[kMethod];
    const ArduinoJson::JsonObject& params = doc[kParams];
    serial::Debug("Received message: %s", method.c_str());
    if (method == "restartMaster") {
      serial::Debug("Restarting...");
      ESP.restart();
    } else if (method == "restartWall") {
      // Forward the message to the wall.
      int wall_id = params["wallId"];
      serial::Debug("wall id: %d", wall_id);
      Wall* wall = cube.GetWall(wall_id);
      if (wall != nullptr) {
        wall->SendRestartCommand();
      }
    }
  }
  cube.Update();

  if ((millis() - last_update_time_millis) > update_delay_millis) {
    last_update_time_millis = millis();
    serial::UpdateStatus(cube);
  }
}
