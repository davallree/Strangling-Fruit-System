#include <WiFi.h>
#include <esp_now.h>

// Structure example to receive data
typedef struct struct_message {
    char message[32];
    uint8_t senderAddress[6];
} struct_message;

struct_message incomingMessage;

// State tracking for each sender
bool sender1Pressed = false;
bool sender2Pressed = false;

uint8_t sender1Address[] = {0x0C, 0x8B, 0x95, 0x93, 0x60, 0xF8}; // Replace with actual address
uint8_t sender2Address[] = {0x0C, 0x8B, 0x95, 0x96, 0xC6, 0xB4}; // Replace with actual address

// Callback function that will be executed when data is received
void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
    memcpy(&incomingMessage, incomingData, sizeof(incomingMessage));

    bool stateChanged = false;
    if (memcmp(mac, sender1Address, 6) == 0) {
        if (strcmp(incomingMessage.message, "Pressed") == 0) {
            sender1Pressed = true;
            stateChanged = true;
        } else if (strcmp(incomingMessage.message, "Released") == 0) {
            sender1Pressed = false;
            stateChanged = true;
        }
    } else if (memcmp(mac, sender2Address, 6) == 0) {
        if (strcmp(incomingMessage.message, "Pressed") == 0) {
            sender2Pressed = true;
            stateChanged = true;
        } else if (strcmp(incomingMessage.message, "Released") == 0) {
            sender2Pressed = false;
            stateChanged = true;
        }
    }

    if (stateChanged) {
        sendState();
        Serial.print("State changed: ");
        Serial.println(sender1Pressed + sender2Pressed);
    }
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

    // Print the MAC address of the receiver
    printMACAddress();

    // Set Wi-Fi to station mode and disconnect from an AP if it was previously connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    Serial.println("ESP-NOW initialized");

    // Register the callback function
    esp_now_register_recv_cb(onDataRecv);
    Serial.println("ESP-NOW initialized and waiting for data...");

    // Add peers
    addPeer(sender1Address);
    addPeer(sender2Address);
}

void addPeer(uint8_t* address) {
    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo));
    memcpy(peerInfo.peer_addr, address, 6);
    peerInfo.channel = 0;  // Channel 0 means the same channel as the Wi-Fi
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
    }
}

void sendState() {
    char stateMessage[32];
    int pressedCount = sender1Pressed + sender2Pressed;
    snprintf(stateMessage, sizeof(stateMessage), "%d button%s pressed", pressedCount, pressedCount == 1 ? "" : "s");

    strcpy(incomingMessage.message, stateMessage);

    esp_now_send(sender1Address, (uint8_t*)&incomingMessage, sizeof(incomingMessage));
    esp_now_send(sender2Address, (uint8_t*)&incomingMessage, sizeof(incomingMessage));

    // Also send over serial to the connected computer
    Serial.println(stateMessage);
}

void loop() {
    // No need for code here, everything is handled by ESP-NOW and callbacks
}
