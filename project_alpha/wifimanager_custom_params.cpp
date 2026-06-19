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
#include "wifimanager_custom_params.h"

//callback notifying us of the need to save config
void saveConfigCallback()
{
    prtn("save config after successful WiFi connection");
    shouldSaveConfig = true;
}

void saveConfigParamsCallback()
{
    prtn("save config after hitting 'Save' from the '/params' page");
    shouldSaveConfig = true;
}

void addCustomBoardSettingsWifiManager()
{
    const char* checkbox_checked = "type='checkbox' checked class='wm-check-row'";
    const char* checkbox = "type='checkbox' class='wm-check-row'";

    /* Easiest way to convert a float to a char *. */
    snprintf(offset_temperature_celsius_dht11, 20, "%f", Config::Sensor::MeasurmentOffset::TEMPERATURE_CELSIUS_DHT11);

    /* Syntax:
    *
    * id/name,
    * placeholder/prompt,
    * default,
    * length,
    * html validation
    */
    custom_display_rotation = new WiFiManagerParameter(
        "r0",
        "Display rotation: {0,2}",
        itoa(Config::Display::ROTATION, display_rotation, 10),
        1,
        "type='number' min='0' max='2' step='2'"
    );
    custom_time_format  = new WiFiManagerParameter(
        "t0",
        "Time format",
        Config::DateTime::Format::TIME,
        31,
        "type='text' maxlength='31'"
    );
    custom_date_format = new WiFiManagerParameter(
      "d0",
        "Date format",
        Config::DateTime::Format::DATE,
        31,
        "type='text' maxlength='31'"
    );
    custom_timezone_posix_string = new WiFiManagerParameter(
        "t1",
        "POSIX timezone string",
        Config::DateTime::Tz::POSIX,
        50,
        "type='text' maxlength='50'"
    );
    custom_ds3231 = new WiFiManagerParameter(
        "b0",
        "DS3231 enabled",
        "T",
        1,
        Config::HardwareFeatures::DS3231 ? checkbox_checked : checkbox
    );
    custom_ssd1306 = new WiFiManagerParameter(
        "b1",
        "SSD1306 enabled",
        "T",
        1,
        Config::HardwareFeatures::SSD1306 ? checkbox_checked : checkbox
    );
    custom_dht11 = new WiFiManagerParameter(
        "b2",
        "DHT11 enabled",
        "T",
        1,
        Config::HardwareFeatures::DHT11 ? checkbox_checked : checkbox
    );
    custom_mq_2 = new WiFiManagerParameter(
        "b3",
        "MQ-2 enabled",
        "T",
        1,
        Config::HardwareFeatures::MQ_2 ? checkbox_checked : checkbox
    );
    custom_mq_135 = new WiFiManagerParameter(
        "b4",
        "MQ-135 enabled",
        "T",
        1,
        Config::HardwareFeatures::MQ_135 ? checkbox_checked : checkbox
    );
    custom_offset_temperature_celsius_dht11 = new WiFiManagerParameter(
        "o0",
        "DHT11 temperature celsius offset",
        offset_temperature_celsius_dht11,
        20,
        "type='number', step='0.1'"
    );
}

void defineCustomBoardSettingsWifiManager()
{
    const char* custom_html_head = 
        "<style>"
        ".wm-check-row { display: flex !important; flex-direction: row-reverse; justify-content: flex-end; align-items: center; margin-bottom: 15px; }"
        ".wm-check-row input { width: auto !important; margin: 0 10px 0 0 !important; cursor: pointer; transform: scale(1.2); }"
        ".wm-check-row label { cursor: pointer; order: 2; }"
        "</style>";
    wm.setCustomHeadElement(custom_html_head);

    wm.setSaveParamsCallback(saveConfigParamsCallback);
    wm.setSaveConfigCallback(saveConfigCallback);

    wm.addParameter(custom_display_rotation);
    wm.addParameter(custom_time_format);
    wm.addParameter(custom_date_format);
    wm.addParameter(custom_timezone_posix_string);

    /* HW features. */
    wm.addParameter(custom_ds3231);
    wm.addParameter(custom_ssd1306);
    wm.addParameter(custom_dht11);
    wm.addParameter(custom_mq_2);
    wm.addParameter(custom_mq_135);

    wm.addParameter(custom_offset_temperature_celsius_dht11);
}

