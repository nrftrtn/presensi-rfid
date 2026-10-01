#ifndef WIFI_MODULE_H
#define WIFI_MODULE_H

#include <Arduino.h>

extern const char* WIFI_SSID;
extern const char* WIFI_PASSWORD;

void initWiFi();

bool isWiFiConnected();

bool wifiTerhubung();

void checkWiFi();

#endif