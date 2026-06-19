/* version 0.16.0
 *
 * MIT License
 *
 * Copyright (c) 2024-2026 Franco Masotti
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef ARDUINO_SECRETS_H
#define ARDUINO_SECRETS_H

#define HW_REVISION "default"

/* Run basic unit tests instead of the main sketch. */
// #define RUN_UNIT_TESTS 0

/* See pin triggers below to toggle this value.
 * Usually set to false in production mode.
 */
inline bool Config::Debug::PRINT = false;

inline char Config::DateTime::Format::TIME[32] = "%H:%M:%S";
/* ISO:     "%Y-%m-%d"
 * Italian: "%d/%m/%y"
 * US:      "%m/%d/%y"
 */
inline char Config::DateTime::Format::DATE[32] = "%Y-%m-%d";

/* Set a compatible POSIX timezone string. See the following list:
 * https://raw.githubusercontent.com/nayarsystems/posix_tz_db/refs/heads/master/zones.csv
 * Set value if for "Europe/Rome"
 */
inline char Config::DateTime::Tz::POSIX[50] = "CET-1CEST,M3.5.0,M10.5.0/3";

// Use exactly 3 NTP servers.
inline const char *Config::DateTime::NTP_SERVERS[] = {"ntp1.inrim.it", "pool.ntp.org", "time.nist.gov"};

/* Network */
// Set this to false if you want an airgap system.
inline const bool Config::Wifi::WORK_ONLINE = true;

inline bool Config::Wifi::Auto::RESET_WIFI_SETTINGS = false;
inline unsigned int Config::Wifi::WIFIMANAGER_CONFIG_PORTAL_TIMEOUT_SECONDS = 3600;

// true to see all the responses from the server on serial.
inline const bool Config::Network::FULL_DEBUG = false;
// true to see the request and a basic response from the server on serial.
inline const bool Config::Network::BASIC_DEBUG = false;

inline bool Config::Network::SEND_TO_API = false;
inline const char* Config::Network::HOST = "my.remote.server";
inline const int Config::Network::PORT = 443;
inline const bool Config::Network::USE_SSL = true;
inline const char* Config::Network::USER_AGENT = "ESP8266";

/* Example for Home Assistant */
inline const char* Config::Network::ENDPOINT = "/api/events";
inline const char* Config::Network::WEBHOOK_ID = "esp_unified_sensor_living_room";

// A friendly name for the board sent in the JSON.
inline const char* Config::Network::BOARD_ID = "living room";

// In Home Assistant go to:
// Profile -> Security -> Long-lived access tokens
// to generate a new token.
inline const char* Config::Network::AUTHORIZATION_BEARER_TOKEN = "d9b4872e-8c4a-49b3-982e-7ea2b3643143";
inline const int Config::Network::SEND_DATA_FREQUENCY_MS = 60000;

/* The hostname used to distinguish each board in OTA mode. */
inline const char* Config::OTA::Generic::HOSTNAME = "living-room";

/* MD5 hash for 'password'. */
inline const char *Config::OTA::Auth::PASSWORD_MD5_HASH = "5f4dcc3b5aa765d61d8327deb882cf99";

/* WiFi Network name and password used in the wizard
 * (in auto mode, these are the config access point credentials),
 * or the ones used to connect to a WiFi network manually.
 */
inline const char* Config::Wifi::SSID = "GasNose";
// Password must be at least 8 characters.
inline const char* Config::Wifi::PASSWORD = "gasnose_0000";

inline bool Config::Wifi::MANUAL = false;

/* Settings applied if Config::Wifi::MANUAL == true.
 * Set these if you don't want to use the WiFi wizard (not recommended).
 */
inline const IPAddress Config::Wifi::Manual::LOCAL_IP(192, 168, 0, 2);
inline const IPAddress Config::Wifi::Manual::GATEWAY(192, 168, 0, 1);
inline const IPAddress Config::Wifi::Manual::SUBNET(255, 255, 255, 0);
inline const IPAddress Config::Wifi::Manual::DNS(192, 168, 0, 1);

