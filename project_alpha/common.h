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
#ifndef COMMON_H
#define COMMON_H

    /* Embedded libraries. */
    #include <Arduino.h>
    #include <LittleFS.h>
    #include <FS.h>
    #include <EEPROM.h>

    /* Network */
    #include <WiFiClient.h>
    #include <WiFiClientSecure.h>
    #if defined(ESP8266)
        #include <ESP8266WiFi.h>
    #elif defined(ESP32)
        #include <WiFi.h>
    #else
        #error "unsupported arch!"
    #endif

    /* Sensors. */
    #include <DHT.h>
    #include "RTClib.h"

    /* Output. */
    #include <Adafruit_SSD1306.h>
    #include <Adafruit_GFX.h>

    /* Utilities. */
    #include <ArduinoJson.h>
    #include <ArduinoOTA.h>
    #include <WiFiManager.h>
    #include <TimeLib.h>
    #include "uptime.h"
    #include <AUnit.h>

    /* Local stuff. */
    #include "macros.h"

#endif