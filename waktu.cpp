#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include "waktu.h"

// =====================================================
// TIMEZONE INDONESIA
// =====================================================

const long GMT_OFFSET_SEC = 7 * 60 * 60;

const int DAYLIGHT_OFFSET_SEC = 0;


// =====================================================
// INISIALISASI WAKTU
// =====================================================

void initWaktu()
{
    Serial.println();
    Serial.println("================================");
    Serial.println("       INISIALISASI WAKTU");
    Serial.println("================================");


    // -------------------------------------------------
    // Konfigurasi waktu Indonesia
    // -------------------------------------------------

    configTime(
        GMT_OFFSET_SEC,
        DAYLIGHT_OFFSET_SEC,
        "pool.ntp.org",
        "time.nist.gov"
    );


    // -------------------------------------------------
    // Jika WiFi tersedia, ambil waktu NTP
    // -------------------------------------------------

    if (WiFi.status() == WL_CONNECTED)
    {
        struct tm timeinfo;

        if (
            getLocalTime(
                &timeinfo,
                10000
            )
        )
        {
            Serial.println(
                "Waktu berhasil disinkronkan."
            );

            Serial.println(
                getTanggal() +
                " " +
                getJam()
            );
        }
        else
        {
            Serial.println(
                "Gagal mendapatkan waktu NTP."
            );
        }
    }
    else
    {
        Serial.println(
            "WiFi belum terhubung."
        );

        Serial.println(
            "Waktu akan disinkronkan "
            "saat WiFi tersedia."
        );
    }
}


// =====================================================
// TANGGAL
// =====================================================

String getTanggal()
{
    struct tm timeinfo;

    if (
        !getLocalTime(
            &timeinfo,
            1000
        )
    )
    {
        return "0000-00-00";
    }


    char tanggal[11];

    strftime(
        tanggal,
        sizeof(tanggal),
        "%Y-%m-%d",
        &timeinfo
    );


    return String(tanggal);
}


// =====================================================
// JAM
// =====================================================

String getJam()
{
    struct tm timeinfo;

    if (
        !getLocalTime(
            &timeinfo,
            1000
        )
    )
    {
        return "00:00:00";
    }


    char jam[9];

    strftime(
        jam,
        sizeof(jam),
        "%H:%M:%S",
        &timeinfo
    );


    return String(jam);
}