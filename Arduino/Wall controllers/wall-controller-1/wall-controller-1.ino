#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <FastLED.h>

// Replace with your receiver's MAC address
uint8_t receiverAddress[] = {0x0C, 0x8B, 0x95, 0x96, 0x41, 0xF4};

typedef struct struct_message {
    char message[32];
} struct_message;

struct_message myData;

// Capacitive touch pin and threshold
const int touchPin = T0; // GPIO4
const int touchThreshold = 40;

// Variable to keep track of touch state
bool touchDetected = false;

// FastLED configuration
#define LED_PIN     5
#define NUM_LEDS    146
#define BRIGHTNESS  255 // Brightness level (0-255)
CRGB leds[NUM_LEDS];

void onSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("\r\nLast Packet Send Status:\t");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void printMACAddress() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    Serial.print("MAC Address: ");
    for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", mac[i]);
        if (i < 5) Serial.print(":");
    }
    Serial.println();
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    // Print the MAC address of the sender
    printMACAddress();

    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Register send callback function
    esp_now_register_send_cb(onSent);

    // Set up peer information
    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo));
    memcpy(peerInfo.peer_addr, receiverAddress, 6);
    peerInfo.channel = 0;  // Channel 0 means the same channel as the Wi-Fi
    peerInfo.encrypt = false;

    // Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }

    // Initialize FastLED
    FastLED.addLeds<WS2811, LED_PIN, RGB>(leds, NUM_LEDS);//.setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.clear();
    FastLED.show();
}

void loop() {
    // Read touch sensor value
    int touchValue = touchRead(touchPin);

    // Print the touch value for debugging
    Serial.print("Touch Value: ");
    Serial.println(touchValue);

    // Check if touch value is below the threshold
    if (touchValue < touchThreshold) {
        if (!touchDetected) {
            Serial.println("Touch detected!");
            touchDetected = true;  // Set the state to indicate touch is detected
            sendMessage("Pressed"); // Send message
            turnOnLEDs();           // Turn on LEDs
        }
    } else {
        if (touchDetected) {
            touchDetected = false; // Reset the state when no touch is detected
            sendMessage("Released"); // Send message
            turnOffLEDs();           // Turn off LEDs
        }
    }

    // Small delay to avoid flooding the serial output
    delay(100);
}

void sendMessage(const char* message) {
    digitalWrite(LED_BUILTIN, HIGH); // Turn on LED
    delay(100); // Keep the LED on for 100 milliseconds for visibility

    strcpy(myData.message, message);
    esp_err_t result = esp_now_send(receiverAddress, (uint8_t *) &myData, sizeof(myData));

    if (result == ESP_OK) {
        Serial.println("Message sent successfully");
    } else {
        Serial.println("Error sending the message");
    }

    digitalWrite(LED_BUILTIN, LOW); // Turn off LED
}

void turnOnLEDs() {
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB(0, 0, 255);
    }
    FastLED.show();
}

void turnOffLEDs() {
    FastLED.clear();
    FastLED.show();
}
