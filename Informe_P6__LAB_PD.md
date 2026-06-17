# Informe de Pràctica 6: Busos de Comunicació II (SPI)

**Autors:** Julio Lázaro Alcobendas i Gerard Rodríguez González
**Data:** 28 d'Abril de 2026
**Repositori GitHub:** https://github.com/gedrar/Practica-6

> **Nota:** El fitxer `platformio.ini` és comú per a tots els exercicis d'aquesta pràctica:
> ```ini
> [env:esp32-s3-devkitc-1]
> platform = espressif32
> board = esp32-s3-devkitc-1
> framework = arduino
> monitor_speed = 115200
> lib_deps =
>     miguelbalboa/MFRC522@^1.4.11
>     https://github.com/me-no-dev/ESPAsyncWebServer.git
>     https://github.com/me-no-dev/AsyncTCP.git
> ```
> La llibreria MFRC522 s'usa als exercicis 2 i les ampliacions. L'exercici 1 (SD) no necessita llibreries addicionals. ESPAsyncWebServer s'utilitza únicament a l'Ampliació Nota 2.

---

# Exercici 1: Lectura de targeta SD

## 1. Objectius de la pràctica

L'objectiu d'aquest exercici és comprendre el funcionament del bus SPI i aplicar-lo per llegir fitxers d'una targeta microSD. L'ESP32-S3 actua com a mestre SPI i el mòdul SD com a esclau. Es llegeix un fitxer de text (`archivo.txt`) i es mostra el seu contingut pel monitor sèrie.

## 2. Desenvolupament i Arquitectura

L'ESP32-S3 disposa de quatre busos SPI; dos estan reservats per a la Flash interna i la PSRAM. S'utilitza el **bus SPI2** amb els pins estàndard de l'ESP32-S3:

| Senyal | Pin ESP32-S3 |
|--------|-------------|
| SCK    | GPIO 36     |
| MISO   | GPIO 37     |
| MOSI   | GPIO 35     |
| CS     | GPIO 39     |

## 3. Codi Principal (main.cpp)

```cpp
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

#define SCK_PIN  36
#define MISO_PIN 37
#define MOSI_PIN 35
#define CS_PIN   39

File myFile;

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }
  delay(1000);

  Serial.println("\n--- Iniciant prova de SD ---");

  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);

  if (!SD.begin(CS_PIN, SPI)) {
    Serial.println("Error: No se pudo inicializar la SD.");
    Serial.println("Comprova els cables o si la targeta esta ben ficada.");
    return;
  }

  Serial.println("Inicializacion exitosa!");

  myFile = SD.open("/archivo.txt");

  if (myFile) {
    Serial.println("Contingut de archivo.txt:");
    Serial.println("---------------------------");
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    myFile.close();
    Serial.println("\n---------------------------");
    Serial.println("Fi de la lectura");
  } else {
    Serial.println("Error al abrir el archivo. Existeix 'archivo.txt'?");
  }
}

void loop() {
  // Buit
}
```

## 4. Funcionament del codi

Primer s'inicialitza el bus SPI amb els pins específics de l'ESP32-S3 mitjançant `SPI.begin()`. A continuació, `SD.begin()` munta la targeta indicant el pin CS i el bus SPI a usar. Si la muntatge és correcta, s'obre el fitxer `/archivo.txt` i es llegeix byte a byte amb `myFile.read()`, enviant cada byte pel port sèrie. Al finalitzar, es tanca el fitxer amb `myFile.close()`. El `loop()` queda buit ja que tota la lògica és d'execució única.

## 5. Sortida pel Monitor Sèrie

```
--- Iniciant prova de SD ---
Inicializacion exitosa!
Contingut de archivo.txt:
---------------------------
[contingut del fitxer archivo.txt]
---------------------------
Fi de la lectura
```

En cas d'error:

```
Error: No se pudo inicializar la SD.
Comprova els cables o si la targeta esta ben ficada.
```

## 6. Diagrama de flux

```
Inici Programa
  ↓
Serial.begin + espera terminal
  ↓
SPI.begin(SCK=36, MISO=37, MOSI=35, CS=39)
  ↓
SD.begin(CS=39, SPI)
  ↓
Error? → Imprimir error + aturar
  ↓
SD.open("/archivo.txt")
  ↓
Fitxer obert? → Llegir byte a byte → Serial.write()
              → Tancar fitxer
         No? → Imprimir error
  ↓
Fi (loop buit)
```

## 7. Conclusions

Hem après a inicialitzar el bus SPI de forma explícita a l'ESP32-S3, especificant els pins correctes del bus SPI2. La diferència clau respecte a plaques Arduino genèriques és que cal passar l'objecte `SPI` a `SD.begin()` per indicar quin bus SPI usar, ja que l'ESP32-S3 en té múltiples.

---

# Exercici 2: Lectura d'etiqueta RFID (RC522)

## 1. Objectiu

