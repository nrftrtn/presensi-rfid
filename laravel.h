#ifndef LARAVEL_H
#define LARAVEL_H

#include <Arduino.h>

// =====================================================
// HASIL ABSENSI
// =====================================================

struct HasilAbsensi
{
    bool berhasil;

    String nama;
    String kegiatan;
    String status;
    String jam;
    String message;
};

// =====================================================
// ABSENSI ONLINE
// =====================================================

// Mengirim UID RFID ke Laravel.
// Laravel akan menentukan tanggal, jam,
// kegiatan, dan status kehadiran.

HasilAbsensi kirimAbsensiKeLaravel(String uid);

// =====================================================
// SINKRONISASI DATA OFFLINE
// =====================================================

// Mengirim data yang sebelumnya tersimpan
// di NVS ketika ESP32 sedang offline.
//
// uid     = UID gelang
// tanggal = tanggal saat scan offline
// jam     = jam saat scan offline

bool kirimDataOfflineKeLaravel(`
    String uid,
    String tanggal,
    String jam
);

#endif