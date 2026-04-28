#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

// Nous pins per a l'SPI3 de l'ESP32-S3 (lliures!)
#define SCK_PIN 12
#define MISO_PIN 13
#define MOSI_PIN 11
#define SS_PIN 10
#define RST_PIN 9

MFRC522 mfrc522(SS_PIN, RST_PIN); 

void setup() {
  Serial.begin(115200); 
  
  while (!Serial) { delay(10); } 
  delay(1000);

  // Forcem l'SPI a utilitzar els nous pins (SPI3)
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  
  mfrc522.PCD_Init(); 
  
  Serial.println("Lector RFID preparat als nous pins!");
  Serial.println("Apropa una targeta o clauer al lector...");
}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent()) {
    if (mfrc522.PICC_ReadCardSerial()) {
      Serial.print("Card UID:");
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
      }
      Serial.println();
      mfrc522.PICC_HaltA(); 
    }
  }
}
