/* version 0.15.0
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
#ifndef WIFIMANAGER_CUSTOM_PARAMS_H
#define WIFIMANAGER_CUSTOM_PARAMS_H

#include "common.h"
#include "config.h"
#include "output.h"
#include "json_file.h"

extern bool shouldSaveConfig;

extern WiFiManager wm;

extern JsonDocument configDoc;
extern char display_rotation[2], time_format[32], date_format[32], timezone_posix_string[50];
extern char ds3231[2], ssd1306[2], dht11[2], mq_2[2], mq_135[2];
extern char offset_temperature_celsius_dht11[21];
extern const char *filePath;

extern WiFiManagerParameter *custom_display_rotation;
extern WiFiManagerParameter *custom_time_format;
extern WiFiManagerParameter *custom_date_format;
extern WiFiManagerParameter *custom_timezone_posix_string;
extern WiFiManagerParameter *custom_ds3231;
extern WiFiManagerParameter *custom_ssd1306;
extern WiFiManagerParameter *custom_dht11;
extern WiFiManagerParameter *custom_mq_2;
extern WiFiManagerParameter *custom_mq_135;
extern WiFiManagerParameter *custom_offset_temperature_celsius_dht11;

void saveConfigCallback();
void saveConfigParamsCallback();
void addCustomBoardSettingsWifiManager();
void defineCustomBoardSettingsWifiManager();
void getCustomBoardSettingsWifiManager();
bool updateConfigFromJsonConfigFile(fs::FS &fs, const char * path, JsonDocument &doc);
bool writeJsonConfigFileAndReloadConfiguration();

#endif