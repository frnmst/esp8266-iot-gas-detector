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
#ifndef CONFIG_H
#define CONFIG_H

// 1 to enable serial print debug.
// Set this to 0 for production mode.
#define PRINT_DEBUG 0

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
        static const bool IGNORE_MISSING_WIFI_ON_MANUAL;
        static const bool MANUAL;
        static const unsigned long WIFIMANAGER_CONFIG_PORTAL_TIMEOUT_SECONDS;

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
            static const bool CLEAR_WIFI_SETTINGS;
        };
    };

    struct Display {
        static const int ROTATION;
        static const int WIDTH = 128;
        static const int HEIGHT = 64;
        static const bool PERIODIC_COLOR_INVERT;
    };

    struct Sensor {
        struct Setting {
            static const unsigned long READING_TIMEOUT_MS;

            // Timeout during the initial boot phase.
            static const unsigned long BOOT_READING_TIMEOUT_MS;

            // Total number of readings in memory (circular array).
            static constexpr int READINGS_BUFFER = 16;

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
            static const int BUZZER_SENSOR_ERROR_ITERATIONS;
            static const int BUZZER_SENSOR_ERROR_DELAY_MS;
        };
    };

    struct Pins {
        struct Digital {
            static const int BUZZER = 0;
            static const int BOARD_LED = 2;
            static const int DHT_11 = 12;
            static const int NOSE = 14;
        };
        struct Analog {
            static const int RTC = 999;
            static const int NOSE = A0;
        };
    };

    struct Triggers {
        static const float GAS_ALARM_VOLTS_THRESHOLD;
        static const int GAS_ALARM_TENDENCY_ITERATIONS;
        static const bool IGNORE_GAS_TENDENCY;
        static const bool IGNORE_GAS_VOLTAGE;
    };

    struct DateTime {
        static const char* NTP_SERVER;
        struct Tz {
            static const TimeChangeRule DST;
            static const TimeChangeRule STD;
        };
        struct Format {
            static const char* TIME;
            static const char* DATE;
        };
    };

    struct WarmingUpInfo {
        static const String TEXT[];
        static const int TEXT_LENGTH;
    };

    struct Other {
        static const bool BOOT_GAS_ALARM_SELF_TEST;
        static const char* GAS_ALARM_TEXT;
    };
};

#endif // CONFIG_H
