#include "wifi_module.h"
#include "rfid.h"
#include "laravel.h"
#include "spiffs.h"
#include "sync.h"
#include "waktu.h"

#include <Wire.h>
#include <LiquidCrystal_I2C.h>


// =====================================================
// PIN INDIKATOR
// =====================================================

#define LED_HIJAU 26
#define LED_MERAH 27
#define BUZZER    25


// =====================================================
// LCD
// =====================================================

LiquidCrystal_I2C lcd(0x27, 16, 2);


// =====================================================
// FUNGSI LCD - TAMPILAN AWAL
// =====================================================

void tampilkanAwal()
{
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Tempel Gelang");

    lcd.setCursor(0, 1);
    if (isWiFiConnected())
    {
        lcd.print(getLocalIPString());
    }
    else
    {
        lcd.print("RFID (OFFLINE)");
    }
}


// =====================================================
// BUZZER
// =====================================================

void bunyiBuzzer(int durasi)
{
    digitalWrite(BUZZER, HIGH);

    delay(durasi);

    digitalWrite(BUZZER, LOW);
}


// =====================================================
// INDIKATOR BERHASIL
// =====================================================

void indikatorBerhasil()
{
    digitalWrite(LED_MERAH, LOW);

    digitalWrite(LED_HIJAU, HIGH);

    bunyiBuzzer(250);

    delay(1500);

    digitalWrite(LED_HIJAU, LOW);
}


// =====================================================
// INDIKATOR GAGAL
// =====================================================

void indikatorGagal()
{
    digitalWrite(LED_HIJAU, LOW);

    digitalWrite(LED_MERAH, HIGH);

    bunyiBuzzer(800);

    delay(1500);

    digitalWrite(LED_MERAH, LOW);
}


// =====================================================
// SETUP
// =====================================================

void setup()
{
    Serial.begin(115200);

    delay(1000);


    // =================================================
    // PIN OUTPUT
    // =================================================

    pinMode(LED_HIJAU, OUTPUT);
    pinMode(LED_MERAH, OUTPUT);
    pinMode(BUZZER, OUTPUT);

    digitalWrite(LED_HIJAU, LOW);
    digitalWrite(LED_MERAH, LOW);
    digitalWrite(BUZZER, LOW);


    // =================================================
    // LCD
    // =================================================

    Wire.begin(21, 22);

    lcd.init();
    lcd.backlight();

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Sistem Presensi");

    lcd.setCursor(0, 1);
    lcd.print("Ubudiyah");

    delay(2000);


    // =================================================
    // SERIAL
    // =================================================

    Serial.println();
    Serial.println("====================================");
    Serial.println("     SISTEM PRESENSI UBUDIYAH");
    Serial.println("          RFID + ESP32");
    Serial.println("       ONLINE - OFFLINE");
    Serial.println("====================================");


    // =================================================
    // NVS
    // =================================================

    inisialisasiPenyimpanan();


    // =================================================
    // WIFI
    // =================================================

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Menghubungkan");

    lcd.setCursor(0, 1);
    lcd.print("WiFi...");

    initWiFi();


    // =================================================
    // WAKTU
    // =================================================

    initWaktu();


    // =================================================
    // RFID
    // =================================================

    initRFID();


    // =================================================
    // SYNC
    // =================================================

    initSync();


    // =================================================
    // DATA OFFLINE
    // =================================================

    int jumlahOffline =
        jumlahDataOffline();

    Serial.println();

    Serial.print(
        "Data offline tersimpan : "
    );

    Serial.println(jumlahOffline);


    // =================================================
    // JIKA ONLINE
    // =================================================

    if (isWiFiConnected())
    {
        Serial.println();
        Serial.println(
            "STATUS SISTEM : ONLINE"
        );


        if (jumlahOffline > 0)
        {
            Serial.println(
                "Memulai sinkronisasi..."
            );

            syncOfflineData();
        }
    }


    // =================================================
    // JIKA OFFLINE
    // =================================================

    else
    {
        Serial.println();
        Serial.println(
            "STATUS SISTEM : OFFLINE"
        );

        Serial.println(
            "Data baru akan disimpan di NVS."
        );
    }


    // =================================================
    // SISTEM SIAP
    // =================================================

    tampilkanAwal();

    Serial.println();
    Serial.println(
        "===================================="
    );

    Serial.println(
        "      SISTEM SIAP DIGUNAKAN"
    );

    Serial.println(
        "      TEMPELKAN GELANG RFID"
    );

    Serial.println(
        "===================================="
    );
}


