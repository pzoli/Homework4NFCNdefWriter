#include <Arduino.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <Wire.h>
#include <NfcAdapter.h>

PN532_I2C pn532_i2c(Wire);
PN532 nfcDriver(pn532_i2c);
NfcAdapter nfcAdapter = NfcAdapter(pn532_i2c);

uint8_t ndefKey[] = { 0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7 };
uint8_t factoryKey[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
uint8_t factoryKey2[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t keyMad[] = { 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5 };

void writeNDefTag() {
   NdefMessage message = NdefMessage();
   message.addUriRecord("https://infokristaly.hu");

   bool success = nfcAdapter.write(message);
    
    if (success) {
        Serial.println("SIKER! Az adat felírva.");
    } else {
        Serial.println("HIBA! Nem sikerült írni (talán zárolt kártya?).");
    }
}

void readTag() {
    NfcTag tag = nfcAdapter.read();
    Serial.println("Kártya észlelve!");
    tag.print();
}

void formatNfcTag() {
    Serial.println("Kártya észlelve, formázás folyamatban...");
    bool success = nfcAdapter.format();
    if (success) {
        Serial.println("SIKER! A kártya formázva lett.");
    } else {
        Serial.println("HIBA! Nem sikerült formázni a kártyát.");
    }
}

void eraseNfcTag() {
    Serial.println("Kártya észlelve, törlés folyamatban...");
    bool success = nfcAdapter.erase();
    if (success) {
        Serial.println("SIKER! A kártya törölve lett.");
    } else {
        Serial.println("HIBA! Nem sikerült törölni a kártyát.");
    }
}

void setup() {
  Serial.begin(115200);
  while (!Serial){}
  Serial.println("Wire inicializálása és PN532 kezdése...");
  Wire.begin();

  Serial.println("Wire inicializálva.");
  Serial.println("PN532 inicializálása...");
  nfcDriver.begin();
  Serial.println("PN532 inicializálva.");
  nfcDriver.SAMConfig();
  nfcDriver.setPassiveActivationRetries(0xFF);
}

char cmd;

void loop() {
    if (Serial.available() > 0) {
        cmd = Serial.read();
        Serial.print("Parancs beolvasva: ");
        Serial.println(cmd);      
    }
    if (nfcAdapter.tagPresent()) {
                switch (cmd) {
            case 'f':
                Serial.println("Formázás indítása...");
                formatNfcTag();
                break;
            case 'w':
                Serial.println("Írás indítása...");
                writeNDefTag();
                break;
            case 'e':
                Serial.println("Törlés indítása...");
                eraseNfcTag();
                break;
            case 'r':
                Serial.println("Olvasás indítása...");
                readTag();
                break;
            default:
                Serial.println("Olvasás indítása...");
                readTag();
                break;
        }
        
    }

    delay(3000);
}
