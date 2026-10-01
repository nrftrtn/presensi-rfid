#include <Arduino.h>
#include <WiFi.h>
#include "wifi_module.h"

// =====================================================
// KONFIGURASI WIFI
// =====================================================

const char* WIFI_SSID = "Almahfudzy";
const char* WIFI_PASSWORD = "nuri12345";

// =====================================================
// INISIALISASI WIFI
// =====================================================

void initWiFi()
{
    Serial.println();
    Serial.println("================================");
    Serial.println("       KONEKSI WIFI");
    Serial.println("================================");

    Serial.print("Menghubungkan ke: ");
    Serial.println(WIFI_SSID);

    WiFi.mode(WIFI_STA);

    WiFi.begin(
        WIFI_SSID,
        WIFI_PASSWORD
    );

    int percobaan = 0;

    while (
        WiFi.status() != WL_CONNECTED &&
        percobaan < 30
    )
    {
        delay(500);
        Serial.print(".");
        percobaan++;
    }

    Serial.println();

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println(
            "WiFi berhasil terhubung!"
        );

        Serial.print("IP ESP32 : ");
        Serial.println(
            WiFi.localIP()
        );

        Serial.print("Gateway  : ");
        Serial.println(
            WiFi.gatewayIP()
        );

        Serial.print("RSSI     : ");
        Serial.print(
            WiFi.RSSI()
        );

        Serial.println(" dBm");
    }
    else
    {
        Serial.println(
            "WiFi gagal terhubung."
        );

        Serial.println(
            "Mode OFFLINE akan digunakan."
        );
    }
}

// =====================================================
// CEK WIFI
// =====================================================

bool isWiFiConnected()
{
    return (
        WiFi.status() ==
        WL_CONNECTED
    );
}

// =====================================================
// KOMPATIBILITAS NAMA LAMA
// =====================================================

bool wifiTerhubung()
{
    return isWiFiConnected();
}

// =====================================================
// CEK DAN SAMBUNG KEMBALI WIFI
// =====================================================

void checkWiFi()
{
    if (
        WiFi.status() ==
        WL_CONNECTED
    )
    {
        return;
    }

    Serial.println();
    Serial.println(
        "WiFi terputus."
    );

    Serial.println(
        "Mencoba menghubungkan kembali..."
    );

    WiFi.disconnect();

    WiFi.begin(
        WIFI_SSID,
        WIFI_PASSWORD
    );

    int percobaan = 0;

    while (
        WiFi.status() != WL_CONNECTED &&
        percobaan < 10
    )
    {
        delay(500);
        Serial.print(".");
        percobaan++;
    }

    Serial.println();

    if (
        WiFi.status() ==
        WL_CONNECTED
    )
    {
        Serial.println(
            "WiFi terhubung kembali!"
        );

        Serial.print(
            "IP ESP32 : "
        );

        Serial.println(
            WiFi.localIP()
        );
    }
    else
    {
        Serial.println(
            "Masih offline."
        );
    }
}