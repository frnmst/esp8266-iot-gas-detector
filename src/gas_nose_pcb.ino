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
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <DHT.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFiClientSecure.h>
#include <WiFiManager.h>
#include <TimeLib.h>
#include <Timezone.h>
#include "RTClib.h"

#include "config.h"
#include "version.h"

// Devices.
RTC_DS3231 rtc;
Adafruit_SSD1306 display = Adafruit_SSD1306(Config::Display::WIDTH, Config::Display::HEIGHT, &Wire);
DHT dht(Config::Pins::Digital::DHT_11, Config::Sensor::Setting::DHT_TYPE);

// Data structures.
WiFiManager wm;
Timezone localTZ(Config::DateTime::Tz::DST, Config::DateTime::Tz::STD);
float gasSensorReadings[Config::Sensor::Setting::READINGS_BUFFER];
float temperatureSensorReadings[Config::Sensor::Setting::READINGS_BUFFER];
float humiditySensorReadings[Config::Sensor::Setting::READINGS_BUFFER];
float averageGasReading, averageTemperatureReading, averageHumidityReading, gasTotal = 0.0, temperatureTotal = 0.0, humidityTotal = 0.0;
float averageGasReadingVolts;
char averageGasReadingNormalizedChar[32];
static int gasIndex = 0, temperatureIndex = 0, humidityIndex = 0;
int gasState, gasTendency = 0;
bool inverted = false, gasDetected = false;
unsigned long previousMillis = 0, previousMillisNetworkCheck;
WiFiClientSecure httpsClient;
// Create a UDP instance.
WiFiUDP udp;
// Create an NTPClient instance: always work in UTC.
NTPClient timeClient(udp, Config::DateTime::NTP_SERVER, 0);
int check_wifi = 1;

String iso8601Timestamp = "";

#if PRINT_DEBUG
    #define prt(x) Serial.print(x)
    #define prtn(x) Serial.println(x)
#else
    #define prt(x)
    #define prtn(x)
#endif

void beep(int repetitions = 2, bool invertColors = false)
{
    for (int i = 0; i < repetitions; i++)
    {
        digitalWrite(Config::Pins::Digital::BUZZER, HIGH);
        delay(Config::Sensor::Alarm::BUZZER_SENSOR_ERROR_DELAY_MS);
        digitalWrite(Config::Pins::Digital::BUZZER, LOW);
        delay(Config::Sensor::Alarm::BUZZER_SENSOR_ERROR_DELAY_MS);
        if (invertColors)
            invertDisplayColors();
    }
}

String epochToIso8601(unsigned long epoch)
{
    // Create a time structure.
    struct tm timeinfo;

    // Cast.
    time_t epochTime = (time_t) epoch;

     // Convert epoch to UTC time.
    gmtime_r(&epochTime, &timeinfo);

    // Format the time as ISO 8601.
    char buffer[30];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S %z", &timeinfo);

    return String(buffer);
}

String getJson()
{
    JsonDocument doc;
    String output;

    JsonObject board = doc["board"].to<JsonObject>();

    board["id"] = Config::Network::BOARD_ID;

    JsonObject sensors = doc["sensors"].to<JsonObject>();

    // Transform the vchar array to float.
    sensors["gas_concentration_volts"] = atof(averageGasReadingNormalizedChar);

    sensors["temperature_celsius"] = averageTemperatureReading;
    sensors["humidity_percent"] = averageHumidityReading;
    if (gasDetected)
        sensors["gas_state_bool"] = true;
    else
        sensors["gas_state_bool"] = false;

    JsonObject computations = doc["computations"].to<JsonObject>();
    computations["gas_tendency"] = gasTendency;
    computations["timestamp_utc"] = iso8601Timestamp;

    serializeJson(doc, output);

    return output;
}

void clearDisplay(int text_size = 2)
{
    display.clearDisplay();
    display.setTextSize(text_size);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
}

