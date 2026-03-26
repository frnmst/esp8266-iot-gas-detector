/* version 0.13.1
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

/* Time and date. */
// The number of seconds to compensate between compiler timestamp and first boot.
const long Config::DateTime::COMPILATION_SECONDS_OFFSET = 60;

// Time in ms between readings.
const unsigned int Config::Sensor::Setting::READING_TIMEOUT_MS = 10000;

const unsigned int Config::Display::DISPLAY_DEBUG_DATA_FREQUENCY_MS = 60000;

// Time in ms to assume the MQ-2 sensor is properly warmed up.
const int Config::Sensor::Setting::MQ_2_WARM_UP_MS = 60000;

// Time in ms between readings during the initial boot phase.
const unsigned int Config::Sensor::Setting::BOOT_READING_TIMEOUT_MS = 1200;

/* Settings applied if Config::Wifi::MANUAL == true.
 * Set these if you don't want to use the WiFi wizard (not recommended).
 */
const IPAddress Config::Wifi::Manual::LOCAL_IP(192, 168, 0, 2);
const IPAddress Config::Wifi::Manual::GATEWAY(192, 168, 0, 1);
const IPAddress Config::Wifi::Manual::SUBNET(255, 255, 255, 0);
const IPAddress Config::Wifi::Manual::DNS(192, 168, 0, 1);
