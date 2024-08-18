// Wall controller.
//
// The wall controller starts off unpaired from the master controller. Once the
// master sends it a message, it will connect to it.
//
// The wall controller forwards events about the state of the hand.
#include <Arduino.h>
#include <FastLED.h>
#include <Preferences.h>
#include <WiFi.h>
#include <esp_now.h>

#include <ArduinoJson.hpp>
#include <vector>

#include "common/common.h"
#include "common/messages.h"
#include "wall/animation.h"
#include "wall/led_mapper_data.h"

// The MAC address of the master controller. Set once the master sends a
// message.
MacAddress master_address;

// Whether the hand is currently pressed or not.
constexpr uint8_t kHandPin = T0;
constexpr char kTouchThresholdKey[] = "touch_threshold";
constexpr uint16_t kDefaultTouchThreshold = 18;
uint16_t touch_threshold = kDefaultTouchThreshold;
bool hand_pressed = false;
bool calibrated = false;
uint16_t calibration_step = 0;
constexpr uint16_t kCalibrationLimit = 20;

LEDController controller;

Preferences prefs;

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
  Serial.printf("%s\n", data);

  // We assume any message we get is from the master.
  master_address = MacAddressFromArray(mac_addr);

  // Parse message.
  ArduinoJson::JsonDocument doc;
  ArduinoJson::deserializeJson(doc, data, data_len);
  if (doc[kMethod] == SetPatternCommand::kMethodName) {
    SetPatternCommand command = SetPatternCommand::FromJsonCommand(doc);
    Serial.printf(
        "Received command, switching to id=%d, speed=%d, transition=%d\n",
        command.pattern_id, command.pattern_speed,
        command.transition_duration_millis);
    controller.SetCurrentPattern(command.pattern_id, command.pattern_speed,
                                 command.transition_duration_millis);
  } else if (doc[kMethod] == kRestartMethod) {
    ESP.restart();
  } else if (doc[kMethod] == kSetTouchThresholdMethod) {
    uint16_t new_touch_threshold = doc[kParams][kTouchThresholdParam];
    Serial.printf("Setting new touch threshold: %d\n", new_touch_threshold);
    prefs.putUShort(kTouchThresholdKey, new_touch_threshold);
    touch_threshold = new_touch_threshold;
  }
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
  ArduinoJson::JsonDocument doc;
  doc[kMethod] = kSetHandStateMethod;
  if (event.type == HandEventType::kPressed) {
    doc[kParams][kHandStateParam] = kPressed;
  } else {
    doc[kParams][kHandStateParam] = kReleased;
  }
  std::string out;
  ArduinoJson::serializeJson(doc, out);
  esp_err_t result = esp_now_send(
      master_address.data(), reinterpret_cast<const uint8_t *>(out.c_str()),
      out.size() + 1);
  if (result != ESP_OK) {
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
#ifdef ACTUAL_WALL
  FastLED
      .addLeds<WS2811, 5, BRG>(controller.led_data().data(),
                               controller.led_data().size())
      .setCorrection(TypicalLEDStrip);
#else
  FastLED
      .addLeds<NEOPIXEL, 5>(controller.led_data().data(),
                            controller.led_data().size())
      .setCorrection(TypicalLEDStrip);
#endif
  FastLED.setBrightness(10);
  FastLED.setMaxRefreshRate(60, true);

  prefs.begin("wall_prefs");
  if (prefs.isKey(kTouchThresholdKey)) {
    Serial.printf("Touch threshold already set: %d\n",
                  prefs.getUShort(kTouchThresholdKey));
  } else {
    Serial.printf("Touch threshold unset, using default: %d\n",
                  kDefaultTouchThreshold);
  }
  touch_threshold = prefs.getUShort(kTouchThresholdKey, kDefaultTouchThreshold);
  touch_threshold = touchRead(kHandPin);
}

void animate() {
  controller.Update();
  FastLED.show();
}

void calibrate() {
  if (!calibrated) {
    calibration_step++;

    // Read the touch value
    uint16_t touch_value = touchRead(kHandPin);
    
    touch_threshold = ((touch_threshold * (calibration_step - 1)) + touch_value) / calibration_step;
    if (calibration_step == kCalibrationLimit) {
      calibrated = true;
    }
  }
}

void loop() {
  EVERY_N_SECONDS(1) {
    calibrate();

    Serial.printf("Current pattern: %d\n", controller.current_pattern_id());

    // Read the touch value
    uint16_t touch_value = touchRead(kHandPin);

    // Detect touch or release based on the threshold
    if (calibrated && touch_value < static_cast<uint16_t>(touch_threshold * 0.9)) {  // Touch detected
      if (!hand_pressed) {
        Serial.println("Button pressed!");
        SendHandEvent(HandEvent{.type = HandEventType::kPressed});
        hand_pressed = true;
      }
    } else {  // No touch detected
      if (hand_pressed) {
        Serial.println("Button released!");
        SendHandEvent(HandEvent{.type = HandEventType::kReleased});
        hand_pressed = false;
      }
    }
  }
  animate();

  
}