void printDisplay(String text, int timeout = 5000, bool center = false)
{
    int16_t x, y; // Variables to hold the position
    uint16_t textWidth, textHeight;

    // Transform the String to a const char* before calling the getTextBounds method.
    display.getTextBounds(text.c_str(), 0, 0, &x, &y, &textWidth, &textHeight);
    int16_t xPos = (Config::Display::WIDTH - textWidth) / 2; // Calculate x position
    int16_t yPos = (Config::Display::HEIGHT - textHeight) / 2; // Center vertically

    clearDisplay();

    prtn(text);
    if (center)
    {
        display.setCursor(xPos, yPos);
        display.print(text);
    }
    else
    {
        display.println(text);
    }
    display.display();

    delay(timeout);
}

// Function to read sensor value and update index and total
void updateSensorData(float sensor_value, float* sensorReadings, int& index, float& total, int& tendency)
{
    float prevTotal = total;
    total = total - sensorReadings[index];
    sensorReadings[index] = sensor_value;
    total = total + sensorReadings[index];

    // Move to the next position in the array.
    index = (index + 1) % Config::Sensor::Setting::READINGS_BUFFER;

    // Specific to the gas readings only.
    if (prevTotal > total)
        tendency += 1;
    else if (prevTotal == total)
        // Flatten if the reading does not change.
        if (tendency < 0)
            tendency = 0;
        else
            // Adjust to -10% the alarm value.
            tendency -= (int) (Config::Triggers::GAS_ALARM_TENDENCY_ITERATIONS * 0.10);
        else if (prevTotal < total)
            tendency -= 1;
}

bool connectManualWifi()
{
    // Connect to Wi-Fi with static IP.
    printDisplay("WiFi\nmanual\nconfig", 1000);
    WiFi.config(
        Config::Wifi::Manual::LOCAL_IP,
        Config::Wifi::Manual::GATEWAY,
        Config::Wifi::Manual::SUBNET,
        Config::Wifi::Manual::DNS
    );
    WiFi.begin(Config::Wifi::SSID, Config::Wifi::PASSWORD);

    int attempts = 0;

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        prtn("Connecting to WiFi...");
        if (Config::Wifi::IGNORE_MISSING_WIFI_ON_MANUAL && attempts >= 10)
        {
            beep(2);
            return false;
        }
        attempts++;
    }
    beep(1);
    return true;
}

bool connectAutoWifi()
{
    if (Config::Wifi::Auto::CLEAR_WIFI_SETTINGS)
        wm.resetSettings();

    wm.setConfigPortalTimeout(
        Config::Wifi::WIFIMANAGER_CONFIG_PORTAL_TIMEOUT_SECONDS
    );

    printDisplay("WiFi\nauto\nconfig\n", 1000);
    printDisplay("WiFi\naccess\npoint\nfor " + String(Config::Wifi::WIFIMANAGER_CONFIG_PORTAL_TIMEOUT_SECONDS) + "s", 2500);
    printDisplay(
        "NW: " + String(Config::Wifi::SSID)
        + "\nPW: " + String(Config::Wifi::PASSWORD)
    );

    // Config access point.
    bool wifi = wm.autoConnect(Config::Wifi::SSID, Config::Wifi::PASSWORD);
    if (wifi)
    {
        prtn("connected to WiFi");
        printDisplay("connected to WiFi", 1000);
        beep(1);
    }
    else
    {
        prtn("unable to connect to WiFi");
        printDisplay("unable to connect to WiFi");
        beep(2);
    }

    return wifi;
}

bool syncRTC()
{
    /* Return true if ok, false otherwise. */

    if (timeClient.update())
    {
        time_t localTime = localTZ.toLocal(timeClient.getEpochTime());
        // Set the RTC with the epoch time + TZ offset from NTP.
        rtc.adjust(DateTime(localTime));
        return true;
    }
    else
        return false;
}

String easterEgg()
{
    int r = random(0, Config::WarmingUpInfo::TEXT_LENGTH);
    return "Gas Nose Project\n\n" + Config::WarmingUpInfo::TEXT[r] + "\n\n";
}

