/* version 0.8.0
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
#include "config.h"

/* Hardware features. Set to false to use on minimal boards. */
const bool Config::HardwareFeatures::DS3231 = true;
const bool Config::HardwareFeatures::SSD1306 = true;
const bool Config::HardwareFeatures::DHT11 = true;
const bool Config::HardwareFeatures::MQ_2 = true;

/* Display rotation 
 *     0 = 0°
 *     1 = 90°
 *     2 = 180°
 *     3 = 270°
*/
const int Config::Display::ROTATION = 0;

// Fine tune calibration.
// Summer values: 0,0,0,0
const float Config::Sensor::MeasurmentOffset::TEMPERATURE_CELSIUS_DHT11 =  -0.5;
const float Config::Sensor::MeasurmentOffset::TEMPERATURE_CELSIUS_DS3231 = -1.5;
const float Config::Sensor::MeasurmentOffset::HUMIDITY_PERCENT = 0.0;
const float Config::Sensor::MeasurmentOffset::GAS_VOLTS = 0.0;

// Enable this if you want an airgap system.
const bool Config::Wifi::WORK_OFFLINE = true;

/* Time and date. */
// The number of seconds to compensate between compiler timestamp and first boot.
const long Config::DateTime::COMPILATION_SECONDS_OFFSET = 60;

const char* Config::DateTime::NTP_SERVER = "ntp1.inrim.it";
const char* Config::DateTime::Format::TIME = "%H:%M:%S";
/* ISO:     "%Y-%m-%d"
 * Italian: "%d/%m/%y"
 * US:      "%m/%d/%y"
 */
const char* Config::DateTime::Format::DATE = "%Y-%m-%d";

// Time in ms between readings.
const unsigned int Config::Sensor::Setting::READING_TIMEOUT_MS = 10000;

// Time in ms to assume the MQ-2 sensor is properly warmed up.
const int Config::Sensor::Setting::MQ_2_WARM_UP_MS = 60000;

// Time in ms between readings during the initial boot phase.
const unsigned int Config::Sensor::Setting::BOOT_READING_TIMEOUT_MS = 1200;

// Ser to true if you want to use the built-in LED to show the board is working.
const bool Config::Other::ENABLE_LED_HEARTBEAT = true;
