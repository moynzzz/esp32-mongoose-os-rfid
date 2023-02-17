#include "../includes/main.h"

#include "mgos.h"
#include "mgos_timers.h"
#include "mgos_http_client.h"

#include <SPI.h>
#include "../includes/MFRC522.h"

#define SS_PIN 5 // ESP32 pin GIOP5
#define RST_PIN 27 // ESP32 pin GIOP27

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key;

// Init array that will store new NUID
byte nuidPICC[4];

void logFlashString(const __FlashStringHelper *str) {
    char buf[strlen_P((PGM_P)str) + 1];
    strcpy_P(buf, (PGM_P)str);
    LOG(LL_INFO, ("%.*s", strlen(buf), buf));
}

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void printHex(byte *buffer, byte bufferSize) {
    // Convert byte buffer to hexadecimal string
    char hexString[bufferSize * 3 + 1];

    for (int i = 0; i < bufferSize; i++) {
        sprintf(&hexString[i * 2], "%02x ", buffer[i]);
    }

    hexString[bufferSize * 3] = '\0';

    // Print hexadecimal string using LOG function
    LOG(LL_INFO, ("%s", hexString));
}

/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte *buffer, byte bufferSize) {
    // Convert byte buffer to decimal string
    char decimalString[bufferSize * 4 + 1];

    for (int i = 0; i < bufferSize; i++) {
        sprintf(&decimalString[i * 2], "%d ", buffer[i]);
    }

    decimalString[bufferSize * 3] = '\0';

    // Print hexadecimal string using LOG function
    LOG(LL_INFO, ("%s", decimalString));
}

static void http_cb(struct mg_connection *c, int ev, void *p, void *user_data) {
    struct http_message *hm = (struct http_message *) p;

    switch (ev) {
        case MG_EV_HTTP_REPLY:
            LOG(LL_INFO, ("HTTP reply status: %.*s", hm->resp_status_msg.len, hm->resp_status_msg.p));
            LOG(LL_INFO, ("HTTP reply content: %.*s", hm->body.len, hm->body.p));
            break;
        case MG_EV_CLOSE:
            *(int *) user_data = 1;
            break;
    }
}

void sendHttpRequest() {
    struct mg_connection *nc = mg_connect_http("http://192.168.0.67/rpc/Switch.Set?id=0&on=true", "80", http_cb, &nc, NULL);

    if (nc == NULL) {
        LOG(LL_ERROR, ("Failed to connect to server"));
        return;
    }

    char *message = "Hello, server!";
    mg_printf(nc, "GET /post HTTP/1.0\r\nContent-Length: %d\r\n\r\n%s", strlen(message), message);
}

static void my_timer_cb(void *arg) {
    // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
    if (!rfid.PICC_IsNewCardPresent()) {
        (void) arg;

        return;
    }

    if (!rfid.PICC_ReadCardSerial()) {
        (void) arg;

        return;
    }

    LOG(LL_INFO, ("PICC type: "));
    MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
    logFlashString(rfid.PICC_GetTypeName(piccType));

    // Check is the PICC of Classic MIFARE type
    if (
        piccType != MFRC522::PICC_TYPE_MIFARE_MINI
        && piccType != MFRC522::PICC_TYPE_MIFARE_1K
        && piccType != MFRC522::PICC_TYPE_MIFARE_4K
    ) {
        LOG(LL_INFO, ("Your tag is not of type MIFARE Classic."));

        (void) arg;

        return;
    }

    if (
        rfid.uid.uidByte[0] != nuidPICC[0]
        || rfid.uid.uidByte[1] != nuidPICC[1]
        || rfid.uid.uidByte[2] != nuidPICC[2]
        || rfid.uid.uidByte[3] != nuidPICC[3]
    ) {
        LOG(LL_INFO, ("A new card has been detected."));

        // Store NUID into nuidPICC array
        for (byte i = 0; i < 4; i++) {
            nuidPICC[i] = rfid.uid.uidByte[i];
        }

        LOG(LL_INFO, ("The NUID tag is:"));
        LOG(LL_INFO, ("In hex: "));
        printHex(rfid.uid.uidByte, rfid.uid.size);

        LOG(LL_INFO, ("In dec: "));
        printDec(rfid.uid.uidByte, rfid.uid.size);
    } else {
        LOG(LL_INFO, ("Card read previously."));
    }

    // Halt PICC
    rfid.PICC_HaltA();

    // Stop encryption on PCD
    rfid.PCD_StopCrypto1();

    (void) arg;
}

enum mgos_app_init_result mgos_app_init(void) {
    SPI.begin(); // Init SPI bus
    rfid.PCD_Init(); // Init MFRC522
    rfid.PCD_SetAntennaGain(MFRC522::RxGain_max);

    for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
    }

    LOG(LL_INFO, ("This code scan the MIFARE Classsic NUID."));
    LOG(LL_INFO, ("Using the following key:"));

    printHex(key.keyByte, MFRC522::MF_KEY_SIZE);

    mgos_set_timer(1, MGOS_TIMER_REPEAT, my_timer_cb, NULL);

    return MGOS_APP_INIT_SUCCESS;
}