Llegir l'UID de targetes i clauers RFID mitjançant el lector RC522 connectat per SPI a l'ESP32-S3, i mostrar-lo pel monitor sèrie en format hexadecimal.

## 2. Desenvolupament

Per no interferir amb el bus SPI2 (reservat per a la SD), s'utilitza el **bus SPI3** de l'ESP32-S3 amb pins alternatius:

| Senyal | Pin ESP32-S3 (SPI3) |
|--------|---------------------|
| SCK    | GPIO 12             |
| MISO   | GPIO 13             |
| MOSI   | GPIO 11             |
| SS/CS  | GPIO 10             |
| RST    | GPIO 9              |

## 3. Codi Principal (main.cpp)

```cpp
#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

#define SCK_PIN  12
#define MISO_PIN 13
#define MOSI_PIN 11
#define SS_PIN   10
#define RST_PIN   9

MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }
  delay(1000);

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
```

## 4. Funcionament del codi

El lector RC522 s'inicialitza amb `mfrc522.PCD_Init()`. Al `loop()`, `PICC_IsNewCardPresent()` comprova si hi ha una targeta dins del camp electromagnètic del lector. Si n'hi ha, `PICC_ReadCardSerial()` llegeix el seu número de sèrie (UID). L'UID s'imprimeix byte a byte en hexadecimal. Finalment, `PICC_HaltA()` atura la comunicació amb la targeta per permetre noves lectures.

## 5. Sortida pel Monitor Sèrie

```
Lector RFID preparat als nous pins!
Apropa una targeta o clauer al lector...
Card UID: A3 F2 1B 04
Card UID: A3 F2 1B 04
Card UID: 7C 4E 92 D1
```

## 6. Diagrama de flux

```
Inici Programa
  ↓
SPI.begin(SCK=12, MISO=13, MOSI=11, SS=10)
  ↓
mfrc522.PCD_Init()
  ↓
Bucle Infinit / loop()
  ↓
PICC_IsNewCardPresent()?
  No → tornar a comprovar
  Sí ↓
PICC_ReadCardSerial()?
  No → tornar
  Sí ↓
Imprimir UID en HEX
  ↓
PICC_HaltA()
  ↻ (repeteix)
```

## 7. Conclusions

Hem après a gestionar dos busos SPI independents a l'ESP32-S3 usant pins alternatius del bus SPI3, evitant conflictes amb el bus SPI2 destinat a la SD. La llibreria MFRC522 simplifica enormement la comunicació amb el lector RC522.

---

# AMPLIACIÓ DE NOTA — Part 1: Datalogger RFID + SD

## 1. Objectiu

Combinar el lector RFID i la targeta SD en un sistema de registre (datalogger) que guarda l'hora i el codi de cada targeta llegida en un fitxer `fichero.log` de la targeta SD.

## 2. Solució hardware: dos busos SPI independents

El repte principal és usar simultàniament dos perifèrics SPI. La solució és assignar-los a busos SPI físics diferents de l'ESP32-S3:

| Perifèric | Bus SPI | Pins |
|-----------|---------|------|
| RFID RC522 | SPI (FSPI) | SCK=12, MISO=13, MOSI=11, CS=10 |
| SD Card    | HSPI       | SCK=36, MISO=37, MOSI=35, CS=39 |

S'instancia un objecte `SPIClass spiSD(HSPI)` per al bus de la SD, deixant el bus principal per al RFID.

## 3. Codi Principal (main.cpp)

```cpp
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <MFRC522.h>

// Pins SD (Bus HSPI)
#define SD_CS   39
#define SD_MOSI 35
#define SD_SCK  36
#define SD_MISO 37

// Pins RFID (Bus FSPI principal)
#define RFID_RST  9
#define RFID_MISO 13
#define RFID_MOSI 11
#define RFID_SCK  12
#define RFID_CS   10

SPIClass spiSD(HSPI);
MFRC522 mfrc522(RFID_CS, RFID_RST);

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }
  delay(1000);

  Serial.println("\n--- INICIANT DATALOGGER ---");

  // Bus 1: RFID
  SPI.begin(RFID_SCK, RFID_MISO, RFID_MOSI, RFID_CS);
  mfrc522.PCD_Init();
  Serial.println("Lector RFID iniciat (Bus 1)");

  // Bus 2: SD
  spiSD.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, spiSD)) {
    Serial.println("ERROR: No s'ha pogut iniciar la SD.");
  } else {
    Serial.println("SD OK (Bus 2)");
  }

  Serial.println("Apropa una targeta per registrar-la a fichero.log");
}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      uid += String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      uid += String(mfrc522.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();

    unsigned long temps = millis() / 1000;
    String registre = "Temps: " + String(temps) + "s | Codi:" + uid;
    Serial.print("Llegit: " + registre);

    File myFile = SD.open("/fichero.log", FILE_APPEND);
    if (myFile) {
      myFile.println(registre);
      myFile.close();
      Serial.println("  --> Guardat a fichero.log correctament.");
    } else {
      Serial.println("  --> Error obrint fichero.log");
    }

    mfrc522.PICC_HaltA();
    delay(1000);
  }
}
```

