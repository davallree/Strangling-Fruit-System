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

// The MAC address of the master controller. Set once the master sends a
// message.
MacAddress master_address;

// Whether the hand is currently pressed or not.
constexpr uint8_t kHandPin = BUTTON;
bool hand_pressed = false;

LEDController controller;
#define NUM_LEDS 256

byte coordsX[NUM_LEDS] = {
    0,   17,  34,  51,  68,  85,  102, 119, 136, 153, 170, 187, 204, 221, 238,
    255, 255, 238, 221, 204, 187, 170, 153, 136, 119, 102, 85,  68,  51,  34,
    17,  0,   0,   17,  34,  51,  68,  85,  102, 119, 136, 153, 170, 187, 204,
    221, 238, 255, 255, 238, 221, 204, 187, 170, 153, 136, 119, 102, 85,  68,
    51,  34,  17,  0,   0,   17,  34,  51,  68,  85,  102, 119, 136, 153, 170,
    187, 204, 221, 238, 255, 255, 238, 221, 204, 187, 170, 153, 136, 119, 102,
    85,  68,  51,  34,  17,  0,   0,   17,  34,  51,  68,  85,  102, 119, 136,
    153, 170, 187, 204, 221, 238, 255, 255, 238, 221, 204, 187, 170, 153, 136,
    119, 102, 85,  68,  51,  34,  17,  0,   0,   17,  34,  51,  68,  85,  102,
    119, 136, 153, 170, 187, 204, 221, 238, 255, 255, 238, 221, 204, 187, 170,
    153, 136, 119, 102, 85,  68,  51,  34,  17,  0,   0,   17,  34,  51,  68,
    85,  102, 119, 136, 153, 170, 187, 204, 221, 238, 255, 255, 238, 221, 204,
    187, 170, 153, 136, 119, 102, 85,  68,  51,  34,  17,  0,   0,   17,  34,
    51,  68,  85,  102, 119, 136, 153, 170, 187, 204, 221, 238, 255, 255, 238,
    221, 204, 187, 170, 153, 136, 119, 102, 85,  68,  51,  34,  17,  0,   0,
    17,  34,  51,  68,  85,  102, 119, 136, 153, 170, 187, 204, 221, 238, 255,
    255, 238, 221, 204, 187, 170, 153, 136, 119, 102, 85,  68,  51,  34,  17,
    0};
byte coordsY[NUM_LEDS] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   17,  17,  17,  17,  17,  17,  17,  17,  17,  17,  17,  17,  17,  17,
    17,  17,  34,  34,  34,  34,  34,  34,  34,  34,  34,  34,  34,  34,  34,
    34,  34,  34,  51,  51,  51,  51,  51,  51,  51,  51,  51,  51,  51,  51,
    51,  51,  51,  51,  68,  68,  68,  68,  68,  68,  68,  68,  68,  68,  68,
    68,  68,  68,  68,  68,  85,  85,  85,  85,  85,  85,  85,  85,  85,  85,
    85,  85,  85,  85,  85,  85,  102, 102, 102, 102, 102, 102, 102, 102, 102,
    102, 102, 102, 102, 102, 102, 102, 119, 119, 119, 119, 119, 119, 119, 119,
    119, 119, 119, 119, 119, 119, 119, 119, 136, 136, 136, 136, 136, 136, 136,
    136, 136, 136, 136, 136, 136, 136, 136, 136, 153, 153, 153, 153, 153, 153,
    153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 170, 170, 170, 170, 170,
    170, 170, 170, 170, 170, 170, 170, 170, 170, 170, 170, 187, 187, 187, 187,
    187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 204, 204, 204,
    204, 204, 204, 204, 204, 204, 204, 204, 204, 204, 204, 204, 204, 221, 221,
    221, 221, 221, 221, 221, 221, 221, 221, 221, 221, 221, 221, 221, 221, 238,
    238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255};
