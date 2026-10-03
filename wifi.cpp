#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <LiquidCrystal_I2C.h>
#include "wifi_module.h"

extern LiquidCrystal_I2C lcd;

// =====================================================
// KONFIGURASI CAPTIVE PORTAL & WEB SERVER
// =====================================================

static WebServer server(80);
static DNSServer dnsServer;
static const byte DNS_PORT = 53;

// Hotspot Setup ESP32
static const char* AP_SSID = "SIPUDA-Presensi-Setup";
static const IPAddress AP_IP(192, 168, 4, 1);
static const IPAddress AP_NETMASK(255, 255, 255, 0);

static bool isPortalRunning = false;
static unsigned long portalStartTime = 0;
static const unsigned long PORTAL_TIMEOUT_MS = 180000; // 3 Menit

// NVS Namespace
static const char* PREFS_NAMESPACE = "netconfig";

// Default Fallback
static const char* DEFAULT_SSID = "Almahfudzy";
static const char* DEFAULT_PASS = "nuri12345";
static const char* DEFAULT_SERVER = "http://192.168.233.195:8000";

// =====================================================
// FUNGSI BACA PENGATURAN TERSIMPAN DARI NVS
// =====================================================

String getSavedSSID()
{
    Preferences prefs;
    prefs.begin(PREFS_NAMESPACE, true);
    String ssid = prefs.getString("ssid", DEFAULT_SSID);
    prefs.end();
    return ssid;
}

String getSavedPass()
{
    Preferences prefs;
    prefs.begin(PREFS_NAMESPACE, true);
    String pass = prefs.getString("pass", DEFAULT_PASS);
    prefs.end();
    return pass;
}

String getSavedServerHost()
{
    Preferences prefs;
    prefs.begin(PREFS_NAMESPACE, true);
    String host = prefs.getString("server", DEFAULT_SERVER);
    prefs.end();
    return host;
}

String getSavedServerUrl()
{
    String host = getSavedServerHost();
    if (!host.endsWith("/")) {
        return host + "/api/rfid/scan";
    }
    return host + "api/rfid/scan";
}

String getSavedServerSyncUrl()
{
    String host = getSavedServerHost();
    if (!host.endsWith("/")) {
        return host + "/api/rfid/sync";
    }
    return host + "api/rfid/sync";
}

// =====================================================
// HALAMAN HTML PORTAL KONFIGURASI
// =====================================================

