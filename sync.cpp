#include "sync.h"

#include "wifi_module.h"
#include "laravel.h"
#include "spiffs.h"
#include "waktu.h"
#include "tampilan.h"

// =====================================================
// STATUS SINKRONISASI
// =====================================================

bool sedangSync = false;

// =====================================================
// INISIALISASI SYNC
// =====================================================

void initSync()
{
    sedangSync = false;

    Serial.println();
    Serial.println("================================");
    Serial.println("   MODUL SINKRONISASI SIAP");
    Serial.println("================================");
}

// =====================================================
// CEK STATUS SYNC
// =====================================================

bool isSyncing()
{
    return sedangSync;
}

// =====================================================
// SINKRONISASI DATA OFFLINE
// =====================================================

void syncOfflineData()
{
    // -------------------------------------------------
    // Jangan sinkronisasi jika WiFi tidak tersedia
    // -------------------------------------------------

    if (!isWiFiConnected())
    {
        return;
    }

    // -------------------------------------------------
    // Cek jumlah data offline
    // -------------------------------------------------

    int jumlah = jumlahDataOffline();

    if (jumlah <= 0)
    {
        return;
    }

    // -------------------------------------------------
    // Mulai sinkronisasi
    // -------------------------------------------------

    sedangSync = true;

    Serial.println();
    Serial.println("================================");
    Serial.println("   SINKRONISASI DATA OFFLINE");
    Serial.println("================================");

    Serial.print("Jumlah data : ");
    Serial.println(jumlah);

    // -------------------------------------------------
    // Kirim data satu per satu
    // -------------------------------------------------

    int index = 0;

    while (index < jumlahDataOffline())
    {
        // =============================================
        // AMBIL DATA DARI NVS
        // Format:
        //
        // UID|TANGGAL|JAM
        //
        // Contoh:
        // AABBCCDD|2026-08-08|13:04:12
        // =============================================

        String dataOffline =
            ambilDataOffline(index);

        if (dataOffline == "")
        {
            Serial.println(
                "Data kosong, dilewati."
            );

            index++;

            continue;
        }

        Serial.println();
        Serial.println("--------------------------------");
        Serial.print("Data offline ke-");
        Serial.println(index + 1);

        Serial.print("Data : ");
        Serial.println(dataOffline);

        // =============================================
        // PISAHKAN UID
        // =============================================

        int pemisahPertama =
            dataOffline.indexOf('|');

        int pemisahKedua =
            dataOffline.indexOf(
                '|',
                pemisahPertama + 1
            );

        // =============================================
        // VALIDASI FORMAT DATA
        // =============================================

        if (
            pemisahPertama == -1 ||
            pemisahKedua == -1
        )
        {
            Serial.println(
                "[ERROR] Format data offline salah."
            );

            index++;

            continue;
        }

        // =============================================
        // AMBIL UID
        // =============================================

        String uid =
            dataOffline.substring(
                0,
                pemisahPertama
            );

        // =============================================
        // AMBIL TANGGAL DAN JAM
        // =============================================

        String tanggal =
            dataOffline.substring(
                pemisahPertama + 1,
                pemisahKedua
            );

        String jam =
            dataOffline.substring(
                pemisahKedua + 1
            );

        // Jika tanggal/jam belum tersinkron saat tersimpan (0000-00-00),
        // gunakan waktu NTP saat ini jika sudah tersedia
        if (tanggal == "0000-00-00" || tanggal == "" || jam == "00:00:00" || jam == "")
        {
            String nowTgl = getTanggal();
            String nowJam = getJam();

            if (nowTgl != "0000-00-00" && nowTgl != "")
            {
                Serial.println("Memperbaiki tanggal offline dengan waktu sekarang...");
                tanggal = nowTgl;
                jam     = nowJam;
            }
            else
            {
                Serial.println("Data offline tanggal tidak valid dan NTP belum siap. Dihapus.");
                hapusDataOffline(index);
                continue;
            }
        }

        // =============================================
        // TAMPILKAN DATA
        // =============================================

        Serial.print("UID     : ");
        Serial.println(uid);

        Serial.print("Tanggal : ");
        Serial.println(tanggal);

        Serial.print("Jam     : ");
        Serial.println(jam);

        // =============================================
        // KIRIM DATA KE LARAVEL
        // =============================================

        Serial.println(
            "Mengirim data ke Laravel..."
        );

        bool berhasil =
            kirimDataOfflineKeLaravel(
                uid,
                tanggal,
                jam
            );

        // =============================================
        // JIKA BERHASIL
        // =============================================

        if (berhasil)
        {
            Serial.println(
                "[OK] Data berhasil disinkronkan."
            );

            // -----------------------------------------
            // Hapus data dari NVS
            // -----------------------------------------

            hapusDataOffline(index);

            Serial.println(
                "Data dihapus dari penyimpanan offline."
            );

            // -----------------------------------------
            // JANGAN TAMBAH INDEX
            //
            // Karena setelah data dihapus,
            // data berikutnya bergeser ke index ini.
            // -----------------------------------------
        }

        // =============================================
        // JIKA GAGAL
        // =============================================

        else
        {
            Serial.println(
                "[GAGAL] Sinkronisasi gagal."
            );

            Serial.println(
                "Sinkronisasi dihentikan."
            );

            Serial.println(
                "Data tetap disimpan di NVS."
            );

            break;
        }

        delay(300);
    }

    // =================================================
    // SELESAI
    // =================================================

    int sisa =
        jumlahDataOffline();

    Serial.println();
    Serial.println("================================");
    Serial.println("   SINKRONISASI SELESAI");
    Serial.println("================================");

    Serial.print("Data tersisa : ");
    Serial.println(sisa);

    sedangSync = false;
}