#ifndef INCLUDE_COMMON_COMMON_H_
#define INCLUDE_COMMON_COMMON_H_

#include <algorithm>
#include <array>
#include <cstdint>

// C++ wrapper around the MAC address array. Lets us compare MAC addresses using
// operator==.
using MacAddress = std::array<uint8_t, 6>;
const MacAddress& EmptyMacAddress();

// Create a MAC address from the given byte array. arr cannot be null.
MacAddress MacAddressFromArray(const uint8_t* arr);

// Broken for some reason
// std::string MacAddressToString(const MacAddress& addr);

// Initializes Wifi and ESPNow.
void InitEspNow();

// Add a peer with the given MAC address.
void AddPeer(const MacAddress& mac_addr);

#endif // INCLUDE_COMMON_COMMON_H_
