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

// Segédfüggvény, ami minden alkalommal újra észleli a kártyát
bool tryAuthAndWrite(int block, uint8_t* key, uint8_t* data) {
    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
    uint8_t uidLen;

    // Kényszerített újra-észlelés (Ez az MCT titka)
    if (!nfcDriver.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLen, 100)) return false;

    // Próba Key A-val
    if (nfcDriver.mifareclassic_AuthenticateBlock(uid, uidLen, block, 0, key)) {
        if (nfcDriver.mifareclassic_WriteDataBlock(block, data)) return true;
    }

    // Újra-észlelés a Key B próbához
    if (!nfcDriver.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLen, 100)) return false;
    
    // Próba Key B-vel
    if (nfcDriver.mifareclassic_AuthenticateBlock(uid, uidLen, block, 1, key)) {
        if (nfcDriver.mifareclassic_WriteDataBlock(block, data)) return true;
    }

    return false;
}

void factoryResetTag() {
    uint8_t factoryTrailer[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0x80, 0x69, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    
    // Csak a két legvalószínűbb kulcs (Gyári és NDEF)
    uint8_t k1[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t k2[] = {0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7};

    for (int sector = 0; sector < 16; sector++) {
        int trailerBlock = (sector * 4) + 3;
        bool success = false;

        // PRÓBA 1: Gyári Key A és B
        if (tryAuthAndWrite(trailerBlock, k1, factoryTrailer)) success = true;
        
        // PRÓBA 2: NDEF Key A és B (ha az első nem ment)
        if (!success && tryAuthAndWrite(trailerBlock, k2, factoryTrailer)) success = true;

        Serial.print(F("Szektor reset "));
        Serial.print(sector);
        Serial.println(success ? F(" Siker") : F(" Hiba"));
        if (sector % 4 == 3) Serial.println();
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
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            return;
        }
        cmd = c;
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
            case 'x':
                Serial.println("Factory Reset indítása...");
                factoryResetTag();
                break;
            default:
                Serial.println("Olvasás indítása...");
                readTag();
                break;
        }
        
    }

    delay(3000);
}