static String getPortalHtml()
{
    String currentSSID = getSavedSSID();
    String currentServer = getSavedServerHost();

    // Pindai jaringan WiFi sekitar
    int n = WiFi.scanNetworks();
    String options = "";
    for (int i = 0; i < n; ++i) {
        String netSsid = WiFi.SSID(i);
        int rssi = WiFi.RSSI(i);
        String selected = (netSsid == currentSSID) ? " selected" : "";
        options += "<option value=\"" + netSsid + "\"" + selected + ">" + netSsid + " (" + String(rssi) + " dBm)</option>";
    }

    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>SIPUDA - Setup ESP32</title>
    <style>
        body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; background: #f0fdf4; margin: 0; padding: 20px; color: #1e293b; }
        .card { max-width: 440px; margin: auto; background: white; padding: 24px; border-radius: 18px; box-shadow: 0 4px 20px rgba(0,0,0,0.08); }
        h2 { color: #15803d; margin-top: 0; font-size: 22px; text-align: center; }
        p { font-size: 13px; color: #64748b; text-align: center; margin-bottom: 20px; }
        label { font-size: 12px; font-weight: bold; color: #334155; display: block; margin-top: 14px; margin-bottom: 6px; }
        input, select { width: 100%; padding: 10px 12px; border: 1px solid #cbd5e1; border-radius: 10px; box-sizing: border-box; font-size: 14px; }
        input:focus, select:focus { border-color: #22c55e; outline: none; box-shadow: 0 0 0 2px rgba(34,197,94,0.2); }
        button { width: 100%; padding: 12px; background: #16a34a; color: white; border: none; border-radius: 10px; font-size: 15px; font-weight: bold; margin-top: 24px; cursor: pointer; transition: background .2s; }
        button:hover { background: #15803d; }
        .hint { font-size: 11px; color: #94a3b8; margin-top: 4px; }
    </style>
</head>
<body>
    <div class="card">
        <h2>⚙️ Konfigurasi SIPUDA RFID</h2>
        <p>Hubungkan perangkat ESP32 ke jaringan Wi-Fi dan Server Laravel</p>
        <form action="/save" method="POST">
            <label>Pilih Wi-Fi Terdekat:</label>
            <select onchange="document.getElementById('ssid_input').value=this.value;">
                <option value="">-- Pilih dari hasil scan Wi-Fi --</option>
)rawliteral";

    html += options;

    html += R"rawliteral(
            </select>

            <label>Nama Wi-Fi (SSID):</label>
            <input type="text" id="ssid_input" name="ssid" value=")rawliteral";
    html += currentSSID;
    html += R"rawliteral(" required placeholder="Nama Wi-Fi / Hotspot HP">

            <label>Password Wi-Fi:</label>
            <input type="password" name="password" placeholder="Password Wi-Fi (kosongkan jika tanpa password)">

            <label>Alamat Server Laravel (URL / IP):</label>
            <input type="text" name="server" value=")rawliteral";
    html += currentServer;
    html += R"rawliteral(" required placeholder="http://192.168.x.x:8000">
            <div class="hint">Contoh: http://192.168.233.195:8000</div>

            <button type="submit">💾 Simpan & Sambungkan</button>
        </form>
    </div>
</body>
</html>
)rawliteral";

    return html;
}

static bool routesConfigured = false;

static void setupServerRoutes()
{
    if (routesConfigured) return;

    server.on("/", HTTP_GET, []() {
        server.send(200, "text/html", getPortalHtml());
    });

    server.on("/save", HTTP_POST, []() {
        String newSsid = server.arg("ssid");
        String newPass = server.arg("password");
        String newServer = server.arg("server");

        newSsid.trim();
        newPass.trim();
        newServer.trim();

        if (newServer.endsWith("/")) {
            newServer = newServer.substring(0, newServer.length() - 1);
        }

        Preferences prefs;
        prefs.begin(PREFS_NAMESPACE, false);
        prefs.putString("ssid", newSsid);
        prefs.putString("pass", newPass);
        prefs.putString("server", newServer);
        prefs.end();

        Serial.println("\n[OK] Pengaturan baru berhasil disimpan ke NVS:");
        Serial.println("SSID   : " + newSsid);
        Serial.println("Server : " + newServer);

        String resp = "<!DOCTYPE html><html><body style='font-family:sans-serif;text-align:center;padding:40px;background:#f0fdf4;color:#1e293b;'>"
                      "<h2 style='color:#15803d;'>✅ Pengaturan Berhasil Disimpan!</h2>"
                      "<p>ESP32 sedang me-restart untuk terhubung ke <b>" + newSsid + "</b>...</p>"
                      "<p>Silakan hubungkan kembali HP/laptop Anda ke Wi-Fi utama.</p>"
                      "</body></html>";
        server.send(200, "text/html", resp);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Tersimpan!");
        lcd.setCursor(0, 1);
        lcd.print("Me-restart...");

        delay(2000);
        ESP.restart();
    });

    // Captive Portal Redirects (Android, iOS, Windows)
    server.on("/generate_204", []() {
        server.sendHeader("Location", "http://192.168.4.1/", true);
        server.send(302, "text/plain", "");
    });
    server.on("/hotspot-detect.html", []() {
        server.sendHeader("Location", "http://192.168.4.1/", true);
        server.send(302, "text/plain", "");
    });
    server.onNotFound([]() {
        server.sendHeader("Location", "http://192.168.4.1/", true);
        server.send(302, "text/plain", "");
    });

    routesConfigured = true;
}

// =====================================================
// MENJALANKAN CAPTIVE PORTAL HOTSPOT
// =====================================================

void startConfigPortal()
{
    Serial.println();
    Serial.println("================================");
    Serial.println("   MODE PORTAL KONFIGURASI");
    Serial.println("================================");

    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(AP_IP, AP_IP, AP_NETMASK);
    WiFi.softAP(AP_SSID);

    Serial.print("Hotspot Aktif: ");
    Serial.println(AP_SSID);
    Serial.print("Buka browser ke: http://");
    Serial.println(AP_IP);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("AP: SIPUDA-Setup");
    lcd.setCursor(0, 1);
    lcd.print("IP: 192.168.4.1");

    dnsServer.start(DNS_PORT, "*", AP_IP);

    setupServerRoutes();
    server.begin();
    isPortalRunning = true;
    portalStartTime = millis();

    while (isPortalRunning) {
        dnsServer.processNextRequest();
        server.handleClient();
        delay(10);

        // Jika lewat 3 menit tanpa konfigurasi, beralih ke mode offline
        if (millis() - portalStartTime > PORTAL_TIMEOUT_MS) {
            Serial.println();
            Serial.println("Portal timeout. Melanjutkan dalam mode OFFLINE.");
            break;
        }
    }

    server.stop();
    dnsServer.stop();
    WiFi.softAPdisconnect(true);
    isPortalRunning = false;
}

bool isConfigPortalActive()
{
    return isPortalRunning;
}

void handlePortalClient()
{
    if (isPortalRunning) {
        dnsServer.processNextRequest();
    }
    server.handleClient();
}

// =====================================================
// INISIALISASI WIFI (MODE NORMAL)
// =====================================================

void initWiFi()
{
    String ssid = getSavedSSID();
    String pass = getSavedPass();

    Serial.println();
    Serial.println("================================");
    Serial.println("       KONEKSI WIFI");
    Serial.println("================================");

    Serial.print("Menghubungkan ke: ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    int percobaan = 0;
    while (WiFi.status() != WL_CONNECTED && percobaan < 25) {
        delay(500);
        Serial.print(".");
        percobaan++;
    }

    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi berhasil terhubung!");
        Serial.print("IP ESP32 : ");
        Serial.println(WiFi.localIP());
        Serial.print("Gateway  : ");
        Serial.println(WiFi.gatewayIP());
        Serial.print("RSSI     : ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("WiFi Terhubung!");
        lcd.setCursor(0, 1);
        lcd.print(WiFi.localIP());
        delay(2500);

        setupServerRoutes();
        server.begin();
        Serial.print("Web server aktif di: http://");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("WiFi gagal terhubung dengan pengaturan tersimpan.");
        Serial.println("Membuka Hotspot Portal Konfigurasi...");

        // Jalankan portal setup hotspot
        startConfigPortal();
    }
}

// =====================================================
// STATUS KONEKSI
// =====================================================

bool isWiFiConnected()
{
    return (WiFi.status() == WL_CONNECTED);
}

bool wifiTerhubung()
{
    return isWiFiConnected();
}

String getLocalIPString()
{
    if (isWiFiConnected()) {
        return WiFi.localIP().toString();
    }
    return "";
}

// =====================================================
// CEK DAN SAMBUNG KEMBALI
// =====================================================

void checkWiFi()
{
    if (isWiFiConnected() || isPortalRunning) {
        return;
    }

    String ssid = getSavedSSID();
    String pass = getSavedPass();

    Serial.println();
    Serial.println("WiFi terputus. Mencoba menghubungkan kembali...");

    WiFi.disconnect();
    WiFi.begin(ssid.c_str(), pass.c_str());

    int percobaan = 0;
    while (WiFi.status() != WL_CONNECTED && percobaan < 10) {
        delay(500);
        Serial.print(".");
        percobaan++;
    }

    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi terhubung kembali!");
        Serial.print("IP ESP32 : ");
        Serial.println(WiFi.localIP());
        setupServerRoutes();
        server.begin();
    } else {
        Serial.println("Masih offline.");
    }
}