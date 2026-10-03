#include "wifi_module.h"
#include "rfid.h"
#include "laravel.h"
#include "spiffs.h"
#include "sync.h"
#include "waktu.h"
#include "tampilan.h"

// =====================================================
// SETUP
// =====================================================

void setup()
{
    Serial.begin(115200);
    delay(500);

    // =================================================
    // INISIALISASI TAMPILAN & INDIKATOR (LCD 16x2)
    // =================================================

    initTampilan();

    Serial.println();
    Serial.println("====================================");
    Serial.println("     SISTEM PRESENSI UBUDIYAH");
    Serial.println("          RFID + ESP32");
    Serial.println("       ONLINE - OFFLINE");
    Serial.println("====================================");

    // =================================================
    // PENYIMPANAN NVS
    // =================================================

    inisialisasiPenyimpanan();

    // =================================================
    // KONEKSI WIFI
    // =================================================

    tampilkanDuaBaris("Menghubungkan...", "WiFi...");

    initWiFi();

    // =================================================
    // WAKTU NTP / RTC
    // =================================================

    initWaktu();

    // =================================================
    // PEMBACA RFID RC522
    // =================================================

    initRFID();

    // =================================================
    // MODUL SYNC OFFLINE
    // =================================================

    initSync();

    // =================================================
    // CEK DATA OFFLINE TERSIMPAN
    // =================================================

    int jumlahOffline = jumlahDataOffline();

    Serial.println();
    Serial.print("Data offline tersimpan: ");
    Serial.println(jumlahOffline);

    if (isWiFiConnected())
    {
        Serial.println("STATUS SISTEM : ONLINE");

        if (jumlahOffline > 0)
        {
            Serial.println("Memulai sinkronisasi data NVS...");
            tampilkanSyncStatus(1, jumlahOffline);
            syncOfflineData();
        }
    }
    else
    {
        Serial.println("STATUS SISTEM : OFFLINE");
        Serial.println("Data scan baru akan disimpan di memori NVS.");
    }

    // =================================================
    // SISTEM SIAP DIGUNAKAN
    // =================================================

    tampilkanStandby(isWiFiConnected(), getLocalIPString());

    Serial.println();
    Serial.println("====================================");
    Serial.println("      SISTEM SIAP DIGUNAKAN");
    Serial.println("      TEMPELKAN GELANG RFID");
    Serial.println("====================================");
}


// =====================================================
// LOOP UTAMA
// =====================================================

void loop()
{
    // =================================================
    // PORTAL SETUP WIFI & WEB SERVER
    // =================================================

    handlePortalClient();

    // =================================================
    // CEK DAN REKONEKSI WIFI
    // =================================================

    checkWiFi();

    // =================================================
    // JIKA ONLINE: SINKRONISASI DATA OFFLINE OTOMATIS
    // =================================================

    static unsigned long lastSyncCheck = 0;
    if (isWiFiConnected() && (millis() - lastSyncCheck > 15000))
    {
        lastSyncCheck = millis();
        if (jumlahDataOffline() > 0)
        {
            syncOfflineData();
        }
    }

    // =================================================
    // BACA UID KARTU / GELANG RFID
    // =================================================

    String uid = readRFID();

    // Tidak ada gelang yang menempel
    if (uid == "")
    {
        delay(80);
        return;
    }

    // =================================================
    // RFID TERBACA
    // =================================================

    Serial.println();
    Serial.println("====================================");
    Serial.println("       GELANG RFID TERBACA");
    Serial.print  ("UID : ");
    Serial.println(uid);
    Serial.println("====================================");

    // Tampilkan pesan membaca sejenak (feedback visual & audio klik)
    tampilkanMembaca();

    // =================================================
    // SKENARIO A: MODE ONLINE (WIFI TERHUBUNG)
    // =================================================

    if (isWiFiConnected())
    {
        Serial.println("MODE : ONLINE - Mengirim ke Laravel...");

        HasilAbsensi hasil = kirimAbsensiKeLaravel(uid);

        // Kasus Khusus: Jika server Laravel tidak merespon (Down / Timeout),
        // amankan data presensi santri ke NVS agar tidak hilang!
        if (hasil.message == "Server tidak terhubung" || hasil.message == "Response server tidak valid")
        {
            Serial.println("Peringatan: Server tidak merespon. Menyimpan ke NVS...");

            String tanggal = getTanggal();
            String jam     = getJam();

            simpanDataOffline(uid, tanggal, jam);

            tampilkanServerOfflineFallback(jam);
        }
        else
        {
            // Tampilkan hasil absensi sesuai kondisi (Hadir / Terlambat / Keluar / Ditolak)
            // Mendukung teks berjalan (marquee) jika nama atau pesan > 16 karakter
            tampilkanHasilAbsensi(
                hasil.berhasil,
                hasil.nama,
                hasil.status,
                hasil.kegiatan,
                hasil.action,
                hasil.message
            );
        }
    }

    // =================================================
    // SKENARIO B: MODE OFFLINE (TIDAK ADA WIFI)
    // =================================================

    else
    {
        Serial.println("MODE : OFFLINE - Menyimpan ke NVS...");

        String tanggal = getTanggal();
        String jam     = getJam();

        Serial.print("Tanggal : ");
        Serial.println(tanggal);
        Serial.print("Jam     : ");
        Serial.println(jam);

        simpanDataOffline(uid, tanggal, jam);

        int totalOffline = jumlahDataOffline();

        // Tampilkan notifikasi offline dengan rapi di LCD
        tampilkanOfflineTersimpan(jam, totalOffline);
    }

    // =================================================
    // KEMBALI KE TAMPILAN STANDBY
    // =================================================

    tampilkanStandby(isWiFiConnected(), getLocalIPString());

    // Jeda sejenak sebelum pembacaan berikutnya
    delay(500);
}