#include "rfid.h"

// =====================================================
// OBJECT RC522
// =====================================================

MFRC522 rfid(
    RFID_SS_PIN,
    RFID_RST_PIN
);

// =====================================================
// INISIALISASI RFID
// =====================================================

void initRFID()
{
    Serial.println();
    Serial.println("================================");
    Serial.println("       INISIALISASI RC522");
    Serial.println("================================");

    // SPI ESP32
    SPI.begin(
        RFID_SCK,
        RFID_MISO,
        RFID_MOSI,
        RFID_SS_PIN
    );

    // Inisialisasi RC522
    rfid.PCD_Init();

    delay(100);

    // Baca versi RC522
    byte version =
        rfid.PCD_ReadRegister(
            MFRC522::VersionReg
        );

    Serial.print("RC522 Version : 0x");
    Serial.println(version, HEX);

    if (
        version == 0x91 ||
        version == 0x92
    )
    {
        Serial.println("RC522 TERDETEKSI");
    }
    else
    {
        Serial.println("RC522 TIDAK TERDETEKSI");
    }
}

// =====================================================
// CEK RC522
// =====================================================

bool cekRFID()
{
    byte version =
        rfid.PCD_ReadRegister(
            MFRC522::VersionReg
        );

    if (
        version == 0x91 ||
        version == 0x92
    )
    {
        return true;
    }

    return false;
}

// =====================================================
// CEK KARTU / GELANG
// =====================================================

bool adaKartuRFID()
{
    // Tidak ada kartu
    if (!rfid.PICC_IsNewCardPresent())
    {
        return false;
    }

    // Kartu ada tetapi gagal dibaca
    if (!rfid.PICC_ReadCardSerial())
    {
        return false;
    }

    return true;
}

// =====================================================
// BACA UID
// =====================================================

String readRFID()
{
    // Tidak ada kartu
    if (!rfid.PICC_IsNewCardPresent())
    {
        return "";
    }

    // Kartu ada tetapi gagal dibaca
    if (!rfid.PICC_ReadCardSerial())
    {
        return "";
    }

    String uid = "";

    for (
        byte i = 0;
        i < rfid.uid.size;
        i++
    )
    {
        if (rfid.uid.uidByte[i] < 0x10)
        {
            uid += "0";
        }

        uid += String(
            rfid.uid.uidByte[i],
            HEX
        );

        if (
            i < rfid.uid.size - 1
        )
        {
            uid += ":";
        }
    }

    uid.toUpperCase();

    Serial.print("UID RFID : ");
    Serial.println(uid);

    // Selesai komunikasi
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();

    return uid;
}

// =====================================================
// SELESAI MEMBACA RFID
// =====================================================

void selesaiRFID()
{
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
}