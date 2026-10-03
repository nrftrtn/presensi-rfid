#include "tampilan.h"
#include <Wire.h>

// =====================================================
// OBJEK LCD
// =====================================================

LiquidCrystal_I2C lcd(0x27, LCD_COLS, LCD_ROWS);

// =====================================================
// INISIALISASI
// =====================================================

void initTampilan()
{
    // Pin Output Indikator
    pinMode(PIN_LED_HIJAU, OUTPUT);
    pinMode(PIN_LED_MERAH, OUTPUT);
    pinMode(PIN_BUZZER, OUTPUT);

    digitalWrite(PIN_LED_HIJAU, LOW);
    digitalWrite(PIN_LED_MERAH, LOW);
    digitalWrite(PIN_BUZZER, LOW);

    // Inisialisasi I2C (SDA = 21, SCL = 22)
    Wire.begin(21, 22);

    // Inisialisasi Layar LCD 16x2
    lcd.init();
    lcd.backlight();
    lcd.clear();

    // Pesan Awal Booting
    tampilkanDuaBaris("Sistem Presensi", "Ubudiyah (IoT) ");
    delay(1500);
}

// =====================================================
// FUNGSI DASAR LCD
// =====================================================

void bersihkanLayar()
{
    lcd.clear();
}

String rapikanTeks(String teks)
{
    teks.trim();
    teks.replace("\r", "");
    teks.replace("\n", " ");
    teks.replace("\t", " ");

    while (teks.indexOf("  ") >= 0) {
        teks.replace("  ", " ");
    }

    return teks;
}

String pad16(String teks)
{
    while (teks.length() < 16) {
        teks += " ";
    }
    return teks;
}

void tampilkanBaris(uint8_t baris, String teks)
{
    teks = rapikanTeks(teks);
    lcd.setCursor(0, baris);

    if (teks.length() <= 16) {
        lcd.print(pad16(teks));
    } else {
        lcd.print(teks.substring(0, 16));
    }
}

void tampilkanDuaBaris(String baris0, String baris1)
{
    tampilkanBaris(0, baris0);
    tampilkanBaris(1, baris1);
}

// =====================================================
// TEKS BERJALAN (SCROLLING MARQUEE) UNTUK TEKS > 16 KARAKTER
// =====================================================

void tampilkanScroll(
    String baris0,
    String baris1,
    unsigned long jedaPerLangkahMs,
    unsigned long jedaAwalMs,
    unsigned long jedaAkhirMs
) {
    String b0 = rapikanTeks(baris0);
    String b1 = rapikanTeks(baris1);

    // Jika kedua baris pas (<= 16 karakter), tampilkan statis tanpa kedip
    if (b0.length() <= 16 && b1.length() <= 16) {
        tampilkanDuaBaris(b0, b1);
        delay(jedaAwalMs + jedaAkhirMs);
        return;
    }

    // Tambahkan 1 spasi pemisah di akhir baris yang panjang
    if (b0.length() > 16) {
        b0 += " ";
    }
    if (b1.length() > 16) {
        b1 += " ";
    }

    int max0 = (b0.length() > 16) ? (b0.length() - 16) : 0;
    int max1 = (b1.length() > 16) ? (b1.length() - 16) : 0;
    int totalLangkah = max(max0, max1);

    // Langkah Awal: Tampilkan 16 karakter pertama dan tahan sejenak agar terbaca
    tampilkanBaris(0, (b0.length() <= 16) ? b0 : b0.substring(0, 16));
    tampilkanBaris(1, (b1.length() <= 16) ? b1 : b1.substring(0, 16));
    delay(jedaAwalMs);

    // Kecepatan teks berjalan menyesuaikan panjang teks
    unsigned long delayStep = jedaPerLangkahMs;
    if (totalLangkah > 25) {
        delayStep = 180;
    } else if (totalLangkah > 15) {
        delayStep = 210;
    }

    // Animasi Pergeseran Teks Berjalan
    for (int i = 1; i <= totalLangkah; i++) {
        if (max0 > 0) {
            int off0 = (i <= max0) ? i : max0;
            lcd.setCursor(0, 0);
            lcd.print(b0.substring(off0, off0 + 16));
        }

        if (max1 > 0) {
            int off1 = (i <= max1) ? i : max1;
            lcd.setCursor(0, 1);
            lcd.print(b1.substring(off1, off1 + 16));
        }

        delay(delayStep);
    }

    // Langkah Akhir: Tahan sejenak setelah teks mencapai akhir
    delay(jedaAkhirMs);
}

// =====================================================
// KONTROL BUZZER
// =====================================================

void bunyiBuzzer(int durasiMs)
{
    digitalWrite(PIN_BUZZER, HIGH);
    delay(durasiMs);
    digitalWrite(PIN_BUZZER, LOW);
}

void buzzerDeteksi()
{
    // Beep klik pendek saat kartu RFID menempel
    bunyiBuzzer(50);
}

void buzzerSukses()
{
    // Pola nada sukses (2x nada pendek ceria)
    bunyiBuzzer(90);
    delay(60);
    bunyiBuzzer(90);
}

void buzzerTerlambat()
{
    // Pola peringatan terlambat (3x nada cepat)
    bunyiBuzzer(80);
    delay(50);
    bunyiBuzzer(80);
    delay(50);
    bunyiBuzzer(80);
}

void buzzerGagal()
{
    // Pola ditolak (1x nada panjang tegas)
    bunyiBuzzer(650);
}

// =====================================================
// IMPLEMENTASI KONDISI-KONDISI TAMPILAN
// =====================================================

