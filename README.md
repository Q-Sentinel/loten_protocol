

# QSentinel Loten Protocol

## Overview

The QSentinel Loten Protocol is a new network layer protocol designed to work on top of the ESP-NOW protocol. It addresses the limitations of the ESP-NOW protocol, such as the payload size limitation of only 250 bytes and the lack of secure and efficient encryption. Loten Protocol provides a solution to these issues, offering enhanced payload size, improved security, and faster encryption.

### Features

- Increased payload size support
- Enhanced security with efficient encryption
- Compatible with ESP-NOW protocol
- Easy integration with existing projects

## Version 1.0.0-beta

This is the beta release of the QSentinel Loten Protocol. In this version, we have focused on solving the payload size limitation and improving encryption. It's important to note that this version is still in the testing phase and may contain bugs.
## ESP-NOW
1. Peer-to-Peer (P2P) Connection: ESP-NOW enables direct communication between ESP32 devices, eliminating the routing overhead associated with traditional Wi-Fi networks.

2. Lightweight and Efficient: Designed for resource-constrained devices, ESP-NOW has a minimal footprint, making it well-suited for ESP32 microcontrollers with limited resources.

3. Low-Latency Data Transmission: By sidestepping routing delays, ESP-NOW facilitates rapid data exchange between connected devices, ideal for real-time applications requiring swift responsiveness.

## Applications

1. Sensor Networks: Efficiently collect data from multiple sensors to a central ESP32 device.
2. Industrial Automation: Enable real-time control and communication between devices in industrial environments.
3. Smart Home Automation: Facilitate low-latency communication between smart home devices for tasks like lighting control or appliance management.
4. Drone Communication: Ensure reliable data exchange between drones and ground control units.
### Planned Features for Next Version

In the upcoming version, we plan to introduce a master-slave communication API, providing more flexibility and functionality to the protocol.

## Contribution

We welcome contributions from the community! If you have any suggestions, bug fixes, or feature requests, please open an issue or submit a pull request.

## Getting Started

To use the Loten Protocol in your project, follow these steps:

1. Install `esp_now` and `AESLib` libraries.
2. Set up the data receiving callback function using `setProcessReceivedDataCallback(userProcessReceivedData)`.
3. Initialize the protocol using `lotenInit(destMacList)`. Provide a list of destination MAC addresses (`destMacList`) in the setup function.

```cpp
// Example Usage
#include "loten.h"

typedef struct hi {
    int temp[100];
} hi;


// Define the data receiving callback function
void userProcessReceivedData(const uint8_t* data, uint16_t length) {
    // Your custom data processing logic here
       Serial.println("User defined received data:");
    if (length != sizeof(hi)) {
        Serial.println("Received data length does not match hi structure size");
        return;
    }
    hi receivedData;
    memcpy(&receivedData, data, sizeof(hi));

    for (int i = 0; i < 100; i++) {
        Serial.println(receivedData.temp[i]);
    }
}

void setup() {
    // Initialize serial communication
    Serial.begin(115200);

    // Install ESP-NOW libraries and set up callback function
    setProcessReceivedDataCallback(userProcessReceivedData);

    // Initialize Loten Protocol with destination MAC list
    uint8_t destMacList[MAX_CONNECTIONS][6] = {
        {0xB0, 0xB2, 0x1C, 0xB1, 0xD1, 0xA8},
        {0xB0, 0xB2, 0x1C, 0xB1, 0xD2, 0xA4}
    };
    lotenInit(destMacList);

 
}

void loop() {
    // Your loop code here
   // Example data sending
    hi data;
    for (int i = 0; i < 100; i++) {
        data.temp[i] = i + 1;
    }
    lotenSend(&data, sizeof(data), destMacList[0], true);
}

