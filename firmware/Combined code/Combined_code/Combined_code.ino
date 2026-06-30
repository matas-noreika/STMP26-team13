#include <Arduino.h>           // Core Arduino framework: Provides basic functions like setup(), loop(), and delay()
#include <WiFi.h>              // Wi-Fi Driver: Allows the ESP32 chip to connect to an access point
#include <WiFiClientSecure.h>  // SSL/TLS Layer: Enables secure, encrypted network connections for HTTPS
#include <HTTPClient.h>        // HTTP Handler: Provides methods like GET and POST to request or send data
#include <ArduinoJson.h>       // JSON Parser: Used to pack data into JSON format or parse received JSON payloads

// ─────────────────────────────────────────────
//  User configuration  ← edit these
// ─────────────────────────────────────────────
static constexpr char WIFI_SSID[]     = "Matas ltu";
static constexpr char WIFI_PASSWORD[] = "matas-ltu";

// GitHub Personal Access Token
//   Scopes needed:  repo  (for private repos / creating issues)
//   or just:        public_repo  (for public repos only)
static constexpr char GITHUB_TOKEN[]  = "Add token here manual from whatsapp message";

// Owner (your GitHub username or org) and target repository
static constexpr char GITHUB_OWNER[] = "matas-noreika";
static constexpr char GITHUB_REPO[]  = "STMP26-team13";

// ─────────────────────────────────────────────
//  GitHub API constants
// ─────────────────────────────────────────────
static constexpr char GITHUB_HOST[]        = "api.github.com";
static constexpr uint16_t HTTPS_PORT       = 443;
static constexpr char API_BASE[]           = "https://api.github.com";
static constexpr char USER_AGENT[]         = "ESP32-S3-GitHub-Client/1.0";
static constexpr char ACCEPT_HEADER[]      = "application/vnd.github+json";
static constexpr char API_VERSION_HEADER[] = "2022-11-28";   // X-GitHub-Api-Version

// ─────────────────────────────────────────────
//  Root CA for api.github.com
//  (DigiCert Global Root CA – valid until 2031)
// ─────────────────────────────────────────────
static constexpr char GITHUB_ROOT_CA[] = R"(
-----BEGIN CERTIFICATE-----
MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD
QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB
CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97
nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt
43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P
T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4
gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2IwYDAO
BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR
TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw
DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr
hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg
06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF
PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls
YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk
CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=
-----END CERTIFICATE-----
)";

// ─────────────────────────────────────────────
//  Helper: add standard GitHub API headers
// ─────────────────────────────────────────────
void addGitHubHeaders(HTTPClient& http) {
    String auth = "Bearer ";
    auth += GITHUB_TOKEN;                 
    http.addHeader("Authorization", auth); 
    http.addHeader("User-Agent", USER_AGENT); 
    http.addHeader("Accept", ACCEPT_HEADER); 
    http.addHeader("X-GitHub-Api-Version", API_VERSION_HEADER); 
}

// ─────────────────────────────────────────────
//  Example 1 – GET /user
// ─────────────────────────────────────────────
void exampleGetAuthenticatedUser() {
    Serial.println("\n── Example 1: GET /user ──────────────────────");

    WiFiClientSecure secureClient;         
    secureClient.setCACert(GITHUB_ROOT_CA); 
    secureClient.setTimeout(15);           

    HTTPClient http;                       
    String url = String(API_BASE) + "/user"; 

    if (!http.begin(secureClient, url)) {
        Serial.println("[ERROR] http.begin() failed");
        return;
    }
    addGitHubHeaders(http);               

    int httpCode = http.GET();             
    Serial.printf("HTTP status: %d\n", httpCode); 

    if (httpCode == HTTP_CODE_OK) {        
        String payload = http.getString(); 

        JsonDocument doc;                  
        DeserializationError err = deserializeJson(doc, payload); 
        if (err) {                         
            Serial.printf("[ERROR] JSON parse: %s\n", err.c_str());
        } else {                           
            Serial.printf("Login       : %s\n", doc["login"].as<const char*>());
            Serial.printf("Name        : %s\n", doc["name"].as<const char*>());
            Serial.printf("Public repos: %d\n", doc["public_repos"].as<int>());
            Serial.printf("Followers   : %d\n", doc["followers"].as<int>());
        }
    } else {                               
        Serial.printf("[ERROR] Body: %s\n", http.getString().c_str());
    }

    http.end();                            
}

