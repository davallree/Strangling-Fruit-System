#include "common.h"

#include <WiFi.h>
#include <esp_now.h>

#include <algorithm>
#include <array>
#include <iterator>

const MacAddress& EmptyMacAddress() {
  static MacAddress empty{};
  return empty;
}

std::string MacAddressToString(const MacAddress& mac) {
  char macStr[18];
  std::snprintf(macStr, 18, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1],
                mac[2], mac[3], mac[4], mac[5]);
  return std::string(macStr);
}

void InitEspNow() {
  WiFi.mode(WIFI_MODE_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
}

MacAddress MacAddressFromArray(const uint8_t* arr) {
  MacAddress result{};
  if (arr == nullptr) return result;
  for (int i = 0; i < result.size(); ++i) {
    result[i] = arr[i];
  }
  return result;
}

void AddPeer(const MacAddress& mac_addr) {
  // Note: {} default-initializes the struct. Since there are no default values,
  // this zeroes out the struct.
  esp_now_peer_info_t peer_info{};
  std::copy(mac_addr.begin(), mac_addr.end(), std::begin(peer_info.peer_addr));
  peer_info.channel = 0;
  peer_info.encrypt = false;

  if (esp_now_add_peer(&peer_info) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}