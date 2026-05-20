#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

// Pins exactes per al bus SPI2 de l'ESP32-S3 segons el teu document
#define SCK_PIN 36
#define MISO_PIN 37
#define MOSI_PIN 35
#define CS_PIN 39 

File myFile;

void setup() {
  Serial.begin(115200); 
  
  // Aquest bucle fa que la placa t'esperi pacientment. 
  // No avançarà fins que no obris el Monitor Sèrie!
  while (!Serial) {
    delay(10);
  }
  
  // Un cop obert el terminal, fem un delay de cortesia
  delay(1000);

  Serial.println("\n--- Iniciant prova de SD ---");

  // 1. Inicialitzem el bus SPI amb els nostres pins
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);

  // 2. Inicialitzem la SD indicant-li quin CS i quin bus SPI ha d'utilitzar!
  if (!SD.begin(CS_PIN, SPI)) {
    Serial.println("❌ Error: No se pudo inicializar la SD.");
    Serial.println("Comprova els cables o si la targeta esta ben ficada.");
    return; // Ens aturem aquí
  }
  
  Serial.println("✅ Inicializacion exitosa!");

  // Obrim el fitxer (recorda tenir archivo.txt creat a la targeta)
  myFile = SD.open("/archivo.txt"); 
  
  if (myFile) {
    Serial.println("📄 Contingut de archivo.txt:");
    Serial.println("---------------------------");
    
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    
    myFile.close(); 
    Serial.println("\n---------------------------");
    Serial.println("Fi de la lectura");
    
  } else {
    Serial.println("❌ Error al abrir el archivo. Existeix 'archivo.txt'?");
  }
}

void loop() {
  // Buit
}
