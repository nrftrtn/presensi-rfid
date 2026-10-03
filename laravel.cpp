#include "laravel.h"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "wifi_module.h"


// =====================================================
// FUNGSI ONLINE
// =====================================================

HasilAbsensi kirimAbsensiKeLaravel(String uid)
{
    HasilAbsensi hasil;

    hasil.berhasil = false;
    hasil.nama = "";
    hasil.kegiatan = "";
    hasil.status = "";
    hasil.jam = "";
    hasil.message = "";

    // =================================================
    // CEK WIFI
    // =================================================

    if (!wifiTerhubung())
    {
        hasil.message = "WiFi tidak terhubung";
        return hasil;
    }

    // =================================================
    // INFORMASI
    // =================================================

    Serial.println();
    Serial.println("================================");
    Serial.println("       KIRIM KE LARAVEL");
    Serial.println("================================");

    Serial.print("UID : ");
    Serial.println(uid);

    String serverUrl = getSavedServerUrl();

    Serial.print("URL : ");
    Serial.println(serverUrl);

    // =================================================
    // DEBUG WIFI
    // =================================================

    Serial.println();
    Serial.println("===== DEBUG KONEKSI =====");

    Serial.print("WiFi Status : ");
    Serial.println(WiFi.status());

    Serial.print("SSID        : ");
    Serial.println(WiFi.SSID());

    Serial.print("IP ESP32    : ");
    Serial.println(WiFi.localIP());

    Serial.print("Gateway     : ");
    Serial.println(WiFi.gatewayIP());

    Serial.print("Subnet      : ");
    Serial.println(WiFi.subnetMask());

    Serial.print("RSSI        : ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");

    Serial.println("========================");

    // =================================================
    // HTTP
    // =================================================

    HTTPClient http;
    WiFiClient client;
    WiFiClientSecure secureClient;

    if (serverUrl.startsWith("https://")) {
        secureClient.setInsecure();
        http.begin(secureClient, serverUrl);
    } else {
        http.begin(client, serverUrl);
    }

    http.addHeader(
        "Content-Type",
        "application/json"
    );

    http.setTimeout(5000);

    // =================================================
    // JSON
    // =================================================

    String jsonData =
        "{\"uid_rfid\":\"" +
        uid +
        "\"}";

    Serial.print("JSON : ");
    Serial.println(jsonData);

    // =================================================
    // POST
    // =================================================

    int httpCode = http.POST(jsonData);

    Serial.print("HTTP Code : ");
    Serial.println(httpCode);

    // =================================================
    // GAGAL TERHUBUNG
    // =================================================

    if (httpCode <= 0)
    {
        Serial.println(
            "Gagal menghubungi Laravel."
        );

        hasil.message =
            "Server tidak terhubung";

        http.end();

        return hasil;
    }

    // =================================================
    // RESPONSE
    // =================================================

    String response = http.getString();

    Serial.println();
    Serial.println("Response Laravel:");
    Serial.println(response);

    // =================================================
    // PARSING JSON
    // =================================================

    DynamicJsonDocument doc(2048);

    DeserializationError error =
        deserializeJson(doc, response);

    if (error)
    {
        Serial.print("JSON error: ");
        Serial.println(error.c_str());

        hasil.message =
            "Response server tidak valid";

        http.end();

        return hasil;
    }

    // =================================================
    // STATUS
    // =================================================

    hasil.berhasil =
        doc["status"] | false;

    hasil.message =
        doc["message"] | "";

    // =================================================
    // ABSENSI BERHASIL
    // =================================================

    if (hasil.berhasil)
    {
        hasil.nama =
            doc["nama"] | "";

        hasil.kegiatan =
            doc["kegiatan"] | "";

        hasil.status =
            doc["status_kehadiran"] | "";

        hasil.jam =
            doc["jam"] | "";

        Serial.println();
        Serial.println("ABSENSI BERHASIL");

        Serial.print("Nama     : ");
        Serial.println(hasil.nama);

        Serial.print("Kegiatan : ");
        Serial.println(hasil.kegiatan);

        Serial.print("Status   : ");
        Serial.println(hasil.status);

        Serial.print("Jam      : ");
        Serial.println(hasil.jam);
    }

    // =================================================
    // ABSENSI DITOLAK
    // =================================================

    else
    {
        Serial.println();
        Serial.println("ABSENSI DITOLAK");

        Serial.print("Pesan : ");
        Serial.println(hasil.message);
    }

    http.end();

    return hasil;
}


