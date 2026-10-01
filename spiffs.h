#ifndef SPIFFS_H
#define SPIFFS_H

#include <Arduino.h>

// =====================================================
// INISIALISASI PENYIMPANAN OFFLINE
// =====================================================

void inisialisasiPenyimpanan();


// =====================================================
// SIMPAN DATA ABSENSI OFFLINE
// =====================================================

// Menyimpan:
// UID RFID
// Tanggal scan
// Jam scan

void simpanDataOffline(
    String uid,
    String tanggal,
    String jam
);


// =====================================================
// JUMLAH DATA OFFLINE
// =====================================================

int jumlahDataOffline();


// =====================================================
// AMBIL DATA OFFLINE
// =====================================================

// Mengambil data berdasarkan index.
//
// Format hasil:
// UID|tanggal|jam
//
// Contoh:
// AABBCCDD|2026-08-08|13:04:12

String ambilDataOffline(int index);


// =====================================================
// HAPUS DATA OFFLINE
// =====================================================

void hapusDataOffline(int index);


// =====================================================
// HAPUS SEMUA DATA OFFLINE
// =====================================================

void hapusSemuaDataOffline();

#endif