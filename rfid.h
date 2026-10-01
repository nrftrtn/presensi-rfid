#ifndef RFID_H
#define RFID_H

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

// =====================================================
// PIN RC522
// =====================================================

#define RFID_SS_PIN  5
#define RFID_RST_PIN 4

#define RFID_SCK     18
#define RFID_MISO    19
#define RFID_MOSI    23

// =====================================================
// FUNGSI RFID
// =====================================================

// Inisialisasi RC522
void initRFID();

// Mengecek apakah RC522 terdeteksi
bool cekRFID();

// Mengecek apakah ada gelang/kartu baru
bool adaKartuRFID();

// Membaca UID gelang
String readRFID();

// Menghentikan komunikasi dengan kartu
void selesaiRFID();

#endif