/* version 0.14.0
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

// Set to 1 to enable serial print debug, 0 for production mode.
#define PRINT_DEBUG 1

#define HW_REVISION "default"

const char* Config::DateTime::Format::TIME = "%H:%M:%S";
/* ISO:     "%Y-%m-%d"
 * Italian: "%d/%m/%y"
 * US:      "%m/%d/%y"
 */
const char* Config::DateTime::Format::DATE = "%Y-%m-%d";

/* Network */
// Enable this if you want an airgap system.
const bool Config::Wifi::WORK_OFFLINE = false;

// true to see the response from the server on serial.
const bool Config::Network::DEBUG = false;

const bool Config::Network::SEND_TO_API = false;
const char* Config::Network::HOST = "my.remote.server";
const int Config::Network::PORT = 443;
const bool Config::Network::USE_SSL = true;
const char* Config::Network::USER_AGENT = "ESP8266";

/* Example for Home assistant */
const char* Config::Network::ENDPOINT = "/api/events";
const char* Config::Network::WEBHOOK_ID = "esp_unified_sensor_living_room";

// A friendly name for the board sent in the JSON.
const char* Config::Network::BOARD_ID = "living room";

// In Home Assistant go to:
// Profile -> Security -> Long-lived access tokens
// to generate a new token.
const char* Config::Network::AUTHORIZATION_BEARER_TOKEN = "d9b4872e-8c4a-49b3-982e-7ea2b3643143";
const int Config::Network::SEND_DATA_FREQUENCY_MS = 60000;

/* The hostname used to distinguish each board in OTA mode. */
const char* Config::OTA::Generic::HOSTNAME = "living-room";

/* MD5 hash for 'password'. */
const char *Config::OTA::Auth::PASSWORD_MD5_HASH = "5f4dcc3b5aa765d61d8327deb882cf99";

/* WiFi Network name and password used in the wizard
 * (in auto mode, these are the config access point credentials),
 * or the ones used to connect to a WiFi network manually.
 */
const char* Config::Wifi::SSID = "GasNose";
// Password must be at least 8 characters.
const char* Config::Wifi::PASSWORD = "gasnose_0000";

// Fine tune calibration.
const float Config::Sensor::MeasurmentOffset::TEMPERATURE_CELSIUS_DHT11 =  -0.5;
const float Config::Sensor::MeasurmentOffset::TEMPERATURE_CELSIUS_DS3231 = -1.5;
const float Config::Sensor::MeasurmentOffset::HUMIDITY_PERCENT = 0.0;
const float Config::Sensor::MeasurmentOffset::GAS_VOLTS = 0.0;

/* Hardware features. Set to false to use on different hardware revisions. */
const bool Config::HardwareFeatures::DS3231 = true;
const bool Config::HardwareFeatures::SSD1306 = true;
const bool Config::HardwareFeatures::DHT11 = true;
const bool Config::HardwareFeatures::MQ_2 = true;
const bool Config::HardwareFeatures::MQ_135 = false;

const char* Config::WarmingUpInfo::TEXT [] = {
    "REST API",
    "Time",
    "Date",
    "RTC",
    "DS3231 (w/ temp sensor)",
    "DHT11",
    "SSD1306",
    "ESP8266",
    "MQ-2",
    "MQ-135",
    "Buzzer",
    "WiFi",
    "frnmst",
    "https://codeberg.org/frnmst/esp8266-iot-gas-detector",
    ("ver. %s", VERSION),
};

/* Number of elements in the array pointer. */
const int Config::WarmingUpInfo::TEXT_LENGTH = 15;

/* Display rotation 
 *     0 = 0°
 *     1 = 90°
 *     2 = 180°
 *     3 = 270°
*/
const int Config::Display::ROTATION = 0;

// Set to true if you want to use the built-in LED to show the board is working.
const bool Config::Other::ENABLE_LED_HEARTBEAT = true;

/* Set a compatible POSIX timezone string. See the following list:
 * https://raw.githubusercontent.com/nayarsystems/posix_tz_db/refs/heads/master/zones.csv
 * Set value if for "Europe/Rome"
 */
const char* Config::DateTime::Tz::POSIX = "CET-1CEST,M3.5.0,M10.5.0/3";

// Use exactly 3 NTP servers.
const char* Config::DateTime::NTP_SERVERS[] = {"ntp1.inrim.it", "pool.ntp.org", "time.nist.gov"};

/* Time adjustment used only with DS3231 in offline mode:
 *
 * Config::Wifi::WORK_OFFLINE = true
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
const long Config::DateTime::Tz::COMPUTER_NAIVE_TIME_TO_UTC_SECONDS_OFFSET = -7200;
#endif