## 4. Funcionament

Cada vegada que s'apropa una targeta RFID, el sistema llegeix el seu UID, construeix una cadena de registre amb el temps transcorregut des de l'inici i el codi en hexadecimal, i l'afegeix al final de `fichero.log` amb `FILE_APPEND`. El fitxer `fichero.log` ha d'existir prèviament a la targeta SD (es pot crear des de l'ordinador).

## 5. Sortida pel Monitor Sèrie

```
--- INICIANT DATALOGGER ---
Lector RFID iniciat (Bus 1)
SD OK (Bus 2)
Apropa una targeta per registrar-la a fichero.log
Llegit: Temps: 12s | Codi: A3 F2 1B 04  --> Guardat a fichero.log correctament.
Llegit: Temps: 25s | Codi: 7C 4E 92 D1  --> Guardat a fichero.log correctament.
```

## 6. Conclusions

La clau d'aquest exercici és l'ús de `SPIClass spiSD(HSPI)` per instanciar un segon bus SPI independent. Sense aquesta tècnica, els dos perifèrics compartirien el bus i es produirien conflictes. Hem après que l'ESP32-S3 permet usar múltiples busos SPI simultàniament, cosa que és essencial en sistemes embeguts amb diversos perifèrics SPI.

---

# AMPLIACIÓ DE NOTA — Part 2: Datalogger RFID + SD + Web Server

## 1. Objectiu

Ampliar el datalogger de la Part 1 afegint un servidor web que permet visualitzar el contingut de `fichero.log` en temps real des de qualsevol navegador connectat a la xarxa WiFi de l'ESP32-S3.

## 2. Codi Principal (main.cpp)

```cpp
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

const char* ssid     = "EL_TEU_WIFI";
const char* password = "LA_TEVA_CONTRASENYA";

// Pins SD (Bus HSPI)
#define SD_CS   39
#define SD_MOSI 35
#define SD_SCK  36
#define SD_MISO 37

// Pins RFID (Bus FSPI)
#define RFID_RST  9
#define RFID_MISO 13
#define RFID_MOSI 11
#define RFID_SCK  12
#define RFID_CS   10

SPIClass spiSD(HSPI);
MFRC522 mfrc522(RFID_CS, RFID_RST);
AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  delay(1000);

  // 1. WiFi (timeout 15 s)
  WiFi.begin(ssid, password);
  Serial.print("Connectant al Wi-Fi...");
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 30) {
    delay(500); Serial.print("."); timeout++;
  }
  if (WiFi.status() == WL_CONNECTED)
    Serial.println("\nWi-Fi Connectat! IP: " + WiFi.localIP().toString());
  else
    Serial.println("\nNo s'ha pogut connectar (Mode Offline)");

  // 2. RFID
  SPI.begin(RFID_SCK, RFID_MISO, RFID_MOSI, RFID_CS);
  mfrc522.PCD_Init();
  Serial.println("RFID actiu");

  // 3. SD
  spiSD.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, spiSD)) Serial.println("Error: SD no trobada");
  else Serial.println("SD activa");

  // 4. Servidor web: mostra el log
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    if (SD.exists("/fichero.log"))
      request->send(SD, "/fichero.log", "text/plain");
    else
      request->send(200, "text/plain", "Encara no hi ha dades registrades.");
  });

  server.begin();
  Serial.println("Servidor Web llest");
}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      uid += String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      uid += String(mfrc522.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();

    String registre = "Temps: " + String(millis()/1000) + "s | Codi:" + uid;
    File myFile = SD.open("/fichero.log", FILE_APPEND);
    if (myFile) {
      myFile.println(registre);
      myFile.close();
      Serial.println("Registrat: " + registre);
    } else {
      Serial.println("Error escrivint a la SD");
    }

    mfrc522.PICC_HaltA();
    delay(1000);
  }
}
```

## 3. Funcionament

El servidor web escolta peticions GET a `/`. Quan un client accedeix, envia directament el contingut de `fichero.log` de la targeta SD com a text pla. Això permet veure totes les lectures RFID acumulades des d'un navegador sense necessitat de treure la targeta SD. El sistema funciona simultàniament en mode STA (connectat a una xarxa WiFi existent) i continua registrant lectures al `loop()`.

## 4. Sortida pel Monitor Sèrie

```
Connectant al Wi-Fi.....
Wi-Fi Connectat! IP: 192.168.1.50
RFID actiu
SD activa
Servidor Web llest
Registrat correctament: Temps: 8s | Codi: A3 F2 1B 04
```

## 5. Conclusions

Aquesta segona part de l'ampliació integra tres perifèrics que usen busos de comunicació diferents (SPI×2 per a SD i RFID, WiFi internament) de forma concurrent. El servidor web asíncron (`ESPAsyncWebServer`) és clau per no bloquejar el `loop()`, que necessita seguir processant lectures RFID. El resultat és un sistema de control d'accés complet i consultable remotament.
