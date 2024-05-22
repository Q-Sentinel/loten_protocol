#include "loten.h"


ConnectionInfo connectionList[MAX_CONNECTIONS];
int connectionCount = 0;
uint16_t prime_modulus = 5669795882633;
uint16_t private_key = random(prime_modulus);
uint16_t public_key = modPow(generator, private_key, prime_modulus);
esp_now_peer_info_t peerInfo;
ProcessReceivedDataFunc processReceivedDataCallback = nullptr;

bool addConnection(const uint8_t* mac, uint16_t sharedKey) {
    if (connectionCount < MAX_CONNECTIONS) {
        memcpy(connectionList[connectionCount].macAddress, mac, 6);
        connectionList[connectionCount].sharedKey = sharedKey;
        connectionCount++;
        return true;
    }
    return false;
}

int findConnection(const uint8_t* mac) {
    for (int i = 0; i < connectionCount; i++) {
        if (memcmp(connectionList[i].macAddress, mac, 6) == 0) {
            return i;
        }
    }
    return -1;
}

uint16_t findSharedKey(const uint8_t* mac) {
    for (int i = 0; i < MAX_CONNECTIONS; ++i) {
        if (memcmp(connectionList[i].macAddress, mac, 6) == 0) {
  
            return connectionList[i].sharedKey;
        }
    }

    return 0; 
}
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("\r\nLast Packet Send Status:\t");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    Packet myData;
    memcpy(&myData, incomingData, sizeof(myData));

    uint8_t Addr[6];
    convertMacAddress(myData.header.mac, Addr);
    Key receivedKey;
    
    if (myData.header.handshake == true) {
        memcpy(&receivedKey, myData.payload, sizeof(Key));
        int connectionIndex = findConnection(Addr);
        uint16_t shared_secret = modPow(receivedKey.key, private_key, prime_modulus);
        if (connectionIndex == -1) {
          
            if (addConnection(Addr, shared_secret)) {
                Serial.println("New connection added");
            } else {
                Serial.println("Connection list is full");
            }
        
            sendPublicKey(Addr);
        } else {
            
            if (connectionList[connectionIndex].sharedKey != shared_secret) {
                connectionList[connectionIndex].sharedKey = shared_secret;
                Serial.println("Connection updated with new public key");sendPublicKey(Addr);
            } else {
                Serial.println("Public key already exists");
            }
    
            
        }
        Serial.println("handshaking");
        Serial.println(receivedKey.key);
    } else {
        receivePacket(myData);
    }
}

void lotenSend(const void *data, size_t dataSize, const uint8_t *destMac, bool encrypted) {


    byte serializedData[dataSize];
    memcpy(serializedData, data, dataSize);
    
    int totalPackets = dataSize / PACKET_PAYLOAD_SIZE + 1;

    if (encrypted) {
          uint8_t aes_key[16] ;
          convertKey( findSharedKey(destMac) , aes_key);
          encryptData((uint8_t*)&serializedData, sizeof(serializedData), aes_key);
    }

        for (int i = 0; i < totalPackets; ++i) {
            Packet packet;
            packet.header.packetNumber = i;
            packet.header.totalPackets = totalPackets;
            packet.header.handshake = false;
            packet.header.encrypted = encrypted;
            packet.header.length = dataSize;
            String Address = WiFi.macAddress();
            Address.toCharArray(packet.header.mac, 20);
            int payloadSize = min(PACKET_PAYLOAD_SIZE, (int)dataSize - i * PACKET_PAYLOAD_SIZE);
            memcpy(packet.payload, serializedData + i * PACKET_PAYLOAD_SIZE, payloadSize);
            esp_err_t result = esp_now_send(destMac, (uint8_t *) &packet, sizeof(Packet));

            if (result == ESP_OK) {
                Serial.println("Sent with success");
            } else {
                Serial.println("Error sending the data");
            }
        }
    
}