byte angles[NUM_LEDS] = {
    32,  35,  38,  41,  45,  49,  54,  59,  64,  69,  74,  78,  83,  86,  90,
    93,  96,  93,  89,  85,  80,  75,  70,  64,  58,  52,  47,  43,  39,  35,
    32,  29,  26,  29,  32,  36,  40,  45,  51,  57,  64,  70,  77,  83,  88,
    92,  96,  99,  102, 99,  96,  91,  86,  79,  72,  64,  56,  48,  42,  36,
    32,  28,  25,  23,  19,  21,  24,  27,  32,  38,  45,  54,  64,  74,  83,
    90,  96,  100, 104, 106, 111, 109, 106, 101, 96,  88,  77,  64,  51,  40,
    32,  26,  22,  19,  16,  15,  10,  11,  13,  15,  19,  24,  32,  45,  64,
    83,  96,  104, 109, 112, 114, 116, 122, 121, 119, 118, 114, 109, 96,  64,
    32,  19,  13,  10,  8,   7,   6,   5,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   128, 128, 128, 128, 128, 128, 128, 133, 134, 136, 137, 141, 146,
    159, 191, 223, 236, 242, 245, 247, 248, 249, 250, 245, 244, 242, 240, 236,
    231, 223, 210, 191, 172, 159, 151, 146, 143, 141, 139, 144, 146, 149, 154,
    159, 167, 178, 191, 204, 215, 223, 229, 233, 236, 239, 240, 236, 234, 231,
    228, 223, 217, 210, 201, 191, 181, 172, 165, 159, 155, 151, 149, 153, 156,
    159, 164, 169, 176, 183, 191, 199, 207, 213, 219, 223, 227, 230, 232, 229,
    226, 223, 219, 215, 210, 204, 198, 191, 185, 178, 172, 167, 163, 159, 156,
    159, 162, 166, 170, 175, 180, 185, 191, 197, 203, 208, 212, 216, 220, 223,
    226};
byte radii[NUM_LEDS] = {
    255, 240, 225, 213, 202, 193, 186, 182, 180, 182, 186, 193, 202, 213, 225,
    240, 223, 208, 194, 182, 172, 164, 159, 158, 159, 164, 172, 182, 194, 208,
    223, 240, 225, 208, 191, 176, 163, 151, 143, 137, 135, 137, 143, 151, 163,
    176, 191, 208, 194, 176, 159, 144, 131, 121, 115, 113, 115, 121, 131, 144,
    159, 176, 194, 213, 202, 182, 163, 144, 128, 113, 101, 93,  90,  93,  101,
    113, 128, 144, 163, 182, 172, 151, 131, 113, 96,  81,  71,  68,  71,  81,
    96,  113, 131, 151, 172, 193, 186, 164, 143, 121, 101, 81,  64,  50,  45,
    50,  64,  81,  101, 121, 143, 164, 159, 137, 115, 93,  71,  50,  32,  23,
    32,  50,  71,  93,  115, 137, 159, 182, 180, 158, 135, 113, 90,  68,  45,
    23,  0,   23,  45,  68,  90,  113, 135, 158, 159, 137, 115, 93,  71,  50,
    32,  23,  32,  50,  71,  93,  115, 137, 159, 182, 186, 164, 143, 121, 101,
    81,  64,  50,  45,  50,  64,  81,  101, 121, 143, 164, 172, 151, 131, 113,
    96,  81,  71,  68,  71,  81,  96,  113, 131, 151, 172, 193, 202, 182, 163,
    144, 128, 113, 101, 93,  90,  93,  101, 113, 128, 144, 163, 182, 194, 176,
    159, 144, 131, 121, 115, 113, 115, 121, 131, 144, 159, 176, 194, 213, 225,
    208, 191, 176, 163, 151, 143, 137, 135, 137, 143, 151, 163, 176, 191, 208,
    223, 208, 194, 182, 172, 164, 159, 158, 159, 164, 172, 182, 194, 208, 223,
    240};

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
  MasterCommand command;
  memcpy(&command, data, sizeof(command));
  Serial.printf("Received command, switching to %d\n",
                command.animation_to_play);
  controller.SetCurrentAnimation(command.animation_to_play);
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
  controller.InitLEDs(NUM_LEDS, coordsX, coordsY, angles, radii);
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
    Serial.printf("Current animation: %d\n", controller.current_animation());
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
