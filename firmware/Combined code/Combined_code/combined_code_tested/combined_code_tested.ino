/*
Generated using cluade with the following prompt:

Using the arduino core libraries provided for the esp32-s3 model, create a HTTPS example to interface with github rest api using a personal access token.

*/

/**
 * @file    github_api_esp32s3.ino
 * @brief   HTTPS example: ESP32-S3 interfacing with the GitHub REST API
 *          using a Personal Access Token (PAT).
 *
 * Board    : ESP32-S3 (any variant – DevKitC, Feather S3, etc.)
 * Core     : arduino-esp32  >=  2.x  (WiFiClientSecure + HTTPClient built-in)
 *
 * Features demonstrated
 * ─────────────────────
 *  • TLS via WiFiClientSecure with GitHub's root CA pinned
 *  • Authenticated GET  – fetch authenticated user info  (/user)
 *  • Authenticated GET  – list repos (paginated)         (/user/repos)
 *  • Authenticated POST – create a repo issue            (/repos/:owner/:repo/issues)
 *  • JSON parsing with the bundled ArduinoJson library
 *
 * Required library (install via Arduino Library Manager)
 * ───────────────────────────────────────────────────────
 *  ArduinoJson  >=  6.21   (bblanchon/ArduinoJson)
 *
 * Quick-start
 * ───────────
 *  1. Fill in WIFI_SSID, WIFI_PASSWORD, GITHUB_TOKEN, GITHUB_OWNER,
 *     and GITHUB_REPO below.
 *  2. Flash to your ESP32-S3.
 *  3. Open Serial Monitor at 115200 baud.
 */

#include <Arduino.h>           // Core Arduino framework: Provides basic functions like setup(), loop(), delay(), and pin controls.
#include <WiFi.h>              // Wi-Fi Driver: Allows the ESP32 chip to turn on its radio, scan for networks, and connect to an access point.
#include <WiFiClientSecure.h>  // SSL/TLS Layer: Enables secure, encrypted network connections (necessary for interacting with 'https://' websites).
#include <HTTPClient.h>        // HTTP Handler: Provides easy-to-use methods like GET, POST, and PUT to request or send data across the web.
#include <ArduinoJson.h>       // JSON Parser: A powerful tool used to pack data into JSON format, or unpack and read JSON data received from web APIs.

// ─────────────────────────────────────────────
//  User configuration  ← edit these
// ─────────────────────────────────────────────
// Defines the Wi-Fi Network Name (SSID) as a read-only constant character array
static constexpr char WIFI_SSID[]     = "Matas ltu";
// Defines the Wi-Fi Password
static constexpr char WIFI_PASSWORD[] = "matas-ltu";

// GitHub Personal Access Token
//   Scopes needed:  repo  (for private repos / creating issues)
//   or just:        public_repo  (for public repos only)
// Stores your secret GitHub authentication token used to verify your identity to their servers
static constexpr char GITHUB_TOKEN[]  = "Add token here manual from whatsapp message";

// Owner (your GitHub username or org) and target repository
// Specifies the target GitHub account/organization name
static constexpr char GITHUB_OWNER[] = "matas-noreika";
// Specifies the target repository name where actions (like creating issues) will happen
static constexpr char GITHUB_REPO[]  = "STMP26-team13";

// ─────────────────────────────────────────────
//  GitHub API constants
// ─────────────────────────────────────────────
// The domain name for GitHub's Application Programming Interface (API)
static constexpr char GITHUB_HOST[]        = "api.github.com";
// Standard network port used worldwide for secure HTTPS traffic
static constexpr uint16_t HTTPS_PORT       = 443;
// The complete base web address for routing GitHub API requests
static constexpr char API_BASE[]           = "https://api.github.com";
// Identifies your ESP32 to GitHub so their servers know what kind of device/software is calling them
static constexpr char USER_AGENT[]         = "ESP32-S3-GitHub-Client/1.0";
// Tells GitHub we want the response data formatted specifically in the standard modern GitHub JSON format
static constexpr char ACCEPT_HEADER[]      = "application/vnd.github+json";
// Locks in a specific version of the GitHub API so future changes on GitHub don't break this code
static constexpr char API_VERSION_HEADER[] = "2022-11-28";   // X-GitHub-Api-Version