String convertMacAddressToString(uint8_t* macAddress) {
    String macAddressString;
    for (int i = 0; i < 6; i++) {
        macAddressString += String(macAddress[i], HEX);
        if (i < 6 - 1) {
            macAddressString += ":";
        }
    }
    return macAddressString;
}

uint16_t modPow(uint16_t base, uint16_t exponent, uint16_t modulus) {
    uint16_t result = 1;
    base = base % modulus;
    while (exponent > 0) {
        if (exponent & 1) {
            result = (result * base) % modulus;
        }
        exponent = exponent >> 1;
        base = (base * base) % modulus;
    }
    return result;
}

void convertMacAddress(String macAddressString, uint8_t * address) {
    for (int i = 0; i < 6; i++) {
        String byteString = macAddressString.substring(i * 3, i * 3 + 2);
        while (byteString.length() < 2) {
            byteString = "0" + byteString;
        }
        address[i] = (uint8_t)strtol(byteString.c_str(), NULL, 16);
    }
}


void receivePacket(const Packet &packet) {
    static ReceivedData receivedData;
    uint8_t Addr[6];

    convertMacAddress(packet.header.mac, Addr);
    int packetNumber = packet.header.packetNumber;
    int totalPackets = packet.header.totalPackets;
    receivedData.length = packet.header.length;

    if (packetNumber >= MAX_PACKETS) {
        Serial.println("Packet number out of bounds");
        return;
    }

    bool encrypted = packet.header.encrypted;
    size_t expectedSize = PACKET_PAYLOAD_SIZE * totalPackets;

    if (receivedData.data == nullptr) {
        receivedData.data = new uint8_t[expectedSize]; // Allocate memory
    }

    size_t payloadSize = min(PACKET_PAYLOAD_SIZE, receivedData.length - packetNumber * PACKET_PAYLOAD_SIZE);
    
    if (payloadSize > 0) {
        memcpy(receivedData.data + packetNumber * PACKET_PAYLOAD_SIZE, packet.payload, payloadSize);
    }

    receivedData.receivedPackets++;

    if (receivedData.receivedPackets == totalPackets) {
        if (encrypted) {
            uint8_t aes_key[16];
            convertKey(findSharedKey(Addr), aes_key);
            decryptData(receivedData.data, expectedSize, aes_key);
        }

        if (processReceivedDataCallback) {
            processReceivedDataCallback(receivedData.data, receivedData.length);
        }

        delete[] receivedData.data; // Free memory
        receivedData.data = nullptr;
        receivedData.totalPackets = 0;
        receivedData.receivedPackets = 0;
    }
}

  // void receivePacket(const Packet &packet) {
  //     static ReceivedData receivedData;
  //     uint8_t Addr[6];
      
  //     convertMacAddress(packet.header.mac, Addr);
  //     int packetNumber = packet.header.packetNumber;
  //     int totalPackets = packet.header.totalPackets;
  //     receivedData.length = packet.header.length;
  //     if (packetNumber >= MAX_PACKETS) {
  //         Serial.println("Packet number out of bounds");
  //         return;
  //     }
  //     bool encrypted = packet.header.encrypted;
  //     int payloadSize = min(PACKET_PAYLOAD_SIZE, (int)receivedData.length - packetNumber * PACKET_PAYLOAD_SIZE);
  //     memcpy(receivedData.data + packetNumber * PACKET_PAYLOAD_SIZE, packet.payload, payloadSize);

  //     receivedData.receivedPackets++;

  //     if (receivedData.receivedPackets == totalPackets) {
  //         if(encrypted){
  //           uint8_t aes_key[16] ;
  //           convertKey( findSharedKey(Addr) , aes_key);
  //           decryptData((uint8_t*)&receivedData.data, sizeof(receivedData.data), aes_key);

  //         }
  //               if (processReceivedDataCallback) {
  //                 if (processReceivedDataCallback) {
  //             processReceivedDataCallback(receivedData.data, receivedData.length);
  //         }
  //                 delete[] receivedData.data; // Free memory
  //         receivedData.data = nullptr;
  //         }
  //         receivedData.totalPackets = 0;
  //         receivedData.receivedPackets = 0;
  //         memset(&receivedData.data, 0, sizeof(hi));
  //     }
  // }

