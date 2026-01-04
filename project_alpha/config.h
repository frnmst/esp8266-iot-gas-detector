/* version 0.7.1
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
#ifndef CONFIG_H
#define CONFIG_H

// Set to 1 to enable serial print debug, 0 for production mode.
#define PRINT_DEBUG 1

#include "version.h"

struct Config {
    struct Network {
        static const bool SEND_TO_API;
        static const char* HOST;
        static const int PORT;
        static const char* ENDPOINT;
        static const char* BOARD_ID;
        static const char* AUTHORIZATION_BEARER_TOKEN;
        static const int SEND_DATA_FREQUENCY_MS;
        static const bool DEBUG;
    };

    struct Wifi {
        static const bool WORK_OFFLINE;
        static const bool IGNORE_MISSING_WIFI_ON_MANUAL = true;

        // true to setup WiFi manually without the wizard.
        static const bool MANUAL = false;
        static const unsigned long WIFIMANAGER_CONFIG_PORTAL_TIMEOUT_SECONDS = 180;

        // In auto mode, these are the config access point credentials.
        static const char* SSID;
        static const char* PASSWORD;

        struct Manual {
            static const IPAddress LOCAL_IP;
            static const IPAddress GATEWAY;
            static const IPAddress SUBNET;
            static const IPAddress DNS;
        };

        struct Auto {
            /* Set to true if you need to wipe saved WiFi credentials from the ROM (WiFiManager),
            * then push the sketch, then set it back to false before pushing it again.
            * Config::Wifi::WORK_OFFLINE must be set to `false`.
            */
            static const bool CLEAR_WIFI_SETTINGS = false;
        };
    };

    struct Display {
        static const int ROTATION;
        static const int WIDTH = 128;
        static const int HEIGHT = 64;
        static const bool PERIODIC_COLOR_INVERT = false;
    };

    struct Sensor {
        struct Setting {
            static const unsigned long READING_TIMEOUT_MS;

            // Timeout during the initial boot phase.
            static const unsigned long BOOT_READING_TIMEOUT_MS;

            // Total number of readings in memory (circular array).
            static constexpr int READINGS_BUFFER = 8;

            static const int DHT_TYPE = DHT11;
            static const int MQ_2_WARM_UP_MS;
        };

        struct MeasurmentOffset {
            static const float TEMPERATURE_CELSIUS_DHT11;
            static const float TEMPERATURE_CELSIUS_DS3231;
            static const float HUMIDITY_PERCENT;
            static const float GAS_VOLTS;
        };

        struct Alarm {
            static const int BUZZER_SENSOR_ERROR_ITERATIONS = 10;
            static const int BUZZER_SENSOR_ERROR_DELAY_MS = 500;
        };
    };

    struct Pins {
        struct Digital {
            static const int BUZZER = 0;
            static const int BOARD_LED = 2;
            static const int DHT_11 = 12;
            static const int NOSE = 14;
            static const int LED = LED_BUILTIN;
        };
        struct Analog {
            static const int RTC = 999;
            static const int NOSE = A0;
        };
    };

    struct Triggers {
        static const float GAS_ALARM_VOLTS_THRESHOLD;
        static const int GAS_ALARM_TENDENCY_ITERATIONS;

        // If set to true, do not raise an alarm if the gas tendency passes GAS_ALARM_TENDENCY_ITERATIONS.
        static const bool IGNORE_GAS_TENDENCY = true;
        static const bool IGNORE_GAS_VOLTAGE = true;
    };

    struct DateTime {
        static const char* NTP_SERVER;
        static const long COMPILATION_SECONDS_OFFSET;
        struct Tz {
            static const TimeChangeRule DST;
            static const TimeChangeRule STD;
            static const long COMPUTER_NAIVE_TIME_TO_UTC_SECONDS_OFFSET;
        };
        struct Format {
            static const char* TIME;
            static const char* DATE;
        };
    };

    struct WarmingUpInfo {
        /* These strings are displayed randomly during the MQ-5 warmup phase.
         * Easer egg!
         */
        static const String TEXT[];
        static const int TEXT_LENGTH = 18;
    };

    struct Other {
        static const bool BOOT_GAS_ALARM_SELF_TEST = false;
        static const char* GAS_ALARM_TEXT;

        // Display extra debug only if main debug is enabled.
        static const bool DISPLAY_EXTRA_DEBUG_INFO = true;

        static const int DEBUG_SPEED = 9600;

        static const int BUILTIN_LED_MAX_PWM_VALUE = 255;
        static const bool ENABLE_LED_HEARTBEAT;
    };
};

/* Time adjustment
 * e.g., for Rome, Italy.
 */
// On compilation between Oct and Mar set -3600. On compilation between Apr and Feb set -7200.
const long Config::DateTime::Tz::COMPUTER_NAIVE_TIME_TO_UTC_SECONDS_OFFSET = -3600;

// DST starts last Sunday in March.
const TimeChangeRule Config::DateTime::Tz::DST = {"CEST", Last, Sun, Mar, 2, 120};

// STD starts last Sunday in October.
const TimeChangeRule Config::DateTime::Tz::STD = {"CET", Last, Sun, Oct, 3, 60};

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

// Higher value means less sensitive.
const float Config::Triggers::GAS_ALARM_VOLTS_THRESHOLD = 2.0;

// Raise the alarm if the raw value increases after Config::Triggers::GAS_ALARM_TENDENCY_ITERATIONS.
const int Config::Triggers::GAS_ALARM_TENDENCY_ITERATIONS = 512;

/* Warmup phase. */
const char* Config::Other::GAS_ALARM_TEXT = "GAS ALARM!";
const String Config::WarmingUpInfo::TEXT [] = {
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

#endif // CONFIG_H
