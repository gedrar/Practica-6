#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <MFRC522.h>

// ---------------- PINS LECTOR SD (Bus 2) ----------------
#define SD_CS   39
#define SD_MOSI 35
#define SD_SCK  36
#define SD_MISO 37

// ---------------- PINS LECTOR RFID (Bus 1) ----------------
#define RFID_RST  9
#define RFID_MISO 13
#define RFID_MOSI 11
#define RFID_SCK  12
#define RFID_CS   10

// LA SOLUCIÓ: Utilitzem HSPI perquè no interfereixi amb el bus principal de l'ESP32
SPIClass spiSD(HSPI); 

// L'objecte RFID utilitza el bus SPI principal per defecte
MFRC522 mfrc522(RFID_CS, RFID_RST);

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); } 
  delay(1000);

  Serial.println("\n--- INICIANT DATALOGGER (CORREGIT) ---");

  // 1. Bus 1 (RFID)
  SPI.begin(RFID_SCK, RFID_MISO, RFID_MOSI, RFID_CS);
  mfrc522.PCD_Init();
  Serial.println("✅ Lector RFID iniciat (Bus 1)");

  // 2. Bus 2 (SD)
  spiSD.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  
  Serial.print("Iniciant targeta SD... ");
  if (!SD.begin(SD_CS, spiSD)) {
    Serial.println("❌ ERROR: No s'ha pogut iniciar la SD.");
  } else {
    Serial.println("✅ OK (Bus 2)");
  }

  Serial.println("-------------------------------------------");
  Serial.println("Apropa una targeta per registrar-la a fichero.log");
}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    
    String uidString = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      uidString += String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      uidString += String(mfrc522.uid.uidByte[i], HEX);
    }
    uidString.toUpperCase();
    
    unsigned long tempsSegons = millis() / 1000; 

    String registre = "Temps: " + String(tempsSegons) + "s | Codi:" + uidString;
    Serial.print("Llegit: " + registre);

    // Obrim l'arxiu. FILE_APPEND per defecte l'escriu a la línia següent.
    File myFile = SD.open("/fichero.log", FILE_APPEND);
    
    if (myFile) {
      myFile.println(registre); 
      myFile.close();           
      Serial.println("  --> 💾 Guardat a fichero.log correctament.");
    } else {
      Serial.println("  --> ❌ Error obrint fichero.log (Crea l'arxiu prèviament des de l'ordinador si no existeix)");
    }

    mfrc522.PICC_HaltA(); 
    delay(1000); 
  }
}