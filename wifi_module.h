#ifndef WIFI_MODULE_H
#define WIFI_MODULE_H

#include <Arduino.h>
#include <WiFi.h>

// Inisialisasi dan status koneksi
void initWiFi();
bool isWiFiConnected();
bool wifiTerhubung();
void checkWiFi();
String getLocalIPString();

// Fungsi Portal Konfigurasi Hotspot (Captive Portal)
void startConfigPortal();
void handlePortalClient();
bool isConfigPortalActive();

// Getter konfigurasi tersimpan dari NVS
String getSavedSSID();
String getSavedServerHost();
String getSavedServerUrl();
String getSavedServerSyncUrl();

#endif