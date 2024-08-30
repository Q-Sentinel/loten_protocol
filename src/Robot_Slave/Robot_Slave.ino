#include "loten.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

#define TRIG_PIN 5 // Pin connected to TRIG pin of the sensor
#define ECHO_PIN 5 // Pin connected to ECHO pin of the sensor
#define LDR_PIN 36

#define MOTOR1_P 16
#define MOTOR1_N 17
#define MOTOR2_P 27
#define MOTOR2_N 18
#define LED  33

Adafruit_SSD1306 display(128, 64);

int distance, ldrval;
int offset = -20;

typedef struct hi {
    int temp[2];
} hi;

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  uint8_t mast[] = {0xB0, 0xB2, 0x1C, 0xA8, 0xDD, 0x00};
  //B0:B2:1C:B1:D2:A4
// uint8_t destMacList[MAX_CONNECTIONS][6] = {
//     {0xB0, 0xB2, 0x1C, 0xB1, 0xD1, 0xA8},
//     {0xB0, 0xB2, 0x1C, 0xB1, 0xD2, 0xA4},
//     {0x94, 0xE6, 0x86, 0x05, 0x54, 0x14}, // Master
//     {0xB0, 0xB2, 0x1C, 0xA8, 0xDD, 0x00}

// };
uint8_t destMacList[MAX_CONNECTIONS][6] = {
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}


};

long measure_speed(){
  int duration, distance;
  
  // Clear the TRIG pin
  pinMode(TRIG_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(10);

  // Send a 10 microsecond pulse to the TRIG pin
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read the duration of the echo pulse
  pinMode(ECHO_PIN, INPUT);
  duration = pulseIn(ECHO_PIN, HIGH);

  // Calculate the distance in centimeters
  distance = duration * 0.034 / 2;

  // Print the distance to the Serial Monitor
 

  delay(500); // Wait for half a second before next measurement

  return distance;

}

void goForward(int delay_str){
  analogWrite(MOTOR1_N, 200 - offset);
  digitalWrite(MOTOR1_P, 0);
  analogWrite(MOTOR2_N, 200+ offset);
  digitalWrite(MOTOR2_P, 0);

  delay(delay_str);
  analogWrite(MOTOR1_N, 255);
  digitalWrite(MOTOR1_P, 255);
  analogWrite(MOTOR2_N, 255);
  digitalWrite(MOTOR2_P, 255);

}

void turnLeft(int delay_str){
  analogWrite(MOTOR1_N, 200);
  digitalWrite(MOTOR1_P, 0);
  analogWrite(MOTOR2_N, 0);
  digitalWrite(MOTOR2_P, 0);

  delay(delay_str);
  analogWrite(MOTOR1_N, 255);
  digitalWrite(MOTOR1_P, 255);
  analogWrite(MOTOR2_N, 255);
  digitalWrite(MOTOR2_P, 255);

}

void turnRight(int delay_str){
  analogWrite(MOTOR1_N, 0);
  digitalWrite(MOTOR1_P, 0);
  analogWrite(MOTOR2_N, 200);
  digitalWrite(MOTOR2_P, 0);

  delay(delay_str);
  analogWrite(MOTOR1_N, 255);
  digitalWrite(MOTOR1_P, 255);
  analogWrite(MOTOR2_N, 255);
  digitalWrite(MOTOR2_P, 255);

}

void turnAround(){
  analogWrite(MOTOR1_N, 0);
  digitalWrite(MOTOR1_P, 100);
  analogWrite(MOTOR2_N, 100);
  digitalWrite(MOTOR2_P, 0);

  delay(2000);
  digitalWrite(LED, HIGH);
  analogWrite(MOTOR1_N, 255);
  digitalWrite(MOTOR1_P, 255);
  analogWrite(MOTOR2_N, 255);
  digitalWrite(MOTOR2_P, 255);

}

void oled_print(){
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(distance);
  display.setCursor(0, 20);
  display.println(ldrval);
  display.display();


}
void userProcessReceivedData(const uint8_t* data, uint16_t length) {
    Serial.println("User defined received data:");
    if (length != sizeof(hi)) {
        Serial.println("Received data length does not match hi structure size");
        return;
    }
    hi receivedData;
    memcpy(&receivedData, data, sizeof(hi));

    for (int i = 0; i < 2; i++) {
        Serial.println(receivedData.temp[i]);
    }

    if (receivedData.temp[1]>1500){
      turnAround();
      digitalWrite(LED, HIGH);
    }
    else{
      digitalWrite(LED, LOW);
    }
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);

    analogSetAttenuation(ADC_11db);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    delay(500);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);

    display.println("Slave 4");

    display.display();
    pinMode(MOTOR1_P, OUTPUT);
    pinMode(MOTOR1_N, OUTPUT);
    pinMode(MOTOR2_P, OUTPUT);
    pinMode(MOTOR2_N, OUTPUT);
    
    pinMode(LED,OUTPUT);
    digitalWrite(LED, LOW);



  

        // uint8_t destMac[] = {0xB0, 0xB2, 0x1C, 0xB1, 0xD1, 0xA8};




    // void initializeKeys();
 
    setProcessReceivedDataCallback(userProcessReceivedData);
    lotenInit(destMacList);

}

void loop() {
   

//B0:B2:1C:B1:D1:A8
 
    //lotenSend(&data, sizeof(data), mast , true);

    distance = measure_speed();
    ldrval = analogRead(LDR_PIN);
    hi data;
    
    data.temp[0] = distance;
    data.temp[1] = ldrval;

    //oled_print();
    
//B0:B2:1C:B1:D1:A8
 
    

    
    if(ldrval > 1500){
      lotenSend(&data, sizeof(data), broadcastAddress , false);
      turnAround();
      
    }
    else{
      digitalWrite(LED, LOW);

    }

    delay(100);

   
}
