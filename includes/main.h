//
// Created by Simeon Kolev on 16/02/2023.
//

#ifndef APP1_MAIN_H
#define APP1_MAIN_H

#include "mgos.h"
#include "mgos_timers.h"
//#include "mgos_mongoose.h"

#include <SPI.h>
#include "MFRC522.h"

void logFlashString(const __FlashStringHelper *str);

void printHex(byte *buffer, byte bufferSize);

void printDec(byte *buffer, byte bufferSize);

static void my_timer_cb(void *arg);

static void turnOnRelay(void);

enum mgos_app_init_result mgos_app_init(void);

#endif //APP1_MAIN_H