// =====================================================
// LOOP
// =====================================================

void loop()
{
    // =================================================
    // WEB SERVER PORTAL
    // =================================================

    handlePortalClient();


    // =================================================
    // CEK WIFI
    // =================================================

    checkWiFi();


    // =================================================
    // JIKA WIFI TERHUBUNG
    // COBA SINKRONISASI DATA OFFLINE
    // =================================================

    if (isWiFiConnected())
    {
        if (jumlahDataOffline() > 0)
        {
            syncOfflineData();
        }
    }


    // =================================================
    // BACA RFID
    // =================================================

    String uid = readRFID();


    // =================================================
    // TIDAK ADA GELANG
    // =================================================

    if (uid == "")
    {
        delay(100);
        return;
    }


    // =================================================
    // RFID TERBACA
    // =================================================

    Serial.println();
    Serial.println(
        "===================================="
    );

    Serial.println(
        "       GELANG RFID TERBACA"
    );

    Serial.print(
        "UID : "
    );

    Serial.println(uid);

    Serial.println(
        "===================================="
    );


    // =================================================
    // TAMPILKAN MEMPROSES
    // =================================================

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Memproses...");

    lcd.setCursor(0, 1);
    lcd.print("Mohon Tunggu");


    // =================================================
    // MODE ONLINE
    // =================================================

    if (isWiFiConnected())
    {
        Serial.println(
            "MODE : ONLINE"
        );

        Serial.println(
            "Mengirim ke Laravel..."
        );


        // ---------------------------------------------
        // Kirim UID ke Laravel
        // ---------------------------------------------

        HasilAbsensi hasil =
            kirimAbsensiKeLaravel(uid);


        // =============================================
        // ABSENSI BERHASIL
        // =============================================

        if (hasil.berhasil)
        {
            Serial.println(
                "[OK] Absensi berhasil."
            );

            Serial.println(
                "Nama      : " + hasil.nama
            );

            Serial.println(
                "Kegiatan  : " + hasil.kegiatan
            );

            Serial.println(
                "Status    : " + hasil.status
            );

            Serial.println(
                "Jam       : " + hasil.jam
            );


            // -----------------------------------------
            // LCD
            // -----------------------------------------

            lcd.clear();

            lcd.setCursor(0, 0);

            lcd.print(
                hasil.nama.substring(0, 16)
            );

            lcd.setCursor(0, 1);

            lcd.print(
                hasil.status.substring(0, 16)
            );


            // -----------------------------------------
            // LED HIJAU + BUZZER
            // -----------------------------------------

            indikatorBerhasil();

            delay(2000);
        }


        // =============================================
        // ABSENSI DITOLAK
        // =============================================

        else
        {
            Serial.println(
                "[GAGAL] Absensi ditolak."
            );

            Serial.println(
                "Pesan : " + hasil.message
            );


            lcd.clear();

            lcd.setCursor(0, 0);
            lcd.print("Absensi Gagal");

            lcd.setCursor(0, 1);

            lcd.print(
                hasil.message.substring(0, 16)
            );


            indikatorGagal();

            delay(2000);
        }
    }


    // =================================================
    // MODE OFFLINE
    // =================================================

    else
    {
        Serial.println(
            "MODE : OFFLINE"
        );

        Serial.println(
            "Menyimpan ke NVS..."
        );


        // ---------------------------------------------
        // AMBIL TANGGAL DAN JAM SAAT SCAN
        // ---------------------------------------------

        String tanggal =
            getTanggal();

        String jam =
            getJam();


        Serial.print(
            "Tanggal : "
        );

        Serial.println(tanggal);

        Serial.print(
            "Jam     : "
        );

        Serial.println(jam);


        // ---------------------------------------------
        // SIMPAN UID + TANGGAL + JAM
        // ---------------------------------------------

        simpanDataOffline(
            uid,
            tanggal,
            jam
        );


        // ---------------------------------------------
        // LCD
        // ---------------------------------------------

        lcd.clear();

        lcd.setCursor(0, 0);
        lcd.print("Tersimpan Offline");

        lcd.setCursor(0, 1);
        lcd.print("Data Aman");


        // ---------------------------------------------
        // INDIKATOR
        // ---------------------------------------------

        indikatorBerhasil();

        delay(2000);
    }


    // =================================================
    // KEMBALI KE TAMPILAN AWAL
    // =================================================

    tampilkanAwal();


    // =================================================
    // TUNGGU
    // =================================================

    delay(1000);
}