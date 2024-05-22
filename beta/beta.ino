#include "loten.h"

typedef struct hi {
    int temp[100];
} hi;

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  uint8_t destMac[] = {0xB0, 0xB2, 0x1C, 0xB1, 0xD1, 0xA8};
  //B0:B2:1C:B1:D2:A4
uint8_t destMacList[MAX_CONNECTIONS][6] = {
    {0xB0, 0xB2, 0x1C, 0xB1, 0xD1, 0xA8},
    {0xB0, 0xB2, 0x1C, 0xB1, 0xD2, 0xA4}

};
void userProcessReceivedData(const uint8_t* data, uint16_t length) {
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
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);



  

        // uint8_t destMac[] = {0xB0, 0xB2, 0x1C, 0xB1, 0xD1, 0xA8};




    // void initializeKeys();
 
    setProcessReceivedDataCallback(userProcessReceivedData);
    lotenInit(destMacList);

}

void loop() {
    hi data;
    for (int i = 0; i < 100; i++) {
        data.temp[i] = i + 1;
    }
//B0:B2:1C:B1:D1:A8
 
    lotenSend(&data, sizeof(data), destMac , true);

    delay(10000);
}