bool sketchJustUploaded()
{
    // These values should never be changed.
    #define EEPROM_SIZE 1
    #define ADDR_HASH   0

    EEPROM.begin(EEPROM_SIZE);

    // Compute one‐byte fingerprint of this sketch (32 hex characters).
    String md5 = ESP.getSketchMD5();

    // Get the first 2 chars as `char` of the checksum, then transform them into base 16 int.
    uint8_t hash = strtol(md5.substring(0, 2).c_str(), nullptr, 16);
    uint8_t prev = EEPROM.read(ADDR_HASH);

    if (hash != prev)
    {
        EEPROM.write(ADDR_HASH, hash);
        EEPROM.commit();

        prtn("EEPROM flag not set, adjusting RTC from compile time");
        prt("sketch MD5 hash differs: ");
        prt(hash);
        prt(" vs ");
        prtn(prev);

        return true;
    }
    else
    {
        prtn("sketch not changed, no need to manually set the time.");
    }

    return false;
}

void setup()
{
    int i;

    if (PRINT_DEBUG)
    {
        Serial.begin(9600);

        // Give time to flush the output.
        delay(5000);
    }

    pinMode(Config::Pins::Digital::NOSE, INPUT);
    pinMode(Config::Pins::Digital::BUZZER, OUTPUT);

    beep(1);

    while (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        prtn(F("SSD1306 allocation failed"));
        delay(1000);
    }
    display.setRotation(Config::Display::ROTATION);

    while (!rtc.begin()) {
        prtn("RTC error");
        printDisplay("RTC error", 1000, true);
    }

    prt("ver. ");
    prtn(VERSION);
    printDisplay(String("ver.\n") + String(VERSION), 2500, false);

    if (rtc.lostPower() || sketchJustUploaded()) {
        // When time needs to be set on a new device, or after a power loss, the
        // following line sets the RTC to the date & time this sketch was compiled
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        // This line sets the RTC with an explicit date & time, for example to set
        // January 21, 2014 at 3am you would call:
        // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    }

    if (!Config::Wifi::WORK_OFFLINE)
    {
        printDisplay("WiFi init", 1000);
        bool wifi = Config::Wifi::MANUAL ? connectManualWifi() : connectAutoWifi();

        if (wifi)
            printDisplay("SSID: " + WiFi.SSID() + "\n" + "RSSI: " + WiFi.RSSI());
        else
            printDisplay("NO WiFi\nskipping...");
    }

    dht.begin();

    prtn("Warming up MQ-2 sensor");

    for (i = 0; i < Config::Sensor::Setting::MQ_2_WARM_UP_MS / 1000; i++)
    {
        prt("warming ");
        prt(i);
        prt(" of ");
        prtn(Config::Sensor::Setting::MQ_2_WARM_UP_MS / 1000);

        clearDisplay(1);
        display.println("Warming MQ-2 gas");
        display.println("sensor, please wait");
        display.print(i);
        display.print(" of ");
        display.println(Config::Sensor::Setting::MQ_2_WARM_UP_MS / 1000);
        display.println("seconds");
        display.println(easterEgg());
        display.display();
        delay(1000);
    }

    for (i = 0; i < Config::Sensor::Setting::READINGS_BUFFER; i++)
    {
        clearDisplay();
        display.println("sample ");
        display.println(i + 1);
        display.println("of ");
        display.println(Config::Sensor::Setting::READINGS_BUFFER);
        display.display();

        prt("sampling ");
        prt(i + 1);
        prt(" of ");
        prtn(Config::Sensor::Setting::READINGS_BUFFER);

        gasSensorReadings[i] = analogRead(Config::Pins::Analog::NOSE);
        temperatureSensorReadings[i] = (
            (
                dht.readTemperature()
                + Config::Sensor::MeasurmentOffset::TEMPERATURE_CELSIUS_DHT11
            )
            + (
                rtc.getTemperature()
                + Config::Sensor::MeasurmentOffset::TEMPERATURE_CELSIUS_DS3231
            )
        ) / 2;

        humiditySensorReadings[i] = dht.readHumidity() + Config::Sensor::MeasurmentOffset::HUMIDITY_PERCENT;
        gasTotal += gasSensorReadings[i];
        temperatureTotal += temperatureSensorReadings[i];
        humidityTotal += humiditySensorReadings[i];
        delay(Config::Sensor::Setting::BOOT_READING_TIMEOUT_MS);
    }

    averageGasReading = gasTotal / Config::Sensor::Setting::READINGS_BUFFER;
    averageTemperatureReading = temperatureTotal / Config::Sensor::Setting::READINGS_BUFFER;
    averageHumidityReading = humidityTotal / Config::Sensor::Setting::READINGS_BUFFER;

    if (!Config::Wifi::WORK_OFFLINE)
    {
        timeClient.begin();
        syncRTC();
    }

    // Avoid checking certificate auth.
    httpsClient.setInsecure();

    // This simulates what happens when the gas is detected.
    if (Config::Other::BOOT_GAS_ALARM_SELF_TEST)
    {
        printDisplay("SELF TEST", 1000, true);
        raiseGasAlarm();
        printDisplay("TEST END", 1000, true);
    }
}

void invertDisplayColors()
{
    inverted = !inverted;
    if (inverted)
        // Invert colors.
        display.invertDisplay(true);
    else
        display.invertDisplay(false);
}

String getCurrentTime(DateTime now, const char* format = Config::DateTime::Format::TIME)
{
    char buffer[16];
    struct tm timeinfo;

    timeinfo.tm_hour = now.hour();
    timeinfo.tm_min = now.minute();
    timeinfo.tm_sec = now.second();

    // Not considering daylight saving time: datetime provided to this function is naive.
    timeinfo.tm_isdst = 0;

    strftime(buffer, sizeof(buffer), format, &timeinfo);

    return String(buffer);
}

String getCurrentDate(DateTime now, const char* format = Config::DateTime::Format::DATE)
{
    char buffer[32];
    struct tm dateinfo;

    dateinfo.tm_year = now.year() - 1900; // Year since 1900.
    dateinfo.tm_mon = now.month() - 1;    // Month (0-11).
    dateinfo.tm_mday = now.day();         // Day of the month (1-31).

    // Not considering daylight saving time: datetime provided to this function is naive.
    dateinfo.tm_isdst = 0;

    strftime(buffer, sizeof(buffer),format, &dateinfo);

    return String(buffer);
}

void raiseGasAlarm()
{
    prtn("The gas is present");
    printDisplay(Config::Other::GAS_ALARM_TEXT, 0, true);
    beep(Config::Sensor::Alarm::BUZZER_SENSOR_ERROR_ITERATIONS, true);
    // Reset display colors.
    display.invertDisplay(false);
}

void loop()
{
    int dummy = 0;
    unsigned long currentMillis = millis();

    DateTime now = rtc.now();

    String dTime = getCurrentTime(now);
    String dDate = getCurrentDate(now);

    clearDisplay();
    display.println(dTime);
    display.println(dDate);
    display.print(averageTemperatureReading);
    display.println(" oC");
    display.print(averageHumidityReading);
    display.println(" %H");
    display.display();

    // Use non-blocking code.
    if (currentMillis - previousMillis >= Config::Sensor::Setting::READING_TIMEOUT_MS)
    {
        previousMillis = currentMillis;

        gasState = digitalRead(Config::Pins::Digital::NOSE);
        float gasConcentration = (float) analogRead(Config::Pins::Analog::NOSE) + Config::Sensor::MeasurmentOffset::GAS_VOLTS;
        float temperatureCelsiusDht11 = dht.readTemperature() + Config::Sensor::MeasurmentOffset::TEMPERATURE_CELSIUS_DHT11;
        float temperatureCelsiusDs3231 = rtc.getTemperature() + Config::Sensor::MeasurmentOffset::TEMPERATURE_CELSIUS_DS3231;
        float temperatureCelsius = (temperatureCelsiusDht11 + temperatureCelsiusDs3231) / 2;
        float humidityPercent = dht.readHumidity() + Config::Sensor::MeasurmentOffset::HUMIDITY_PERCENT;

        // Check if the DHT11 or DS3231 sensor have problems.
        if (isnan(temperatureCelsius) || isnan(humidityPercent))
        {
            // If there is a reading problem get the latest available value
            // which is presumably not NaN.
            prtn("Warning DHT11 or DS3231 is NaN");
            temperatureCelsius = temperatureSensorReadings[temperatureIndex];
            humidityPercent = humiditySensorReadings[humidityIndex];
            beep(Config::Sensor::Alarm::BUZZER_SENSOR_ERROR_ITERATIONS);
        }

        updateSensorData(gasConcentration, gasSensorReadings, gasIndex, gasTotal, gasTendency);
        averageGasReading = gasTotal / Config::Sensor::Setting::READINGS_BUFFER;
        updateSensorData(temperatureCelsius, temperatureSensorReadings, temperatureIndex, temperatureTotal, dummy);
        averageTemperatureReading = temperatureTotal / Config::Sensor::Setting::READINGS_BUFFER;
        updateSensorData(humidityPercent, humiditySensorReadings, humidityIndex, humidityTotal, dummy);
        averageHumidityReading = humidityTotal / Config::Sensor::Setting::READINGS_BUFFER;

        // Transform into volts. The sensor is on the 5V rail.
        averageGasReadingVolts = (averageGasReading / 1023) * 5;
        sprintf(averageGasReadingNormalizedChar, "%.6f", averageGasReadingVolts);

        iso8601Timestamp = epochToIso8601(now.unixtime());

        prt("MQ2 sensor avg V: ");
        prtn(averageGasReadingVolts);
        prt("avg gas reading normalized: ");
        prtn(averageGasReadingNormalizedChar);
        prt("avg temp: ");
        prtn(averageTemperatureReading);
        prt("avg humd: ");
        prtn(averageHumidityReading);
        prt("gas tendency: ");
        prtn(gasTendency);
        prt("ISO 8601: ");
        prtn(iso8601Timestamp);

        if (gasState == LOW
            || (!Config::Triggers::IGNORE_GAS_VOLTAGE && averageGasReadingVolts >= Config::Triggers::GAS_ALARM_VOLTS_THRESHOLD)
            || (!Config::Triggers::IGNORE_GAS_TENDENCY && gasTendency >= Config::Triggers::GAS_ALARM_TENDENCY_ITERATIONS)
        )
        {
            gasDetected = true;
            raiseGasAlarm();
        }
        else
        {
            gasDetected = false;
            prtn("The gas is NOT present");
        }
    }

    if (currentMillis - previousMillisNetworkCheck >= Config::Network::SEND_DATA_FREQUENCY_MS)
    {
        if (Config::Display::PERIODIC_COLOR_INVERT)
            invertDisplayColors();

        if (PRINT_DEBUG)
            printDisplay("SERIAL DBG", 1000, true);

        previousMillisNetworkCheck = currentMillis;

        // Avoid busy waiting if not connected.
        if (WiFi.status() == WL_CONNECTED)
        {
            // Update the time.
            syncRTC();

            if (Config::Network::SEND_TO_API)
            {
                prt("connecting to ");
                prtn(Config::Network::HOST);
                if (!httpsClient.connect(Config::Network::HOST, Config::Network::PORT))
                    prtn("connection failed");

                // Create the PUT request.
                String json_data = getJson();
                String request = String("PUT ") + Config::Network::ENDPOINT + " HTTP/1.1\r\n" +
                                "Host: " + Config::Network::HOST + "\r\n" +
                                "User-Agent: ESP-8266\r\n" +
                                "Accept: */*\r\n" +
                                "Content-Type: application/json\r\n" +
                                "Content-Length: " + json_data.length() + "\r\n" +
                                "Authorization: Bearer " + Config::Network::AUTHORIZATION_BEARER_TOKEN + "\r\n"
                                "Connection: close\r\n\r\n" +
                                json_data;

                // Send the request.
                httpsClient.print(request);

                if (Config::Network::DEBUG)
                {
                    prtn(request);
                    // Read the response.
                    while (httpsClient.connected() || httpsClient.available())
                    {
                        if (httpsClient.available())
                        {
                            String line = httpsClient.readStringUntil('\n');
                            prtn(line);
                        }
                    }
                }
            }
        }
        else
        {
            check_wifi++;
        }
    }

    if (!Config::Wifi::WORK_OFFLINE && check_wifi % 10 == 0 && WiFi.status() != WL_CONNECTED)
    {
        // This block is never called in offline mode.

        /* See also
         * https://github.com/tzapu/WiFiManager/issues/1588#issue-1670081323
         */
        wm.setConfigPortalTimeout(0);
        prtn("WiFi is disconnected");
        printDisplay("NO WiFi\nretrying...");
        WiFi.reconnect();
        clearDisplay();
        check_wifi = 1;
    }
}
