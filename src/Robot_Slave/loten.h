#ifndef loten
#define loten

#include <esp_now.h>
#include <WiFi.h>
#include <Arduino.h>
#include <AES.h>
#include <map>
#include <string>
#include <vector>
#define MAX_CONNECTIONS 20
#define PACKET_PAYLOAD_SIZE 223
#define MAX_PACKETS 100
 // Example prime modulus
const uint16_t generator = 23;
struct ReceivedData2 {
    std::vector<uint8_t> data;
    int totalPackets = 0;
    int receivedPackets = 0;
    uint16_t length; 
};
struct PacketHeader {
    uint8_t packetNumber;
    uint8_t totalPackets;
    char mac[20];
    uint16_t length; 
    bool handshake;
    
    bool encrypted;
};

struct Packet {
    PacketHeader header;
    uint8_t payload[PACKET_PAYLOAD_SIZE];
};

struct Key {
    uint16_t key;
};



struct ReceivedData {
    uint8_t* data;
    int totalPackets = 0;
    int receivedPackets = 0;
    uint16_t length; 
};
struct ConnectionInfo {
    uint8_t macAddress[6];
    uint16_t sharedKey;
};

typedef void (*ProcessReceivedDataFunc)(const uint8_t* data, uint16_t length);

extern ProcessReceivedDataFunc processReceivedDataCallback;
extern  uint16_t prime_modulus;
extern ConnectionInfo connectionList[MAX_CONNECTIONS];
extern int connectionCount;
extern uint16_t private_key;
extern uint16_t public_key;
extern esp_now_peer_info_t peerInfo;
void initializeKeys();
bool addConnection(const uint8_t* mac, uint16_t publicKey);
int findConnection(const uint8_t* mac);
uint16_t findSharedKey(const uint8_t* mac);
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);
void lotenSend(const void *data, size_t dataSize, const uint8_t *destMac, bool encrypted);
String convertMacAddressToString(uint8_t* macAddress);
uint16_t modPow(uint16_t base, uint16_t exponent, uint16_t modulus);
void convertMacAddress(String macAddressString, uint8_t * address);
void receivePacket(const Packet &packet);
void broadcastPublicKey();
void lotenInit(uint8_t macList[][6],  uint16_t prime_modulusc = 5669795882633);
void printConnectionList();
void sendPublicKey(const uint8_t *destMac);
void convertKey(uint16_t numericKey, uint8_t* keyArray);
void encryptData(uint8_t* data, size_t size, uint8_t* aes_key) ;
void decryptData(uint8_t* data, size_t size, uint8_t* aes_key);
void setProcessReceivedDataCallback(ProcessReceivedDataFunc callback);
#endif // ESP_NOW_UTILS_H