// ─────────────────────────────────────────────
//  Root CA for api.github.com
//  (DigiCert Global Root CA – valid until 2031)
// ─────────────────────────────────────────────
// This is a cryptographic Public Key Certificate used to verify that the website we 
// are connecting to is genuinely owned by GitHub, preventing "man-in-the-middle" hacking attacks.
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
// Reusable function that injects required metadata and security tokens into our web requests
void addGitHubHeaders(HTTPClient& http) {
    String auth = "Bearer ";
    auth += GITHUB_TOKEN;                 // Combines "Bearer " with your personal token string
    http.addHeader("Authorization", auth); // Injects authentication details so GitHub lets us in
    http.addHeader("User-Agent", USER_AGENT); // Identifies our hardware device setup
    http.addHeader("Accept", ACCEPT_HEADER); // Tells GitHub we expect a JSON payload back
    http.addHeader("X-GitHub-Api-Version", API_VERSION_HEADER); // Tells GitHub what version rules to process with
}

// ─────────────────────────────────────────────
//  Example 1 – GET /user
//  Returns the authenticated user's profile.
// ─────────────────────────────────────────────
void exampleGetAuthenticatedUser() {
    Serial.println("\n── Example 1: GET /user ──────────────────────");

    WiFiClientSecure secureClient;         // Creates an encrypted network client instance
    secureClient.setCACert(GITHUB_ROOT_CA); // Hands the DigiCert security certificate to the client
    secureClient.setTimeout(15);           // Disconnects if the server takes over 15 seconds to reply

    HTTPClient http;                       // Creates the high-level tool to manage web actions
    String url = String(API_BASE) + "/user"; // Builds final endpoint URL: "https://api.github.com/user"

    // Initializes connection; if it fails right out of the gate, exit early
    if (!http.begin(secureClient, url)) {
        Serial.println("[ERROR] http.begin() failed");
        return;
    }
    addGitHubHeaders(http);               // Applies our security tokens and formatting rules

    int httpCode = http.GET();             // Executes an HTTP GET request (asking GitHub for our data)
    Serial.printf("HTTP status: %d\n", httpCode); // Prints server status code (e.g., 200 = Success, 401 = Unauthorized)

    if (httpCode == HTTP_CODE_OK) {        // If status code is 200 OK
        String payload = http.getString(); // Downloads the actual response string (JSON text)

        JsonDocument doc;                  // Allocates RAM memory to structure our parsed JSON object
        DeserializationError err = deserializeJson(doc, payload); // Unpacks the plain text into an indexed JSON tree
        if (err) {                         // If parsing failed (malformed text, out of memory, etc.)
            Serial.printf("[ERROR] JSON parse: %s\n", err.c_str());
        } else {                           // Extract and print fields out of the parsed JSON doc structure
            Serial.printf("Login       : %s\n", doc["login"].as<const char*>());
            Serial.printf("Name        : %s\n", doc["name"].as<const char*>());
            Serial.printf("Public repos: %d\n", doc["public_repos"].as<int>());
            Serial.printf("Followers   : %d\n", doc["followers"].as<int>());
        }
    } else {                               // If code wasn't 200, display what error reason GitHub gave back
        Serial.printf("[ERROR] Body: %s\n", http.getString().c_str());
    }

    http.end();                            // Frees up client heap memory allocations and drops connection
}