// ─────────────────────────────────────────────
//  Example 2 – GET /user/repos
// ─────────────────────────────────────────────
void exampleListRepos() {
    Serial.println("\n── Example 2: GET /user/repos ────────────────");

    WiFiClientSecure secureClient;         
    secureClient.setCACert(GITHUB_ROOT_CA); 
    secureClient.setTimeout(15);

    HTTPClient http;
    String url = String(API_BASE) + "/user/repos?per_page=5&sort=updated";

    if (!http.begin(secureClient, url)) {
        Serial.println("[ERROR] http.begin() failed");
        return;
    }
    addGitHubHeaders(http);

    int httpCode = http.GET();             
    Serial.printf("HTTP status: %d\n", httpCode);

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();

        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, payload);
        if (err) {
            Serial.printf("[ERROR] JSON parse: %s\n", err.c_str());
        } else {
            JsonArray repos = doc.as<JsonArray>(); 
            Serial.printf("Repos returned: %d\n", (int)repos.size());
            
            for (JsonObject repo : repos) {
                Serial.printf("  • %-35s  stars: %d  forks: %d\n",
                    repo["full_name"].as<const char*>(),
                    repo["stargazers_count"].as<int>(),
                    repo["forks_count"].as<int>());
            }
        }
    } else {
        Serial.printf("[ERROR] Body: %s\n", http.getString().c_str());
    }

    http.end();
}

// ─────────────────────────────────────────────
//  Example 3 – POST /repos/:owner/:repo/issues
// ─────────────────────────────────────────────
void exampleCreateIssue() {
    Serial.println("\n── Example 3: POST /repos/.../issues ─────────");

    WiFiClientSecure secureClient;
    secureClient.setCACert(GITHUB_ROOT_CA);
    secureClient.setTimeout(15);
    
    // CRITICAL DETAIL: secureClient.setInsecure() forces the ESP32 to bypass CA validation.
    // While it still encrypts, it leaves the "setCACert(GITHUB_ROOT_CA)" above redundant.
    secureClient.setInsecure();

    HTTPClient http;
    String url = String(API_BASE)
               + "/repos/" + GITHUB_OWNER
               + "/" + GITHUB_REPO
               + "/issues";

    if (!http.begin(secureClient, url)) {
        Serial.println("[ERROR] http.begin() failed");
        return;
    }
    addGitHubHeaders(http);
    http.addHeader("Content-Type", "application/json");

    JsonDocument body;
    body["title"] = "Test issue from ESP32-S3"; 
    body["body"]  = "This issue was created automatically by an ESP32-S3 "
                    "running the arduino-esp32 core over HTTPS."; 
    body["labels"].add("arduino");            
    body["labels"].add("esp32");              

    String bodyStr;
    serializeJson(body, bodyStr);             
    Serial.printf("POST body: %s\n", bodyStr.c_str());

    int httpCode = http.POST(bodyStr);         
    Serial.printf("HTTP status: %d\n", httpCode);   

    if (httpCode == HTTP_CODE_CREATED) {       
        String payload = http.getString();
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, payload);
        if (!err) {
            Serial.printf("Issue #%d created!\n", doc["number"].as<int>());
            Serial.printf("URL: %s\n",            doc["html_url"].as<const char*>());
        }
    } else {
        Serial.printf("[ERROR] Body: %s\n", http.getString().c_str());
    }

    http.end();
}

// ─────────────────────────────────────────────
//  WiFi connect helper
// ─────────────────────────────────────────────
void connectWiFi() {
    Serial.printf("Connecting to %s", WIFI_SSID);
    WiFi.mode(WIFI_STA);                  
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD); 

    constexpr uint32_t TIMEOUT_MS = 20000; 
    uint32_t start = millis();             
    
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start > TIMEOUT_MS) {
            Serial.println("\n[ERROR] WiFi timeout – rebooting.");
            ESP.restart();                 
        }
        delay(500);                        
        Serial.print('.');                 
    }
    Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());
}

// ─────────────────────────────────────────────
//  Arduino entry points
// ─────────────────────────────────────────────
void setup() {
    Serial.begin(115200);                  
    delay(1000);                           
    Serial.println("\n=== ESP32-S3 GitHub REST API Demo ===");

    connectWiFi();                         

    // Setup active examples execution sequence below:
    // exampleGetAuthenticatedUser();      // (Disabled) GET  /user
    // exampleListRepos();                 // (Disabled) GET  /user/repos
    exampleCreateIssue();                  // (Enabled) POST /repos/:owner/:repo/issues

    Serial.println("\n=== All examples complete. ===");
    Serial.println("Uncomment deep-sleep below to wake periodically.");

    // Optional 60-second deep sleep then repeat
    // esp_sleep_enable_timer_wakeup(60ULL * 1000000ULL); 
    // esp_deep_sleep_start();                            
}

void loop() {
    // All tasks execute inside setup() once on boot up. Loop stays idle.
    delay(1000); 
}