// =====================================================
// FUNGSI SINKRONISASI OFFLINE
// =====================================================

bool kirimDataOfflineKeLaravel(
    String uid,
    String tanggal,
    String jam
)
{
    // =================================================
    // CEK WIFI
    // =================================================

    if (!wifiTerhubung())
    {
        Serial.println(
            "WiFi tidak terhubung."
        );

        return false;
    }

    // =================================================
    // INFORMASI
    // =================================================

    Serial.println();
    Serial.println(
        "================================"
    );

    Serial.println(
        "   KIRIM DATA OFFLINE"
    );

    Serial.println(
        "================================"
    );

    Serial.print("UID     : ");
    Serial.println(uid);

    Serial.print("Tanggal : ");
    Serial.println(tanggal);

    Serial.print("Jam     : ");
    Serial.println(jam);

    String serverSyncUrl = getSavedServerSyncUrl();

    Serial.print("URL     : ");
    Serial.println(serverSyncUrl);

    // =================================================
    // HTTP
    // =================================================

    HTTPClient http;
    WiFiClient client;
    WiFiClientSecure secureClient;

    if (serverSyncUrl.startsWith("https://")) {
        secureClient.setInsecure();
        http.begin(secureClient, serverSyncUrl);
    } else {
        http.begin(client, serverSyncUrl);
    }

    http.addHeader(
        "Content-Type",
        "application/json"
    );

    http.setTimeout(5000);

    // =================================================
    // JSON
    // =================================================

    String jsonData =
        "{\"uid_rfid\":\"" +
        uid +
        "\",\"tanggal\":\"" +
        tanggal +
        "\",\"jam_absen\":\"" +
        jam +
        "\"}";

    Serial.print("JSON : ");
    Serial.println(jsonData);

    // =================================================
    // POST
    // =================================================

    int httpCode =
        http.POST(jsonData);

    Serial.print("HTTP Code : ");
    Serial.println(httpCode);

    // =================================================
    // GAGAL
    // =================================================

    if (httpCode <= 0)
    {
        Serial.println(
            "[GAGAL] Server tidak terhubung."
        );

        http.end();

        return false;
    }

    // =================================================
    // RESPONSE
    // =================================================

    String response =
        http.getString();

    Serial.println();
    Serial.println(
        "Response Laravel:"
    );

    Serial.println(response);

    // =================================================
    // PARSING JSON
    // =================================================

    DynamicJsonDocument doc(2048);

    DeserializationError error =
        deserializeJson(
            doc,
            response
        );

    if (error)
    {
        Serial.print("JSON error: ");
        Serial.println(error.c_str());

        http.end();

        return false;
    }

    // =================================================
    // CEK STATUS
    // =================================================

    bool status =
        doc["status"] | false;

    String message =
        doc["message"] | "";

    // =================================================
    // BERHASIL
    // =================================================

    if (status)
    {
        Serial.println();
        Serial.println(
            "[OK] Data offline berhasil "
            "disinkronkan."
        );

        Serial.print("Pesan : ");
        Serial.println(message);

        http.end();

        return true;
    }

    // =================================================
    // DITOLAK
    // =================================================

    Serial.println();
    Serial.println(
        "[GAGAL] Data offline ditolak."
    );

    Serial.print("Pesan : ");
    Serial.println(message);

    http.end();

    return false;
}