// ─────────────────────────────────────────────
//  Example 2 – GET /user/repos
//  Lists the most recently updated repos (up to 5).
// ─────────────────────────────────────────────
void exampleListRepos() {
    Serial.println("\n── Example 2: GET /user/repos ────────────────");

    WiFiClientSecure secureClient;         // Creates our encrypted network socket
    secureClient.setCACert(GITHUB_ROOT_CA); // Pairs it with the security certificate
    secureClient.setTimeout(15);

    HTTPClient http;
    // Builds target URL with query parameters requesting exactly 5 items, sorted by most recently updated
    String url = String(API_BASE) + "/user/repos?per_page=5&sort=updated";

    if (!http.begin(secureClient, url)) {
        Serial.println("[ERROR] http.begin() failed");
        return;
    }
    addGitHubHeaders(http);

    int httpCode = http.GET();             // Submits request to download repos data
    Serial.printf("HTTP status: %d\n", httpCode);

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();

        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, payload);
        if (err) {
            Serial.printf("[ERROR] JSON parse: %s\n", err.c_str());
        } else {
            JsonArray repos = doc.as<JsonArray>(); // Casts document root as a JSON Array (list of objects)
            Serial.printf("Repos returned: %d\n", (int)repos.size());
            
            // Loop through each individual repository object found inside the JSON array list
            for (JsonObject repo : repos) {
                // Formats and prints out specific variables belonging to each specific repository entry
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
//  Creates a new issue in GITHUB_REPO.
//  HTTP 201 Created = success.
// ─────────────────────────────────────────────
void exampleCreateIssue() {
    Serial.println("\n── Example 3: POST /repos/.../issues ─────────");

    WiFiClientSecure secureClient;
    secureClient.setCACert(GITHUB_ROOT_CA);
    secureClient.setTimeout(15);
    
    // CRITICAL DETAIL: By calling setInsecure(), your ESP32 completely bypasses testing the validity 
    // of the GITHUB_ROOT_CA certificate. It will still encrypt data, but it won't verify who it is talking to.
    // Note: Since this is turned on, the "secureClient.setCACert(GITHUB_ROOT_CA)" above is practically ignored.
    secureClient.setInsecure();

    HTTPClient http;
    // Constructs dynamic URL pattern mapping targeting your user profile and repository name specifically
    String url = String(API_BASE)
               + "/repos/" + GITHUB_OWNER
               + "/" + GITHUB_REPO
               + "/issues";

    if (!http.begin(secureClient, url)) {
        Serial.println("[ERROR] http.begin() failed");
        return;
    }
    addGitHubHeaders(http);
    // Explicitly alerts GitHub that we are actively attaching data formatted as JSON inside the request body
    http.addHeader("Content-Type", "application/json");

    // Build JSON body structure natively inside your microcontroller's memory
    JsonDocument body;
    body["title"] = "Test issue from ESP32-S3"; // Sets string value for key "title"
    body["body"]  = "This issue was created automatically by an ESP32-S3 "
                    "running the arduino-esp32 core over HTTPS."; // Sets string description body
    body["labels"].add("arduino");            // Pushes an item into a data sub-array under the "labels" key
    body["labels"].add("esp32");              // Pushes secondary item into data array

    String bodyStr;
    serializeJson(body, bodyStr);             // flattens/packs JSON format tree into one single flat text line
    Serial.printf("POST body: %s\n", bodyStr.c_str());

    int httpCode = http.POST(bodyStr);         // Sends an HTTP POST command pushing the text body up to the server
    Serial.printf("HTTP status: %d\n", httpCode);   // GitHub returns a code '201' if a data asset was successfully generated

    if (httpCode == HTTP_CODE_CREATED) {       // Checks specifically for code 201 (Created)
        String payload = http.getString();
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, payload);
        if (!err) {
            // Extracts tracking metadata created natively by GitHub back to us
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
    WiFi.mode(WIFI_STA);                  // Configures the ESP32 chip to run exclusively as a client Station (STA)
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // Passes router credentials over to the hardware modem

    constexpr uint32_t TIMEOUT_MS = 20000; // Limits connection attempt duration strictly to 20 seconds
    uint32_t start = millis();             // Tracks current uptime snapshot stamp in milliseconds
    
    // Block system progression and execution flow until connection status flag registers confirmation
    while (WiFi.status() != WL_CONNECTED) {
        // Safe check loop: If overall processing duration crosses your 20-second threshold...
        if (millis() - start > TIMEOUT_MS) {
            Serial.println("\n[ERROR] WiFi timeout – rebooting.");
            ESP.restart();                 // Triggers physical hardware system reboot reset on the microprocessor
        }
        delay(500);                        // Pauses runtime execution for half a second to alleviate busy-looping
        Serial.print('.');                 // Prints loading dots to status log terminal output
    }
    // Confirms success and grabs the dynamic IP address provided to your hardware chip by your home router
    Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());
}

// ─────────────────────────────────────────────
//  Arduino entry points
// ─────────────────────────────────────────────
void setup() {
    Serial.begin(115200);                  // Opens up serial communication lane streaming out at 115200 baud rate
    delay(1000);                           // Short structural execution buffer delay allowing serial bus connection stability
    Serial.println("\n=== ESP32-S3 GitHub REST API Demo ===");

    connectWiFi();                         // Calls your custom Wi-Fi network connection loop function above

    // Examples execution queue control layer:
    // exampleGetAuthenticatedUser();      // (Disabled) GET  /user
    // exampleListRepos();                 // (Disabled) GET  /user/repos
    exampleCreateIssue();                  // (Enabled) Runs the POST sequence creating an actual tracking issue ticket

    Serial.println("\n=== All examples complete. ===");
    Serial.println("Uncomment deep-sleep below to wake periodically.");

    // Optional 60-second deep sleep then repeat
    // esp_sleep_enable_timer_wakeup(60ULL * 1000000ULL); // Programs wake-up alarm exactly 60 seconds out
    // esp_deep_sleep_start();                            // Turns off hardware processors completely to maximize battery life
}

void loop() {
    // All work is completed entirely inside setup() block once on boot up. 
    // This loop stays asleep doing nothing except recycling empty delay clocks.
    delay(1000); 
}