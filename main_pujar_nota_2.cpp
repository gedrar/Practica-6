#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// --- CONFIGURACIÓ WI-FI (Canvia-ho pels teus valors) ---
// RECOMANACIÓ: Fes servir el punt d'accés del mòbil en banda 2.4GHz
const char* ssid = "EL_TEU_WIFI";
const char* password = "LA_TEVA_CONTRASENYA";

// --- PINS LECTOR SD (Bus 2 - HSPI) ---
#define SD_CS   39
#define SD_MOSI 35
#define SD_SCK  36
#define SD_MISO 37

// --- PINS LECTOR RFID (Bus 1 - FSPI) ---
#define RFID_RST  9
#define RFID_MISO 13
#define RFID_MOSI 11
#define RFID_SCK  12
#define RFID_CS   10

// Instanciem el bus per a la SD
SPIClass spiSD(HSPI); 
MFRC522 mfrc522(RFID_CS, RFID_RST);
AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n--- SISTEMA DATALOGGER WEB ---");

  // 1. Iniciem Wi-Fi (amb timeout de 15 segons per no bloquejar-se)
  WiFi.begin(ssid, password);
  Serial.print("Connectant al Wi-Fi...");
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 30) {
    delay(500);
    Serial.print(".");
    timeout++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ Wi-Fi Connectat! IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\n⚠️ No s'ha pogut connectar al Wi-Fi (Mode Offline)");
  }

  // 2. Inicialitzem Bus 1 (RFID)
  SPI.begin(RFID_SCK, RFID_MISO, RFID_MOSI, RFID_CS);
  mfrc522.PCD_Init();
  Serial.println("✅ RFID actiu");

  // 3. Inicialitzem Bus 2 (SD)
  spiSD.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, spiSD)) {
    Serial.println("❌ Error: Targeta SD no trobada");
  } else {
    Serial.println("✅ SD activa");
  }

  // 4. Configuració del Servidor Web (Ruta principal)
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    // Intentem obrir el fitxer log per mostrar-lo al navegador
    if (SD.exists("/fichero.log")) {
      request->send(SD, "/fichero.log", "text/plain");
    } else {
      request->send(200, "text/plain", "Encara no hi ha dades registrades al log.");
    }
  });

  server.begin();
  Serial.println("🌐 Servidor Web llest");
}

void loop() {
  // Verifiquem si hi ha una targeta nova
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    
    // Obtenim l'ID de la targeta
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      uid += String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      uid += String(mfrc522.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();
    
    // Timestamp bàsic (segons des de l'inici)
    String registre = "Temps: " + String(millis()/1000) + "s | Codi:" + uid;
    
    // Guardem a la SD
    File myFile = SD.open("/fichero.log", FILE_APPEND);
    if (myFile) {
      myFile.println(registre);
      myFile.close();
      Serial.println("💾 Registrat correctament: " + registre);
    } else {
      Serial.println("❌ Error escrivint a la SD");
    }

    mfrc522.PICC_HaltA();
    delay(1000); // Pausa per evitar múltiples lectures
  }
}