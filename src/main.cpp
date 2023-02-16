//
// Created by Simeon Kolev on 16/02/2023.
//

#include "../includes/main.h"

#include "mgos.h"

#include <SPI.h>
#include "../includes/MFRC522.h"

#define SS_PIN 10 // TODO: Which pin?
#define RST_PIN 9 // TODO: Which pin?

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key;

// Init array that will store new NUID
byte nuidPICC[4];

enum mgos_app_init_result mgos_app_init(void) {
    SPI.begin(); // Init SPI bus
    rfid.PCD_Init(); // Init MFRC522

    for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
    }

//    Serial.println(F("This code scan the MIFARE Classsic NUID."));
//    Serial.print(F("Using the following key:"));
//    printHex(key.keyByte, MFRC522::MF_KEY_SIZE);

    return MGOS_APP_INIT_SUCCESS;
}
