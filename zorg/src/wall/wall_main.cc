// Wall controller.
//
// The wall controller starts off unpaired from the master controller. Once the
// master sends it a message, it will connect to it.
//
// The wall controller forwards events about the state of the hand.
#include <Arduino.h>
#include <FastLED.h>
#include <WiFi.h>
#include <esp_now.h>

#include <vector>

#include "common/common.h"
#include "common/messages.h"
#include "wall/animation.h"
#include "wall/led_mapper_data.h"

// The MAC address of the master controller. Set once the master sends a
// message.
MacAddress master_address;

// Whether the hand is currently pressed or not.
constexpr uint8_t kHandPin = BUTTON;
bool hand_pressed = false;

LEDController controller;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Debug outgoing data.
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success"
                                                : "Delivery Fail");
}

void OnDataReceived(const uint8_t *mac_addr, const uint8_t *data,
                    int data_len) {
  // Debug incoming data.
  Serial.println("Received packet:");
  for (int i = 0; i < data_len; i++) {
    Serial.printf("%#x ", data[i]);
  }
  Serial.println();

  // We assume any message we get is from the master.
  master_address = MacAddressFromArray(mac_addr);

  // Parse message.
  SetPatternCommand command;
  memcpy(&command, data, sizeof(command));
  Serial.printf(
      "Received command, switching to id=%d, speed=%d, transition=%d\n",
      command.pattern_id, command.pattern_speed,
      command.transition_duration_millis);
  controller.SetCurrentPattern(command.pattern_id, command.pattern_speed,
                               command.transition_duration_millis);
}

void SendHandEvent(const HandEvent &event) {
  // Skip sending if we are not paired with the master yet.
  if (master_address == EmptyMacAddress()) {
    Serial.println("Can't send event, no master paired.");
    return;
  }

  // If we aren't connected to the master yet, do that now.
  if (!esp_now_is_peer_exist(master_address.data())) {
    Serial.println("Connecting to master.");
    AddPeer(master_address);
  }

  // Send the event.
  esp_err_t result =
      esp_now_send(master_address.data(),
                   reinterpret_cast<const uint8_t *>(&event), sizeof(event));

  if (result == ESP_OK) {
    Serial.println("Message sent successfully");
  } else {
    Serial.println("Error sending the message");
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(kHandPin, INPUT);
  // Wall always has the LED turned off.
  digitalWrite(LED_BUILTIN, LOW);

  Serial.begin(115200);
  InitEspNow();
  Serial.println("Wall MAC address: " + WiFi.macAddress());

  esp_now_register_send_cb(&OnDataSent);
  esp_now_register_recv_cb(&OnDataReceived);

  // Initialize FastLED.
  controller.InitLEDs(kNumLeds, coordsX, coordsY, angles, radii);
  FastLED
      .addLeds<NEOPIXEL, 5>(controller.led_data().data(),
                            controller.led_data().size())
      .setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(10);
  FastLED.setMaxRefreshRate(60, true);

  for (LED &led : controller.leds()) {
    Serial.printf("{x: %d, y: %d, angle: %d, radius: %d}\n", led.x(), led.y(),
                  led.angle(), led.radius());
  }
}

void animate() {
  controller.Update();
  FastLED.show();
}

void loop() {
  EVERY_N_SECONDS(1) {
    Serial.printf("Current pattern: %d\n", controller.current_pattern_id());
  }
  animate();
  if (digitalRead(kHandPin) == LOW) {
    if (!hand_pressed) {
      Serial.println("Button pressed!");
      SendHandEvent(HandEvent{.type = HandEventType::kPressed});
      hand_pressed = true;
    }
  } else {
    if (hand_pressed) {
      Serial.println("Button released!");
      SendHandEvent(HandEvent{.type = HandEventType::kReleased});
      hand_pressed = false;
    }
  }
}
