#ifndef SYNC_H
#define SYNC_H

#include <Arduino.h>

// =====================================================
// INISIALISASI SINKRONISASI
// =====================================================

void initSync();

// =====================================================
// CEK APAKAH SEDANG SINKRONISASI
// =====================================================

bool isSyncing();

// =====================================================
// SINKRONISASI DATA OFFLINE
// =====================================================

// Mengirim seluruh data offline
// dari NVS ke Laravel
void syncOfflineData();

#endif