// Fine tune calibration.
inline float Config::Sensor::MeasurmentOffset::TEMPERATURE_CELSIUS_DHT11 =  -0.5;
inline float Config::Sensor::MeasurmentOffset::TEMPERATURE_CELSIUS_DS3231 = -1.5;
inline float Config::Sensor::MeasurmentOffset::HUMIDITY_PERCENT = 0.0;
inline float Config::Sensor::MeasurmentOffset::GAS_VOLTS = 0.0;

/* Hardware features. Set values to false to use on different hardware revisions. */
inline bool Config::HardwareFeatures::DS3231 = true;
inline bool Config::HardwareFeatures::SSD1306 = true;
inline bool Config::HardwareFeatures::DHT11 = true;
inline bool Config::HardwareFeatures::MQ_2 = true;
inline bool Config::HardwareFeatures::MQ_135 = false;

/* Display rotation 
 *     0 = 0°
 *     1 = 90°
 *     2 = 180°
 *     3 = 270°
*/
inline int Config::Display::ROTATION = 0;

// Time in ms to assume the MQ-2 sensor is properly warmed up.
inline const int Config::Sensor::Setting::MQ_2_WARM_UP_MS = 60000;

// If OSD debug is enabled, show it every DISPLAY_DEBUG_DATA_FREQUENCY_MS.
inline const unsigned int Config::Display::DISPLAY_DEBUG_DATA_FREQUENCY_MS = 60000;

// Set to true if you want to use the built-in LED to show the board is working.
inline const bool Config::Other::ENABLE_LED_HEARTBEAT = true;

/* Time adjustment used only with DS3231 in offline mode:
 *
 * Config::Wifi::WORK_ONLINE = false
 *
 * and
 *
 * Config::HardwareFeatures::DS3231 = true.
 *
 * Compensate for local to UTC seconds difference.
 *
 * e.g., for Europe/Rome:
 * On sketch compilation between Oct and Mar set -3600.
 * On sketch compilation between Apr and Feb set -7200.
 */
inline const long Config::DateTime::Tz::COMPUTER_NAIVE_TIME_TO_UTC_SECONDS_OFFSET = -7200;

/* Triggers must be sorted by dsc thresholdMs.
 *
 * What follows is the C struct defined in config.h.
 *
 * struct ButtonTrigger {
 *   ActionFunc action;
 *   bool needsWiFi;
 *   const char* name;
 *   const char* displayName;  // Use exactly 3 lines.
 *};
 */
const int triggerMsStep = 750;
inline ButtonTrigger buttonTriggers[] = {
    {
        noop,
        false,
        "ABORT",
        "ABORT\nTRIG\nMODE"
    },
    {
        credits,
        false,
        "CREDITS",
        "CREDITS\n\n"
    },
    {
        doOTA,
        true,
        "OTA",
        "OTA\nARDUINO\nIDE"
    },
    {
        togglePrintDebug,
        false,
        "TOGGLE_PRINT_DEBUG",
        "TOGGLE\nPRINT\nDEBUG"
    },
    {
        selfTest,
        false,
        "SELF_TEST",
        "SELF\nTEST\n"
    },
    {
        toggleShowCurrentTimeOnly,
        false,
        "TOGGLE_CURRENT_TIME_ONLY",
        "TOGGLE\nCURR\nTIME"
    },
    {
        toggleDisplay,
        false,
        "TOGGLE_DISPLAY",
        "TOGGLE\nDISPLAY\n"
    },
    {
        toggleConfigPortal,
        false,
        "TOGGLE_CONFIG_PORTAL",
        "TOGGLE\nCONFIG\nPORTAL"
    },
    {
        togglePomodoroTimer,
        false,
        "TOGGLE_POMODORO_TIMER",
        "TOGGLE\nPOMODORO\nTIMER"
    }
};

// Time in ms between readings.
inline const unsigned int Config::Sensor::Setting::READING_TIMEOUT_MS = 10000;

// Raise the alarm if the raw value increases after Config::Triggers::GAS_ALARM_TENDENCY_ITERATIONS.
inline const int Config::Triggers::GAS_ALARM_TENDENCY_ITERATIONS = 512;

// The number of seconds to compensate between compiler timestamp and first boot.
inline const long Config::DateTime::COMPILATION_SECONDS_OFFSET = 60;

inline const char* Config::Other::GAS_ALARM_TEXT = "GAS ALARM!";

#endif
