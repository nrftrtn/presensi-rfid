#ifndef TAMPILAN_H
#define TAMPILAN_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

// =====================================================
// PIN INDIKATOR & HARDWARE
// =====================================================

#define PIN_LED_HIJAU 26
#define PIN_LED_MERAH 27
#define PIN_BUZZER    25

#define LCD_COLS      16
#define LCD_ROWS      2

// Objek LCD I2C (Alamat: 0x27, 16 kolom, 2 baris)
extern LiquidCrystal_I2C lcd;

// =====================================================
// INISIALISASI
// =====================================================

void initTampilan();

// =====================================================
// FUNGSI DASAR TAMPILAN LCD
// =====================================================

// Membersihkan seluruh layar
void bersihkanLayar();

// Merapikan teks (hilangkan newline, spasi ganda, trim)
String rapikanTeks(String teks);

// Memastikan teks tepat 16 karakter dengan spasi di akhir
String pad16(String teks);

// Menampilkan 1 baris (otomatis dipad 16 karakter agar bersih)
void tampilkanBaris(uint8_t baris, String teks);

// Menampilkan 2 baris statis
void tampilkanDuaBaris(String baris0, String baris1);

// Menampilkan teks dengan animasi teks berjalan (scrolling marquee) jika panjang > 16 karakter
void tampilkanScroll(
    String baris0,
    String baris1,
    unsigned long jedaPerLangkahMs = 240,
    unsigned long jedaAwalMs = 1200,
    unsigned long jedaAkhirMs = 1200
);

// =====================================================
// KONTROL BUZZER & LED
// =====================================================

void bunyiBuzzer(int durasiMs);
void buzzerDeteksi();
void buzzerSukses();
void buzzerTerlambat();
void buzzerGagal();

// =====================================================
// KONDISI-KONDISI TAMPILAN SISTEM PRESENSI
// =====================================================

// Kondisi 1: Layar Standby / Awal
void tampilkanStandby(bool wifiOnline, String infoTambahan = "");

// Kondisi 2: Gelang Ditempelkan (Membaca UID)
void tampilkanMembaca();

// Kondisi 3: Hasil Absensi Online (Sukses Masuk / Terlambat / Keluar / Ditolak)
void tampilkanHasilAbsensi(
    bool berhasil,
    String nama,
    String status,
    String kegiatan,
    String action,
    String pesanServer
);

// Kondisi 4: Hasil Absensi Offline Tersimpan di NVS
void tampilkanOfflineTersimpan(String jam, int totalDataOffline);

// Kondisi 5: Server Offline (Koneksi Putus Saat Scan Online) -> Disimpan ke NVS
void tampilkanServerOfflineFallback(String jam);

// Kondisi 6: Proses Sinkronisasi Data NVS
void tampilkanSyncStatus(int index, int total);

#endif