// Kondisi 1: Standby
void tampilkanStandby(bool wifiOnline, String infoTambahan)
{
    String baris0 = "Tempelkan Gelang";
    String baris1 = "";

    if (wifiOnline) {
        if (infoTambahan.length() > 0) {
            // Tampilkan IP lokal
            baris1 = infoTambahan;
        } else {
            baris1 = "[ONLINE]";
        }
    } else {
        baris1 = "*MODE OFFLINE*  ";
    }

    tampilkanDuaBaris(baris0, baris1);
}

// Kondisi 2: Gelang Terdeteksi
void tampilkanMembaca()
{
    buzzerDeteksi();
    tampilkanDuaBaris("Membaca Gelang..", "Mohon Tunggu... ");
}

// Kondisi 3: Hasil Absensi Online
void tampilkanHasilAbsensi(
    bool berhasil,
    String nama,
    String status,
    String kegiatan,
    String action,
    String pesanServer
) {
    if (berhasil) {
        // -------------------------------------------------
        // SKENARIO 1: PRESENSI MASUK / KELUAR BERHASIL
        // -------------------------------------------------

        digitalWrite(PIN_LED_MERAH, LOW);
        digitalWrite(PIN_LED_HIJAU, HIGH);

        String baris0 = (nama.length() > 0) ? nama : "Santri Terdaftar";
        String baris1 = "";

        if (action == "keluar") {
            buzzerSukses();
            baris1 = "[KELUAR] " + (kegiatan.length() > 0 ? kegiatan : "Presensi");
        } else if (status.equalsIgnoreCase("Terlambat")) {
            buzzerTerlambat();
            // Kedua LED menyala untuk penanda terlambat
            digitalWrite(PIN_LED_MERAH, HIGH);
            baris1 = "[TERLAMBAT] " + (kegiatan.length() > 0 ? kegiatan : "");
        } else {
            // Hadir Tepat Waktu
            buzzerSukses();
            baris1 = "[HADIR] " + (kegiatan.length() > 0 ? kegiatan : "Tepat Waktu");
        }

        // Tampilkan dengan auto-scroll jika nama atau kegiatan melebihi 16 huruf
        tampilkanScroll(baris0, baris1);

        digitalWrite(PIN_LED_HIJAU, LOW);
        digitalWrite(PIN_LED_MERAH, LOW);

    } else {
        // -------------------------------------------------
        // SKENARIO 2: ABSENSI DITOLAK
        // -------------------------------------------------

        digitalWrite(PIN_LED_HIJAU, LOW);
        digitalWrite(PIN_LED_MERAH, HIGH);

        String b0 = "";
        String b1 = "";

        String pesanLower = pesanServer;
        pesanLower.toLowerCase();

        if (action == "selesai" || pesanLower.indexOf("sudah") >= 0) {
            // Santri sudah melakukan presensi pada jadwal ini
            buzzerTerlambat();
            b0 = (nama.length() > 0) ? nama : "Sudah Presensi!";
            b1 = "Sudah Presensi! ";
        } else if (pesanLower.indexOf("tidak terdaftar") >= 0 || pesanLower.indexOf("tidak aktif") >= 0) {
            // Gelang RFID belum diinput di data santri
            buzzerGagal();
            b0 = "RFID Tdk Dikenal";
            b1 = (pesanServer.length() > 0) ? pesanServer : "Kartu Blm Terdaftar";
        } else if (pesanLower.indexOf("tidak ada kegiatan") >= 0 || pesanLower.indexOf("belum dibuka") >= 0 || pesanLower.indexOf("telah ditutup") >= 0) {
            // Di luar jadwal waktu shalat
            buzzerGagal();
            b0 = "Absensi Ditolak!";
            b1 = (pesanServer.length() > 0) ? pesanServer : "Belum Ada Jadwal";
        } else {
            // Kesalahan lain
            buzzerGagal();
            b0 = "Absensi Gagal!";
            b1 = (pesanServer.length() > 0) ? pesanServer : "Silakan Coba Lagi";
        }

        tampilkanScroll(b0, b1);

        digitalWrite(PIN_LED_MERAH, LOW);
    }
}

// Kondisi 4: Hasil Absensi Offline Tersimpan di NVS
void tampilkanOfflineTersimpan(String jam, int totalDataOffline)
{
    digitalWrite(PIN_LED_MERAH, LOW);
    digitalWrite(PIN_LED_HIJAU, HIGH);

    buzzerSukses();

    String b0 = "Simpan Offline  ";
    String b1 = "Jam " + jam + " (Aman)";

    tampilkanScroll(b0, b1, 240, 1500, 1200);

    digitalWrite(PIN_LED_HIJAU, LOW);
}

// Kondisi 5: Server Offline Fallback (Auto-save ke NVS saat koneksi server gagal)
void tampilkanServerOfflineFallback(String jam)
{
    digitalWrite(PIN_LED_MERAH, LOW);
    digitalWrite(PIN_LED_HIJAU, HIGH);

    buzzerTerlambat();

    String b0 = "Server Offline  ";
    String b1 = "Disimpan ke NVS ";

    tampilkanScroll(b0, b1, 240, 1500, 1200);

    digitalWrite(PIN_LED_HIJAU, LOW);
}

// Kondisi 6: Status Sinkronisasi NVS
void tampilkanSyncStatus(int index, int total)
{
    String b0 = "Sinkronisasi... ";
    String b1 = "Data " + String(index) + " dari " + String(total);
    tampilkanDuaBaris(b0, b1);
}
