/* version 0.4.0
 *
 * MIT License
 *
 * Copyright (c) 2024-2025 Franco Masotti
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
#include "config.h"

/* Network */
// Define the constants for the Networkstruct
const bool Config::Network::SEND_TO_API = false;
const char* Config::Network::HOST = "my.remote.server";
const int Config::Network::PORT = 443;
const char* Config::Network::ENDPOINT = "/sensors/data";

// This name must be unique between all boards using this program.
const char* Config::Network::BOARD_ID = "living room";

// See the wsgi.py file for information on how to generate this token.
const char* Config::Network::AUTHORIZATION_BEARER_TOKEN = "d9b4872e-8c4a-49b3-982e-7ea2b3643143";
const int Config::Network::SEND_DATA_FREQUENCY_MS = 60000;

// true to see the response from the server on serial.
const bool Config::Network::DEBUG = true;

/* Time */
const char* Config::DateTime::NTP_SERVER = "ntp1.inrim.it";
const char* Config::DateTime::Format::TIME = "%H:%M:%S";
/* ISO:     "%Y-%m-%d"
 * Italian: "%d/%m/%y"
 * US:      "%m/%d/%y"
 */
const char* Config::DateTime::Format::DATE = "%Y-%m-%d";

// true to setup WiFi manually without the wizard.
const bool Config::Wifi::MANUAL = false;
/* Settings applied if Config::Wifi::MANUAL == true.
 * Set these if you don't want to use the WiFi wizard.
 */
const IPAddress Config::Wifi::Manual::LOCAL_IP(192, 168, 0, 2);
const IPAddress Config::Wifi::Manual::GATEWAY(192, 168, 0, 1);
const IPAddress Config::Wifi::Manual::SUBNET(255, 255, 255, 0);
const IPAddress Config::Wifi::Manual::DNS(192, 168, 0, 1);

/* WiFi Network name and password used in the wizard
 * (in auto mode, these are the config access point credentials),
 * or the ones used to connect to a WiFi network manually.
 */
const char* Config::Wifi::SSID = "GasNose";

// Password must be at least 8 characters.
const char* Config::Wifi::PASSWORD = "gasnose_0000";

const unsigned long Config::Wifi::WIFIMANAGER_CONFIG_PORTAL_TIMEOUT_SECONDS = 180;

/* Set to true if you need to wipe saved WiFi credentials from the ROM (WiFiManager),
 * then push the sketch, then set it back to false before pushing it again.
 * Config::Wifi::WORK_OFFLINE must be set to `false`.
 */
const bool Config::Wifi::Auto::CLEAR_WIFI_SETTINGS = false;

/* 0 = 0°
   1 = 90°
   2 = 180°
   3 = 270°
*/
const int Config::Display::ROTATION = 2;
const bool Config::Display::PERIODIC_COLOR_INVERT = false;

// Time in ms to assume the MQ-2 sensor is properly warmed up.
const int Config::Sensor::Setting::MQ_2_WARM_UP_MS = 60000;

// Time in ms between readings.
const unsigned long Config::Sensor::Setting::READING_TIMEOUT_MS = 10000;

// Time in ms between readings during the initial boot phase.
const unsigned long Config::Sensor::Setting::BOOT_READING_TIMEOUT_MS = 1200;

// Fine tune calibration.
const float Config::Sensor::MeasurmentOffset::TEMPERATURE_CELSIUS_DHT11 =  -3.1;
const float Config::Sensor::MeasurmentOffset::TEMPERATURE_CELSIUS_DS3231 = -3.1;
const float Config::Sensor::MeasurmentOffset::HUMIDITY_PERCENT = 0.0;
const float Config::Sensor::MeasurmentOffset::GAS_VOLTS = 0.0;
const int Config::Sensor::Alarm::BUZZER_SENSOR_ERROR_ITERATIONS = 10;
const int Config::Sensor::Alarm::BUZZER_SENSOR_ERROR_DELAY_MS = 500;

// Enable this if you want an airgap system.
const bool Config::Wifi::WORK_OFFLINE = false;

const bool Config::Wifi::IGNORE_MISSING_WIFI_ON_MANUAL = true;

// Higher value means less sensitive.
const float Config::Triggers::GAS_ALARM_VOLTS_THRESHOLD = 2.0;

// Raise the alarm if the raw value increases after Config::Triggers::GAS_ALARM_TENDENCY_ITERATIONS.
const int Config::Triggers::GAS_ALARM_TENDENCY_ITERATIONS = 512;

// If set to true, do not raise an alarm if the gas tendency passes GAS_ALARM_TENDENCY_ITERATIONS.
const bool Config::Triggers::IGNORE_GAS_TENDENCY = true;
const bool Config::Triggers::IGNORE_GAS_VOLTAGE = true;

/* Time adjustment
 * e.g., for Rome, Italy.
 */
// DST starts last Sunday in March.
const TimeChangeRule Config::DateTime::Tz::DST = {"DST", Last, Sun, Mar, 2, 120};
// STD starts last Sunday in October.
const TimeChangeRule Config::DateTime::Tz::STD = {"STD", Last, Sun, Oct, 3, 60};

/* These strings are displayed randomly during the MQ-5 warmup phase.
 * Easer egg!
 */
const String Config::WarmingUpInfo::TEXT[] = {
    "REST API",
    "Gas sensor",
    "2 temperature sensors",
    "Humidity sensor",
    "Time",
    "Date",
    "RTC",
    "DS3231",
    "DHT11",
    "SSD1306",
    "ESP8266",
    "Arduino",
    "MQ-2",
    "Buzzer",
    "WiFi",
    "Acoustic alarm",
    "Oled display",
    "ver. " + String(VERSION),
};
const int Config::WarmingUpInfo::TEXT_LENGTH = 18;

const bool Config::Other::BOOT_GAS_ALARM_SELF_TEST = false;

const char* Config::Other::GAS_ALARM_TEXT = "GAS ALARM!";
