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
#ifndef CONFIG_H
#define CONFIG_H

#include "version.h"

struct Config {
    struct HardwareFeatures {
        static const bool DS3231;
        static const bool SSD1306;
        static const bool DHT11;
        static const bool MQ_2;
        static const bool MQ_135;
    };
    struct Network {
        static const bool SEND_TO_API;
        static const char* HOST;
        static const int PORT;
        static const bool USE_SSL;
        static const char* ENDPOINT;
        static const char* BOARD_ID;
        static const char* AUTHORIZATION_BEARER_TOKEN;
        static const char* WEBHOOK_ID;
        static const char* USER_AGENT;
        static const int SEND_DATA_FREQUENCY_MS;
        static const bool DEBUG;
    };

    struct OTA {
        struct Auth {
            static const char *PASSWORD_MD5_HASH;
        };
        struct Generic {
            static const char *HOSTNAME;
        };
    };

    struct Wifi {
        static const bool WORK_OFFLINE;
        static const bool IGNORE_MISSING_WIFI_ON_MANUAL = true;

        // true to setup WiFi manually without the wizard.
        static const bool MANUAL = false;
        static const unsigned int WIFIMANAGER_CONFIG_PORTAL_TIMEOUT_SECONDS = 180;

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
        static const unsigned int DISPLAY_TIMEOUT_MS = 500;
        static const unsigned int DISPLAY_DEBUG_DATA_FREQUENCY_MS;
    };

    struct Rtc {
        static const int unsigned RTC_GET_UPDATE_FREQUENCY_MS = 500;
        static const int unsigned RTC_SET_UPDATE_FREQUENCY_MS = 3600;
    };

    struct Sensor {
        struct Setting {
            static const unsigned int READING_TIMEOUT_MS;

            // Timeout during the initial boot phase.
            static const unsigned int BOOT_READING_TIMEOUT_MS;

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

    /* Board specific data. */
    #if defined(ESP8266)
        #include "boards/esp8266.h"
    #elif defined(ESP32)
        #include "boards/esp32.h"
    #else
        #error "unsupported arch!"
    #endif

    struct Triggers {
        static const float GAS_ALARM_VOLTS_THRESHOLD;
        static const int GAS_ALARM_TENDENCY_ITERATIONS;

        // If set to true, do not raise an alarm if the gas tendency passes GAS_ALARM_TENDENCY_ITERATIONS.
        static const bool IGNORE_GAS_TENDENCY = true;
        static const bool IGNORE_GAS_VOLTAGE = true;
    };

    struct DateTime {
        static const char* NTP_SERVERS[];
        static const long COMPILATION_SECONDS_OFFSET;
        struct Tz {
            static const char* POSIX;
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
        static const char* TEXT[];
        static const int TEXT_LENGTH;
    };

    struct Other {
        static const char* GAS_ALARM_TEXT;

        // Display extra debug only if main debug is enabled.
        static const bool DISPLAY_EXTRA_DEBUG_INFO = true;

        static const int DEBUG_SPEED = 115200;

        static const int BUILTIN_LED_MAX_PWM_VALUE = 255;
        static const bool ENABLE_LED_HEARTBEAT;
        static const int BUILTIN_LED_HEARTBEAT_INTERVAL = 50;
    };
};

// Higher value means less sensitive.
const float Config::Triggers::GAS_ALARM_VOLTS_THRESHOLD = 2.0;

// Raise the alarm if the raw value increases after Config::Triggers::GAS_ALARM_TENDENCY_ITERATIONS.
const int Config::Triggers::GAS_ALARM_TENDENCY_ITERATIONS = 512;

/* Warmup phase. */
const char* Config::Other::GAS_ALARM_TEXT = "GAS ALARM!";

#endif // CONFIG_H