void getCustomBoardSettingsWifiManager()
{
    strncpy(display_rotation, custom_display_rotation->getValue(), sizeof(display_rotation) - 1);
    strncpy(time_format, custom_time_format->getValue(), sizeof(time_format) - 1);
    strncpy(date_format, custom_date_format->getValue(), sizeof(date_format) - 1);
    strncpy(timezone_posix_string, custom_timezone_posix_string->getValue(), sizeof(timezone_posix_string) - 1);

    strncpy(ds3231, custom_ds3231->getValue(), sizeof(ds3231) - 1);
    strncpy(ssd1306, custom_ssd1306->getValue(), sizeof(ssd1306) - 1);
    strncpy(dht11, custom_dht11->getValue(), sizeof(dht11) - 1);
    strncpy(mq_2, custom_mq_2->getValue(), sizeof(mq_2) - 1);
    strncpy(mq_135, custom_mq_135->getValue(), sizeof(mq_135) - 1);

    strncpy(offset_temperature_celsius_dht11, custom_offset_temperature_celsius_dht11->getValue(), sizeof(offset_temperature_celsius_dht11) - 1);

    prtn("values passed from WifiManager: ");
    prt("custom_display_rotation: ");
    prtn(display_rotation);
    prt("custom_time_format: ");
    prtn(time_format);
    prt("timezone_posix_string: ");
    prtn(timezone_posix_string);
    prt("ds3231: ");
    prtn(ds3231);
    prt("ssd1306: ");
    prtn(ssd1306);
    prt("dht11: ");
    prtn(dht11);
    prt("mq-2: ");
    prtn(mq_2);
    prt("mq-135: ");
    prtn(mq_135);
    prt("offset_temperature_celsius_dht11: ");
    prtn(offset_temperature_celsius_dht11);
}

bool updateConfigFromJsonConfigFile(fs::FS &fs, const char * path, JsonDocument &doc)
{
    /* Populate values reading JSON from LittleFS to local constants. */
    /* attempt to load the file */
    if (readJson(fs, path, doc))
    {
        prtn("JSON file found, data deserialized");
        if (doc["display_rotation"].is<const char*>())
            Config::Display::ROTATION = doc["display_rotation"].as<int>();
        if (doc["time_format"].is<const char*>())
            strncpy(Config::DateTime::Format::TIME, doc["time_format"], sizeof(Config::DateTime::Format::TIME));
        if (doc["date_format"].is<const char*>())
            strncpy(Config::DateTime::Format::DATE, doc["date_format"], sizeof(Config::DateTime::Format::DATE));
        if (doc["timezone_posix_string"].is<const char*>())
            strncpy(Config::DateTime::Tz::POSIX, doc["timezone_posix_string"], sizeof(Config::DateTime::Tz::POSIX));
        if (doc["ds3231"].is<bool>())
            Config::HardwareFeatures::DS3231 = doc["ds3231"].as<bool>();
        if (doc["ssd1306"].is<bool>())
            Config::HardwareFeatures::SSD1306 = doc["ssd1306"].as<bool>();
        if (doc["dht11"].is<bool>())
            Config::HardwareFeatures::DHT11 = doc["dht11"].as<bool>();
        if (doc["mq_2"].is<bool>())
            Config::HardwareFeatures::MQ_2 = doc["mq_2"].as<bool>();
        if (doc["mq_135"].is<bool>())
            Config::HardwareFeatures::MQ_135 = doc["mq_135"].as<bool>();
        if (doc["offset_temperature_celsius_dht11"].is<float>())
            Config::Sensor::MeasurmentOffset::TEMPERATURE_CELSIUS_DHT11 = doc["offset_temperature_celsius_dht11"];

        return true;
    }
    else
    {
        prt("error: file ");
        prt(path);
        prtn(" not found or empty");
        return false;
    }
}

bool writeJsonConfigFileAndReloadConfiguration()
{
    configDoc.clear();
    configDoc["display_rotation"] = display_rotation;
    configDoc["time_format"] = time_format;
    configDoc["date_format"] = date_format;
    configDoc["timezone_posix_string"] = timezone_posix_string;
    configDoc["ds3231"] = ds3231[0] =='\0' ? false : true;
    configDoc["ssd1306"] = ssd1306[0] == '\0' ? false : true;
    configDoc["dht11"] = dht11[0] =='\0' ? false : true;
    configDoc["mq_2"] = mq_2[0] =='\0' ? false : true;
    configDoc["mq_135"] = mq_135[0] =='\0' ? false : true;
    configDoc["offset_temperature_celsius_dht11"] = offset_temperature_celsius_dht11;

    prtn("saving configuration...");
    if (writeJson(LittleFS, filePath, configDoc))
    {
        configDoc.clear();
        if (updateConfigFromJsonConfigFile(LittleFS, filePath, configDoc))
        {
            // Re-initialize display to re-set updated rotation.
            if (Config::HardwareFeatures::SSD1306)
            {
                ssd1306DeInit();
                ssd1306Init();
            }
            else
            {
                // We just need to turn off the display.
                if (ssd1306Init(false))
                    ssd1306DeInit();
            }

            prtn("configuration stored successfully");
            return true;
        }
        return false;
    }
    else
        return false;
}