void setProcessReceivedDataCallback(ProcessReceivedDataFunc callback) {
    processReceivedDataCallback = callback;
}
void broadcastPublicKey() {
  Serial.println("fuc");
    for (int i = 0; i < connectionCount; i++) {
        Packet packet;
        Key key_c;
        key_c.key = public_key;

        packet.header.packetNumber = 0;
        packet.header.totalPackets = 0;
        packet.header.handshake = true;
        packet.header.encrypted = false;
        String Address = WiFi.macAddress();
        Address.toCharArray(packet.header.mac, 20);

        memcpy(packet.payload, &key_c, sizeof(Key));
        esp_err_t result = esp_now_send(connectionList[i].macAddress, (uint8_t *) &packet, sizeof(Packet));

        if (result == ESP_OK) {
            Serial.println("Public key sent with success");
        } else {
            Serial.println("Error sending the public key");
        }
    }
}
void lotenInit(uint8_t macList[][6] ,uint16_t prime_modulusc ) {
      if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
       esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);
  prime_modulus = prime_modulusc;
  uint8_t ownMac[6];
    WiFi.macAddress(ownMac);
    int count = sizeof(macList);
    for (int i = 0; i < count; i++) {
      
        if (memcmp(macList[i], ownMac, 6) == 0) {
            Serial.print("Ignoring own MAC address: ");
            Serial.println(convertMacAddressToString(ownMac));
            continue;
        }
        memcpy(peerInfo.peer_addr, macList[i], 6);
        peerInfo.channel = 0;
        peerInfo.encrypt = false;

        if (esp_now_add_peer(&peerInfo) != ESP_OK) {
            Serial.print("Failed to add peer: ");
            Serial.println(convertMacAddressToString(macList[i]));
            return;
        }

        addConnection(macList[i], 0); 
    }
     broadcastPublicKey();
}

void printConnectionList() {
    Serial.println("Connection List:");
    for (int i = 0; i < connectionCount; i++) {
        Serial.print("MAC Address: ");
        Serial.print(convertMacAddressToString(connectionList[i].macAddress));
        Serial.print(" | Public Key: ");
        Serial.println(connectionList[i].sharedKey);
    }
}

void sendPublicKey(const uint8_t *destMac) {
    Packet packet;
    Key key_c;
    key_c.key = public_key;

    packet.header.packetNumber = 0;
    packet.header.totalPackets = 0;
    packet.header.handshake = true;
    packet.header.encrypted = false;
    String Address = WiFi.macAddress();
    Address.toCharArray(packet.header.mac, 20);

    memcpy(packet.payload, &key_c, sizeof(Key));
    esp_err_t result = esp_now_send(destMac, (uint8_t *)&packet, sizeof(Packet));

    if (result == ESP_OK) {
        Serial.println("Public key sent with success");
    } else {
        Serial.println("Error sending public key");
    }
}


void convertKey(uint16_t numericKey, uint8_t* keyArray) {
   
    for (int i = 0; i < 16; i++) {
        keyArray[i] = (numericKey >> (i * 4)) & 0xFF;
    }
}

void encryptData(uint8_t* data, size_t size, uint8_t* aes_key) {
    AES aesEncryptor;
    aesEncryptor.set_key(aes_key, sizeof(aes_key));
    aesEncryptor.encrypt(data, data);
}


void decryptData(uint8_t* data, size_t size,uint8_t* aes_key) {
    AES aesDecryptor;
    aesDecryptor.set_key(aes_key, sizeof(aes_key));
    aesDecryptor.decrypt(data, data);
}