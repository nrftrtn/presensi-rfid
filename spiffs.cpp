#include "spiffs.h"

#include <Preferences.h>


// =====================================================
// OBJECT NVS
// =====================================================

Preferences preferences;


// =====================================================
// KONFIGURASI NVS
// =====================================================

const char* NVS_NAMESPACE = "presensi";

const char* NVS_COUNT_KEY = "count";


// =====================================================
// INISIALISASI PENYIMPANAN
// =====================================================

void inisialisasiPenyimpanan()
{
    Serial.println();
    Serial.println("================================");
    Serial.println("     PENYIMPANAN OFFLINE");
    Serial.println("================================");


    // -------------------------------------------------
    // Buka NVS
    // -------------------------------------------------

    preferences.begin(
        NVS_NAMESPACE,
        false
    );


    // -------------------------------------------------
    // Ambil jumlah data
    // -------------------------------------------------

    int jumlah =
        preferences.getInt(
            NVS_COUNT_KEY,
            0
        );


    Serial.print(
        "Data offline tersimpan: "
    );

    Serial.println(jumlah);
}


// =====================================================
// SIMPAN DATA OFFLINE
// =====================================================

void simpanDataOffline(
    String uid,
    String tanggal,
    String jam
)
{
    // -------------------------------------------------
    // Ambil jumlah data saat ini
    // -------------------------------------------------

    int jumlah =
        preferences.getInt(
            NVS_COUNT_KEY,
            0
        );


    // -------------------------------------------------
    // Buat nama key
    //
    // data0
    // data1
    // data2
    // dst.
    // -------------------------------------------------

    String key =
        "data" + String(jumlah);


    // -------------------------------------------------
    // Gabungkan data
    //
    // Format:
    //
    // UID|tanggal|jam
    //
    // Contoh:
    //
    // AABBCCDD|2026-08-08|13:04:12
    // -------------------------------------------------

    String data =
        uid + "|" +
        tanggal + "|" +
        jam;


    // -------------------------------------------------
    // Simpan ke NVS
    // -------------------------------------------------

    preferences.putString(
        key.c_str(),
        data
    );


    // -------------------------------------------------
    // Tambahkan jumlah data
    // -------------------------------------------------

    jumlah++;


    preferences.putInt(
        NVS_COUNT_KEY,
        jumlah
    );


    // -------------------------------------------------
    // Serial Monitor
    // -------------------------------------------------

    Serial.println();
    Serial.println(
        "DATA OFFLINE DISIMPAN"
    );

    Serial.print(
        "UID      : "
    );

    Serial.println(uid);


    Serial.print(
        "Tanggal  : "
    );

    Serial.println(tanggal);


    Serial.print(
        "Jam      : "
    );

    Serial.println(jam);


    Serial.print(
        "Jumlah data offline : "
    );

    Serial.println(jumlah);
}


// =====================================================
// JUMLAH DATA OFFLINE
// =====================================================

int jumlahDataOffline()
{
    return preferences.getInt(
        NVS_COUNT_KEY,
        0
    );
}


// =====================================================
// AMBIL DATA OFFLINE
// =====================================================

String ambilDataOffline(
    int index
)
{
    int jumlah =
        jumlahDataOffline();


    // -------------------------------------------------
    // Validasi index
    // -------------------------------------------------

    if (
        index < 0 ||
        index >= jumlah
    )
    {
        return "";
    }


    // -------------------------------------------------
    // Nama key
    // -------------------------------------------------

    String key =
        "data" + String(index);


    // -------------------------------------------------
    // Ambil data
    // -------------------------------------------------

    return preferences.getString(
        key.c_str(),
        ""
    );
}


// =====================================================
// HAPUS DATA OFFLINE
// =====================================================

void hapusDataOffline(
    int index
)
{
    int jumlah =
        jumlahDataOffline();


    // -------------------------------------------------
    // Validasi
    // -------------------------------------------------

    if (
        index < 0 ||
        index >= jumlah
    )
    {
        return;
    }


    // -------------------------------------------------
    // Geser data berikutnya
    //
    // Contoh:
    //
    // data0
    // data1
    // data2
    //
    // Jika data0 dihapus:
    //
    // data1 → data0
    // data2 → data1
    // -------------------------------------------------

    for (
        int i = index;
        i < jumlah - 1;
        i++
    )
    {
        String keySekarang =
            "data" + String(i);


        String keyBerikutnya =
            "data" + String(i + 1);


        String dataBerikutnya =
            preferences.getString(
                keyBerikutnya.c_str(),
                ""
            );


        preferences.putString(
            keySekarang.c_str(),
            dataBerikutnya
        );
    }


    // -------------------------------------------------
    // Hapus data terakhir
    // -------------------------------------------------

    String keyTerakhir =
        "data" + String(jumlah - 1);


    preferences.remove(
        keyTerakhir.c_str()
    );


    // -------------------------------------------------
    // Kurangi jumlah
    // -------------------------------------------------

    jumlah--;


    preferences.putInt(
        NVS_COUNT_KEY,
        jumlah
    );


    // -------------------------------------------------
    // Serial Monitor
    // -------------------------------------------------

    Serial.print(
        "Data offline dihapus. Sisa: "
    );

    Serial.println(jumlah);
}


// =====================================================
// HAPUS SEMUA DATA OFFLINE
// =====================================================

void hapusSemuaDataOffline()
{
    int jumlah =
        jumlahDataOffline();


    // -------------------------------------------------
    // Hapus semua key data
    // -------------------------------------------------

    for (
        int i = 0;
        i < jumlah;
        i++
    )
    {
        String key =
            "data" + String(i);


        preferences.remove(
            key.c_str()
        );
    }


    // -------------------------------------------------
    // Reset counter
    // -------------------------------------------------

    preferences.putInt(
        NVS_COUNT_KEY,
        0
    );


    Serial.println(
        "Semua data offline dihapus."
    );
}