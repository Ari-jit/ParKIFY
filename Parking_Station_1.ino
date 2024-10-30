//ParKIFY
//Parking Station 1
//All necessary libraries
#include <SPI.h>
#include "MFRC522.h"
#include <Wire.h> 
#include <Servo.h> 

Servo myservo; //Creating servo object

#define RST_PIN 9
#define SS_PIN 10

// Define I2C Address
#define PARKING_ADD_1 7

//To check which vehicle entered or exit
int Vehicle = 0;

MFRC522 mfrc522(SS_PIN, RST_PIN);

int IR1 = 5;
int IR2 = 6;

int Buzzer = 2;
int Green_Led = 3;
int Red_Led = 4;

int Slot = 4;           //Total number of parking Slots

//For checking the ir sensor data
int flag1 = 0;
int flag2 = 0;

//No. of registered vehicle
const int Listed = 6;
String Tag[Listed] = {"C3981E4","E3BDC0D9","13116AA4","39F29E4","A3B846E4","135F1BE4"}; // Change the RFID Data accordingly

int send = 0; // Data to send t0 host

void setup() {
  Serial.begin(9600);

  Wire.begin(PARKING_ADD_1); // Join I2C bus with address #8
  Wire.onRequest(requestEvent); // Register event

  SPI.begin();
  mfrc522.PCD_Init();

  pinMode(IR1, INPUT);
  pinMode(IR2, INPUT);

  pinMode(Buzzer, OUTPUT);
  pinMode(Green_Led, OUTPUT);
  pinMode(Red_Led, OUTPUT);
  
  myservo.attach(7);
  myservo.write(100); 
}

void loop(){ 
  send = (Slot+1)*100;
  if(digitalRead (IR1) == LOW && flag1==0){
    if(Slot>0){
      flag1=1;
      if(flag2==0){
        String rfid = getRFIDValue();
        if(rfid != ""){
          Serial.println("RFID Value: " + rfid);
          scan();
          int index = findIndexOf(Tag, Listed, rfid);
          if(index == -1)
            unconfirmed();
          else{
            Vehicle = index;
            myservo.write(0);
            Slot = Slot-1;
            send = (((Slot+1)*100)+((Vehicle+1)*10));
          }
        }
      }
    }
    else{
     Serial.println("Full");
     full();
    }
  }

  if(digitalRead (IR2) == LOW && flag2==0){
    flag2=1;
    if(flag1==0){
      String rfid = getRFIDValue();
      if(rfid != ""){
        scan();
        int index = findIndexOf(Tag, Listed, rfid);
        Vehicle = index;
        myservo.write(0);
        Slot = Slot+1;
        send = (((Slot+1)*100)+(Vehicle+1));
      }
    }
  }
  Serial.print("Send: ");
  Serial.println(send);

  if(flag1==1 && flag2==1){
    delay (1000);
    myservo.write(100);
    flag1=0, flag2=0;
  }

  Serial.print("Slot Left: ");
  Serial.println(Slot);
}


void requestEvent() {
  if (send) {
    Serial.print("Slot Left and Vehicle : ");
    Serial.println(send);
    Wire.write((byte*)&send, sizeof(send)); // Send the first integer
  }
}


void unconfirmed() {
  for(int i=0; i<5; i++){
    digitalWrite(Buzzer, HIGH);
    digitalWrite(Red_Led, HIGH); 
    delay(500);
    digitalWrite(Buzzer, LOW);
    digitalWrite(Red_Led, LOW); 
    delay(500);
  }

}


void scan() {
  digitalWrite(Buzzer, HIGH);
  digitalWrite(Green_Led, HIGH);
  delay(500);
  digitalWrite(Buzzer, LOW);
  digitalWrite(Green_Led, LOW); 

}

void full(){
  digitalWrite(Buzzer, HIGH);
  digitalWrite(Green_Led, HIGH);
  digitalWrite(Red_Led, HIGH);
  delay(1500);
  digitalWrite(Buzzer, LOW);
  digitalWrite(Green_Led, LOW);
  digitalWrite(Red_Led, LOW); 
}


String getRFIDValue() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return ""; // No new card present
  }
  if (!mfrc522.PICC_ReadCardSerial()) {
    return ""; // Failed to read card serial
  }

  String rfidValue = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    rfidValue.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  rfidValue.toUpperCase();
  mfrc522.PICC_HaltA(); // Stop reading

  return rfidValue;
}


int findIndexOf(String arr[], int size, String target) {
  for (int i = 0; i < Listed; i++) {
    if (arr[i] == target) {
      return i;
    }
  }
  return -1;
}