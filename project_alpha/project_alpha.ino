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

#include <Arduino.h>
#include <DHT.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#if defined(ESP8266)
    #include <ESP8266WiFi.h>
#elif defined(ESP32)
    #include <WiFi.h>
#else
    #error "unsupported arch!"
#endif
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiManager.h>
#include <TimeLib.h>
#include "RTClib.h"
#include "uptime.h"

#include "config.h"
#include "arduino_secrets.h"
#include "version.h"

// Devices.
RTC_DS3231 rtc;
Adafruit_SSD1306 display = Adafruit_SSD1306(Config::Display::WIDTH, Config::Display::HEIGHT, &Wire);
DHT dht(Config::Pins::Digital::DHT_11, Config::Sensor::Setting::DHT_TYPE);

// Data structures.
WiFiManager wm;
float gasSensorReadings[Config::Sensor::Setting::READINGS_BUFFER];
float temperatureSensorReadings[Config::Sensor::Setting::READINGS_BUFFER];
float humiditySensorReadings[Config::Sensor::Setting::READINGS_BUFFER];
float averageGasReading, averageTemperatureReading, averageHumidityReading, gasTotal = 0.0, temperatureTotal = 0.0, humidityTotal = 0.0;
float averageGasReadingVolts;
char averageGasReadingNormalizedChar[32];
static int gasIndex = 0, temperatureIndex = 0, humidityIndex = 0;
int gasState, gasTendency = 0;
char temperatureRelativeTendency = 0, humidityRelativeTendency = 0;
bool inverted = false, gasDetected = false;
unsigned long previousMillis = 0, previousMillisNetworkCheck = 0, previousMillisBuiltinLedHeartbeat = 0, previousMillisTimestampUpdate = 0, previousMillisDisplayData = 0, previousMillisDisplayDebugData = 0, previousMillisSetUpdateRTC = 0;
int checkWifi = 1, printDebugCounter = 0;
int builtinLedBrightness = 0, builtinLedFadeAmount = 5;
const long builtinLedHeartbeatInterval = 50;
char dTime[16], dDate[32];
DateTime dTimeNow = DateTime();
WiFiClient* apiClient = nullptr;

char iso8601Timestamp[32];

/* Button action triggers. */
typedef bool (*ActionFunc)();

/* Triggers must be sorted by dsc thresholdMs. */
struct Trigger {
    ActionFunc action;
    unsigned long thresholdMs;
    bool needsWiFi;
    const char* name;
};

/* Function prototype for triggers. */
bool selfTest();
bool doOTA();

Trigger triggers[] = {
    {
        doOTA,
        2000,
        true,
        "OTA"
    },
    {
        selfTest,
        500,
        false,
        "SELF_TEST"
    }
};
const int numTriggers = sizeof(triggers) / sizeof(triggers[0]);

#if PRINT_DEBUG
    #define prt(x) Serial.print(x)
    #define prtn(x) Serial.println(x)
#else
    #define prt(x)
    #define prtn(x)
#endif

WiFiClient* getHttpClient()
{
    if (Config::Network::USE_SSL)
    {
        WiFiClientSecure* sClient = new WiFiClientSecure();

        // Avoid checking certificate auth.
        sClient->setInsecure();
        return sClient;
    }
    else
        return new WiFiClient();
}

void signal(int repetitions = 2, bool invertColors = false, int lengthMs = Config::Sensor::Alarm::BUZZER_SENSOR_ERROR_DELAY_MS, bool buzzer = true)
{
    for (int i = 0; i < repetitions; i++)
    {
        if (buzzer)
            digitalWrite(Config::Pins::Digital::BUZZER, HIGH);

        digitalWrite(Config::Pins::Digital::LED, LOW);  // Turn the LED on (Note that LOW is the voltage level but actually the LED is on; this is because it is active low on the ESP-01)
        delay(lengthMs);
        if (buzzer)
            digitalWrite(Config::Pins::Digital::BUZZER, LOW);

        digitalWrite(Config::Pins::Digital::LED, HIGH);  // Turn the LED off by making the voltage HIGH

        if (i < repetitions - 1)
            delay(lengthMs);

        if (Config::HardwareFeatures::SSD1306 && invertColors)
            invertDisplayColors();
    }
}

void buildJson(JsonDocument &doc)
{
    uptime::calculateUptime();

    JsonObject board = doc["board"].to<JsonObject>();

    board["id"] = Config::Network::BOARD_ID;

    /* Get the total number of seconds. */
    board["uptime_seconds"] = (
        (uptime::getDays() * 86400)
        + (uptime::getHours() * 3600)
        + (uptime::getMinutes() * 60)
        + uptime::getSeconds()
    );
    board["debug"] = bool(PRINT_DEBUG);
    board["ip_address"] = WiFi.localIP().toString();

    JsonObject hardwareFeatures = board["hardware_features"].to<JsonObject>();
    hardwareFeatures["DS3231"] = Config::HardwareFeatures::DS3231;
    hardwareFeatures["SSD1306"] = Config::HardwareFeatures::SSD1306;
    hardwareFeatures["DHT11"] = Config::HardwareFeatures::DHT11;
    hardwareFeatures["MQ_2"] = Config::HardwareFeatures::MQ_2;
    hardwareFeatures["MQ_135"] = Config::HardwareFeatures::MQ_135;

    JsonObject version = doc["version"].to<JsonObject>();
    version["sw"] = VERSION;
    version["hw"] = HW_REVISION;

    JsonObject sensors = doc["sensors"].to<JsonObject>();

    // Transform the vchar array to float.
    sensors["gas_concentration_volts"] = atof(averageGasReadingNormalizedChar);

    sensors["temperature_celsius"] = averageTemperatureReading;
    sensors["humidity_percent"] = averageHumidityReading;

    if (gasDetected)
        sensors["gas_state_bool"] = true;
    else
        sensors["gas_state_bool"] = false;

    sensors["wifi_rssi"] = WiFi.RSSI();
    
    JsonObject computations = doc["computations"].to<JsonObject>();
    computations["gas_tendency"] = gasTendency;
    computations["timestamp_utc"] = iso8601Timestamp;
}

void sendJSONRequest(Print &p, JsonDocument &doc)
{
    // Create the POST request.
    p.print("POST ");
    p.print(Config::Network::ENDPOINT);
    p.print("/");
    p.print(Config::Network::WEBHOOK_ID);
    p.println(" HTTP/1.1");

    p.print("Host: ");
    p.println(Config::Network::HOST);

    p.print("User-Agent: ");
    p.println(Config::Network::USER_AGENT);

    p.println("Accept: */*");

    p.println("Content-Type: application/json");

    p.print("Content-Length: ");
    p.println(measureJson(doc));

    p.print("Authorization: Bearer ");
    p.println(Config::Network::AUTHORIZATION_BEARER_TOKEN);

    p.println("Connection: close");

    /* Required empty line before body. */
    p.println();

    serializeJson(doc, p);
}

void clearDisplay(int text_size = 2)
{
    display.clearDisplay();
    display.setTextSize(text_size);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
}

void printDisplay(const char *text, int timeout = 5000, bool center = false, int textSize = 2)
{
    // Variables to hold the position.
    int16_t x, y;
    uint16_t textWidth, textHeight;

    display.getTextBounds(text, 0, 0, &x, &y, &textWidth, &textHeight);
    int16_t xPos = (Config::Display::WIDTH - textWidth) / 2; // Calculate x position
    int16_t yPos = (Config::Display::HEIGHT - textHeight) / 2; // Center vertically

    clearDisplay(textSize);

    prt("<OSD>");
    prt(text);
    prtn("</OSD>");
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


float getHalfRollingAverage(float* sensorReadings, int& index, bool oldData)
{
    bool done = false;
    int j = index, itera = 0, ii;
    float total = 0, avg = 0;

    while (!done)
    {
        if (oldData)
        {
            ii = (j + 1) % Config::Sensor::Setting::READINGS_BUFFER;
            j++;
        }
        else
        {
            ii = j % Config::Sensor::Setting::READINGS_BUFFER;
            j--;
        }

        total += sensorReadings[ii];
        itera += 1;
        if (itera >= floor(Config::Sensor::Setting::READINGS_BUFFER / 2))
        {
            done = true;
            avg = total / itera;
        }
    }

    return avg;
}

// Function to read sensor value and update index and total.
void updateSensorData(float sensor_value, float* sensorReadings, int& index, float& total, int& tendency, char& relativeTendency)
{
    float prevTotal = total;
    total = total - sensorReadings[index];
    sensorReadings[index] = sensor_value;
    total = total + sensorReadings[index];

    /* Old. */
    float oldAvg = getHalfRollingAverage(sensorReadings, index, true);
    /* New. */
    float newAvg = getHalfRollingAverage(sensorReadings, index, false);

    /* See
     * https://learn.adafruit.com/adafruit-gfx-graphics-library/graphics-primitives
     * for char table.
     */
    if (newAvg > oldAvg)
        /* Higher. */
        relativeTendency = (char)24;
    else if (newAvg < oldAvg)
        /* Lower. */
        relativeTendency = (char)25;
    else
        /* Equal. */
        relativeTendency = (char)29;

    // Move to the next position in the array.
    index = (index + 1) % Config::Sensor::Setting::READINGS_BUFFER;

    // Specific to the gas readings only.
    if (prevTotal > total)
        tendency += 1;
    else if (prevTotal == total)
    {
        // Flatten if the reading does not change.
        if (tendency < 0)
            tendency = 0;
        else
            // Adjust to -10% the alarm value.
            tendency -= (int) (Config::Triggers::GAS_ALARM_TENDENCY_ITERATIONS * 0.10);
    }
    else if (prevTotal < total)
        tendency -= 1;
}

bool connectManualWifi()
{
    // Connect to Wi-Fi with static IP.
    if (Config::HardwareFeatures::SSD1306)
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
            signal(2);
            return false;
        }
        attempts++;
    }
    signal(1);
    return true;
}

bool connectAutoWifi()
{
    if (Config::Wifi::Auto::CLEAR_WIFI_SETTINGS)
        wm.resetSettings();

    wm.setConfigPortalTimeout(
        Config::Wifi::WIFIMANAGER_CONFIG_PORTAL_TIMEOUT_SECONDS
    );

    if (Config::HardwareFeatures::SSD1306)
    {
        char accessPointString[64];

        printDisplay("WiFi\nauto\nconfig\n", 1000);
        snprintf(accessPointString, sizeof(accessPointString), "WiFi\naccess\npoint\nfor %ds", Config::Wifi::WIFIMANAGER_CONFIG_PORTAL_TIMEOUT_SECONDS);
        printDisplay(accessPointString, 2500);
        snprintf(accessPointString, sizeof(accessPointString), "NW: %s\nPW: %s", Config::Wifi::SSID, Config::Wifi::PASSWORD);
        printDisplay(accessPointString);
    }

    // Config access point.
    bool wifi = wm.autoConnect(Config::Wifi::SSID, Config::Wifi::PASSWORD);
    if (wifi)
    {
        prtn("connected to WiFi");
        if (Config::HardwareFeatures::SSD1306)
            printDisplay("connected to WiFi", 1000);
        signal(1);
    }
    else
    {
        prtn("unable to connect to WiFi");
        if (Config::HardwareFeatures::SSD1306)
            printDisplay("unable to connect to WiFi");
        signal(2);
    }

    return wifi;
}

bool syncRTC()
{
    time_t utcNow = time(nullptr); 
    if (utcNow > 1000000000L)
    {
        rtc.adjust(DateTime(utcNow)); 
        return true;
    }
    else
        return false;
}

void easterEgg(char *easterEggString, size_t easterEggStringLength)
{
    int r = random(0, Config::WarmingUpInfo::TEXT_LENGTH);
    snprintf(easterEggString, easterEggStringLength, "Gas Nose Project\n\n%s\n\n", Config::WarmingUpInfo::TEXT[r]);
}

bool sketchJustUploaded()
{
    // These values should never be changed.
    #define EEPROM_SIZE 4
    #define ADDR_HASH   0
    char md5[9];

    EEPROM.begin(EEPROM_SIZE);

    /* Compute one‐byte fingerprint of this sketch (32 hex characters),
     * using the first 8 characters of the md5 string.
     */
    snprintf(md5, sizeof(md5), "%s", ESP.getSketchMD5().c_str());

    // Transform the checksum prefix into base 16 int.
    uint32_t hash = strtoul(md5, NULL, 16);
    uint32_t prev;
    EEPROM.get(ADDR_HASH, prev);

    if (hash != prev)
    {
        EEPROM.put(ADDR_HASH, hash);
        EEPROM.commit();

        prtn("EEPROM flag not set, adjusting RTC from compile time");
        prt("sketch MD5 extract hash differs: ");
        prt(hash);
        prt(" vs ");
        prtn(prev);

        return true;
    }
    else
    {
        prt("sketch not changed, MD5 extract hash is: ");
        prt(hash);
        prtn(", no need to manually set the time");
    }

    return false;
}

void setPins()
{
    pinMode(Config::Pins::Digital::MQ_2, INPUT);
    pinMode(Config::Pins::Digital::BUZZER, OUTPUT);
    pinMode(Config::Pins::Digital::LED, OUTPUT);
    pinMode(Config::Pins::Digital::SELF_TEST, INPUT_PULLUP);
}

void setup()
{
    int i;
    bool ok, done = false;

    if (PRINT_DEBUG)
    {
        Serial.begin(Config::Other::DEBUG_SPEED);
        Serial.flush();

    }
    setPins();
    
    while (!done)
    {
        ok = true;
        if (Config::HardwareFeatures::SSD1306)
            ok = ok & ssd1306Init();
        if (Config::HardwareFeatures::DHT11)
            ok = ok & dht11Init();
        if (Config::HardwareFeatures::DS3231)
            ok = ok & ds3231Init();
        ok = ok & activeBuzzerInit();
        ok = ok & builtintLedInit();

        if (ok)
            done = true;
    }

    if (Config::HardwareFeatures::SSD1306)
    {
        char versionString[32];
        snprintf(versionString, sizeof(versionString), "SW version\n%s", VERSION);
        printDisplay(versionString, 2000, false);
    }

    /* Set time and TZ.
     * setenv (...,...,1): the 1 overwrites previous values.
     */
    configTime(
        0,
        0,
        Config::DateTime::NTP_SERVERS[0],
        Config::DateTime::NTP_SERVERS[1],
        Config::DateTime::NTP_SERVERS[2]
    );
    setenv("TZ", Config::DateTime::Tz::POSIX, 1);
    tzset();

    if (Config::HardwareFeatures::DS3231)
    {
        if (rtc.lostPower() || sketchJustUploaded()) {
            // When time needs to be set on a new device, or after a power loss, the
            // following line sets the RTC to the date & time this sketch was compiled
            DateTime now = DateTime(F(__DATE__), F(__TIME__));

            // Compensate for local naive to UTC as well.
            DateTime compensated_datetime = now.unixtime() + Config::DateTime::COMPILATION_SECONDS_OFFSET + Config::DateTime::Tz::COMPUTER_NAIVE_TIME_TO_UTC_SECONDS_OFFSET;
            rtc.adjust(compensated_datetime);
        }
    }

    if (!Config::Wifi::WORK_OFFLINE)
    {
        if (Config::HardwareFeatures::SSD1306)
            printDisplay("WiFi init", 1000);
        bool wifi = Config::Wifi::MANUAL ? connectManualWifi() : connectAutoWifi();

        if (Config::HardwareFeatures::SSD1306)
        {
            if (wifi)
            {
                char wifiString[64];
                snprintf(wifiString, sizeof(wifiString), "SSID: %s\nRSSI: %d", WiFi.SSID(), WiFi.RSSI());
                printDisplay(wifiString);
            }
            else
                printDisplay("NO WiFi\nskipping...");
        }
    }

    if (Config::HardwareFeatures::MQ_2 || Config::HardwareFeatures::MQ_135)
    {
        prtn("Warming up MQ sensors");
        // Some MQ-* sensors (e.g. MQ-2) sense gas a few seconds after being powered.
        // delay(4000);
    }

    char easterEggString[64];

    /* If still not in temperature and within timeout, continue waiting. */
    for (i = 0;
         Config::HardwareFeatures::MQ_2
         && i < Config::Sensor::Setting::MQ_2_WARM_UP_MS / 1000
         && digitalRead(Config::Pins::Digital::MQ_2) == LOW;
         i++
    )
    {
        prt("warming seconds ");
        prt(i);
        prt(" of ");
        prtn(Config::Sensor::Setting::MQ_2_WARM_UP_MS / 1000);

        if (Config::HardwareFeatures::SSD1306)
        {
            easterEgg(easterEggString, sizeof(easterEggString));
            clearDisplay(1);
            display.println("Warming MQ-2 gas");
            display.println("sensor, please wait");
            display.print(i);
            display.print(" of ");
            display.println(Config::Sensor::Setting::MQ_2_WARM_UP_MS / 1000);
            display.println("seconds");
            display.println(easterEggString);
            display.display();
            delay(1000);
        }
    }

    for (i = 0; i < Config::Sensor::Setting::READINGS_BUFFER; i++)
    {
        if (Config::HardwareFeatures::SSD1306)
        {
            easterEgg(easterEggString, sizeof(easterEggString));
            clearDisplay(1);
            display.println("Sampling sensor");
            display.println("Please wait");
            display.print(i + 1);
            display.print(" of ");
            display.println(Config::Sensor::Setting::READINGS_BUFFER);
            display.println(easterEggString);
            display.display();
            clearDisplay();
        }

        prtn("sample ");
        prtn(i + 1);
        prtn("of");
        prtn(Config::Sensor::Setting::READINGS_BUFFER);

        if (Config::HardwareFeatures::MQ_2)
            gasSensorReadings[i] = analogRead(Config::Pins::Analog::MQ_2);
        else
            gasSensorReadings[i] = 0;

        float temperatureCelsiusDs3231, temperatureCelsiusDht11;
        getTemperatureCelsius(temperatureSensorReadings[i], temperatureCelsiusDs3231, temperatureCelsiusDht11);
        humiditySensorReadings[i] = Config::HardwareFeatures::DHT11 ? dht.readHumidity() + Config::Sensor::MeasurmentOffset::HUMIDITY_PERCENT : 0.0;
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
        // timeClient.begin();
        if (Config::HardwareFeatures::DS3231)
            syncRTC();
        apiClient = getHttpClient();
    }
}

bool dht11Init()
{
    dht.begin();
    return dht11SelfTest();
}

bool ds3231Init()
{
    if (!rtc.begin())
    {
        prtn("RTC error");
        signal(4);
        if (Config::HardwareFeatures::SSD1306)
            printDisplay("DS3231 KO", 1000, true);
        delay(2000);
        return false;
    }
    else
    {
        if (Config::HardwareFeatures::SSD1306)
            printDisplay("DS3231 OK", 1000, true);
        return true;
    }
}

bool ssd1306Init()
{
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        prtn("SSD1306 allocation failed");
        signal(3);
        delay(2000);
        return false;
    }
    else
    {
        display.setRotation(Config::Display::ROTATION);
        return ssd1306SelfTest();
    }
}

bool activeBuzzerInit()
{
    digitalWrite(Config::Pins::Digital::BUZZER, HIGH);
    delay(100);
    digitalWrite(Config::Pins::Digital::BUZZER, LOW);
    if (Config::HardwareFeatures::SSD1306)
        printDisplay("BUZZER OK", 1000, true);
    return true;
}

bool builtintLedInit()
{
    digitalWrite(Config::Pins::Digital::LED, HIGH);
    digitalWrite(Config::Pins::Digital::LED, LOW);
    delay(100);
    digitalWrite(Config::Pins::Digital::LED, HIGH);
    if (Config::HardwareFeatures::SSD1306)
        printDisplay("LED OK", 1000, true);
    return true;
}

bool ds3231SelfTest()
{
    Wire.beginTransmission(0x68);
    if (Wire.endTransmission() == 0)
    {
        if (Config::HardwareFeatures::SSD1306)
            printDisplay("DS3231 OK", 1000, true);
        prtn("DS3231 OK");
        return true;
    }
    else
    {
        prtn("DS3231 KO");
        signal(4);
        return false;
    }   
}

bool dht11SelfTest()
{
    if (isnan(dht.readHumidity()))
    {
        if (Config::HardwareFeatures::SSD1306)
            printDisplay("DHT11 KO", 1000, true);
        signal(2);
        return false;
    }
    else
    {
        if (Config::HardwareFeatures::SSD1306)
            printDisplay("DHT11 OK", 1000, true);
        return true;
    }
}

bool ssd1306SelfTest()
{
    Wire.beginTransmission(0x3C);
    if (Wire.endTransmission() == 0)
    {
        printDisplay("SSD1306 OK", 1000, true);
        prtn("SSD1306 OK");
        return true;
    }
    else
    {
        prtn("SSD1306 KO");
        signal(3);
        return false;
    }
}

bool activeBuzzerSelfTest()
{
    return activeBuzzerInit();
}

bool builtinLedSelfTest()
{
    return builtintLedInit();
}

bool selfTest()
{
    bool ok = true;
    if (Config::HardwareFeatures::SSD1306)
        printDisplay("HW TEST", 1000, true);

    if (Config::HardwareFeatures::SSD1306)
        ok = ok & ssd1306SelfTest();
    if (Config::HardwareFeatures::DHT11)
        ok = ok & dht11SelfTest();
    if (Config::HardwareFeatures::DS3231)
        ok = ok & ds3231SelfTest();
    ok = ok & activeBuzzerSelfTest();
    ok = ok & builtinLedSelfTest();

    if (Config::HardwareFeatures::SSD1306)
        printDisplay("TEST END", 1000, true);

    return ok;
}

void invertDisplayColors()
{
    inverted = !inverted;
    if (inverted)
        display.invertDisplay(true);
    else
        display.invertDisplay(false);
}

void getCurrentTime(const struct tm* now,  char *buffer, size_t length, const char* format = Config::DateTime::Format::TIME)
{
    if (strftime(buffer, length, format, now) == 0)
        prtn("strftime buffer error");
}

void getCurrentDate(const struct tm* now, char *buffer, size_t length, const char* format = Config::DateTime::Format::DATE)
{
    if (strftime(buffer, length, format, now) == 0)
        prtn("strftime buffer error");
}

void getCurrentISO8601(const struct tm* now, char *buffer, size_t length)
{
    if (strftime(buffer, length, "%Y-%m-%d %H:%M:%S %z", now) == 0)
        prtn("strftime buffer error");
}

void raiseGasAlarm()
{
    prtn("The gas is present");
    if (Config::HardwareFeatures::SSD1306)
        printDisplay(Config::Other::GAS_ALARM_TEXT, 0, true);
    signal(Config::Sensor::Alarm::BUZZER_SENSOR_ERROR_ITERATIONS, true);

    if (Config::HardwareFeatures::SSD1306)
        // Reset display colors.
        display.invertDisplay(false);
}

void getTemperatureCelsius(float &temperatureCelsius, float &temperatureCelsiusDs3231, float &temperatureCelsiusDht11)
{
    temperatureCelsiusDs3231 = Config::HardwareFeatures::DS3231 ? rtc.getTemperature() + Config::Sensor::MeasurmentOffset::TEMPERATURE_CELSIUS_DS3231: 0.0;
    temperatureCelsiusDht11 = Config::HardwareFeatures::DHT11 ? dht.readTemperature() + Config::Sensor::MeasurmentOffset::TEMPERATURE_CELSIUS_DHT11 : 0.0;
    if (Config::HardwareFeatures::DS3231 && Config::HardwareFeatures::DHT11)
        temperatureCelsius = (temperatureCelsiusDs3231 + temperatureCelsiusDht11) / 2;
    else if (!Config::HardwareFeatures::DS3231 && Config::HardwareFeatures::DHT11)
        temperatureCelsius = temperatureCelsiusDht11;
    else if (Config::HardwareFeatures::DS3231 && !Config::HardwareFeatures::DHT11)
        temperatureCelsius = temperatureCelsiusDs3231;
}

void displayTemperatureCelsius(float averageTemperatureReading, char temperatureRelativeTendency)
{
    display.print(averageTemperatureReading, 2);
    display.print(" ");
    display.print((char)247);
    display.print("C ");
    display.println(temperatureRelativeTendency);
}

bool doOTA()
{
    ArduinoOTA.setPasswordHash(Config::OTA::Auth::PASSWORD_MD5_HASH);
    ArduinoOTA.setHostname(Config::OTA::Generic::HOSTNAME);

    ArduinoOTA.onStart([]() {
        const char *type;

        if (ArduinoOTA.getCommand() == U_FLASH)
            type = "sketch";
        else
            // U_FS
            type = "filesystem";

        // NOTE: if updating FS this would be the place to unmount FS using FS.end()
        prt("OTA start ");
        prtn(type);
        if (Config::HardwareFeatures::SSD1306)
        {
            char OTAStartString[20];
            snprintf(OTAStartString, sizeof(OTAStartString), "OTA\nstart\n%s", type);
            printDisplay(OTAStartString, 5000, false);
            signal(1, false, 10);
        }
    });
    ArduinoOTA.onEnd([]() {
        prtn("OTA end");
        if (Config::HardwareFeatures::SSD1306)
        {
            printDisplay("OTA END", 1000, true);
            signal(1);
        }
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        char pgr[10];
        int percentage = progress / (total / 100);
        snprintf(pgr, sizeof(pgr), "OTA %u%%", percentage);
        prtn(pgr);
        if (Config::HardwareFeatures::SSD1306)
            printDisplay(pgr, 10, true);
        signal(1, false, 10);
    });
    ArduinoOTA.onError([](ota_error_t error) {
        char otaError[32];
        const char *err;

        if (error == OTA_AUTH_ERROR)
            err = "Auth Failed";
        else if (error == OTA_BEGIN_ERROR)
            err = "Begin Failed";
        else if (error == OTA_CONNECT_ERROR)
            err = "Connect Failed";
        else if (error == OTA_RECEIVE_ERROR)
            err = "Receive Failed";
        else if (error == OTA_END_ERROR)
            err = "End Failed";

        snprintf(otaError, sizeof(otaError), "OTA error[%u]: ", error);
        prt(otaError);
        prtn(err);

        if (Config::HardwareFeatures::SSD1306)
        {
            printDisplay(otaError, 10000, false);
            printDisplay(err, 10000, false);
        }
    });

    ArduinoOTA.begin();

    unsigned long startOTA = millis();
    prtn("waiting 60s for OTA");
    if (Config::HardwareFeatures::SSD1306)
        printDisplay("OTA 10 MIN", 1000, true);

    while (true)
    {
        ArduinoOTA.handle();

        if (millis() - startOTA > 600000)
        {
            prtn("OTA update was not started");
            if (Config::HardwareFeatures::SSD1306)
                printDisplay("NO OTA", 1000, true);
            return false;
        }
        yield();
    }

    return true;
}

void triggerFunctionWhileGrounded(int pin)
{
    /* Debounce. */
    delay(50);

    if (digitalRead(pin) == HIGH)
        return;

    /* Measure time span. */
    unsigned long start = millis();
    while (digitalRead(pin) == LOW)
    {
        prtn("trigger mode");
        if (Config::HardwareFeatures::SSD1306)
            printDisplay("TRIG MODE", 10, true);

        /* Wait until the button is released. */
        yield();
    }

    unsigned long timeSpan = millis() - start;
    for (int i = 0; i < numTriggers; i++)
    {
        if (timeSpan >= triggers[i].thresholdMs)
        {
            if (triggers[i].action != NULL)
            {
                /* Execute the longest action. */
                if (triggers[i].needsWiFi && WiFi.status() == WL_CONNECTED)
                {
                    triggers[i].action();
                    prtn(triggers[i].name);
                }
                else if (!triggers[i].needsWiFi)
                {
                    triggers[i].action();
                    prtn(triggers[i].name);
                }
                else if (triggers[i].needsWiFi && WiFi.status() != WL_CONNECTED)
                {
                    /* Needs wifi and is not connected, just return.*/
                    prt("trigger ");
                    prt(triggers[i].name);
                    prtn(" needs WiFi and is not connected!");

                    if (Config::HardwareFeatures::SSD1306)
                    {
                        printDisplay("CHECK WiFi", 250, true);
                        printDisplay("TO TRIGGER", 250, true);
                        printDisplay(triggers[i].name, 250, false);
                    }
                    return;
                }

                /* Just return after executing the first matching action by length. */
                return;
            }
            else
            {
                prt("trigger ");
                prt(triggers[i].name);
                prtn(" is undefined!");

                if (Config::HardwareFeatures::SSD1306)
                {
                    printDisplay("TRIGGER", 250, true);
                    printDisplay(triggers[i].name, 250, false);
                    printDisplay("UNDEFINED", 250, true);
                }
                return;
            }
        }
    }
}

void loop()
{
    int dummy = 0;
    char dummy2 = 0;
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillisTimestampUpdate >= Config::Rtc::RTC_GET_UPDATE_FREQUENCY_MS)
    {
        previousMillisTimestampUpdate = currentMillis;

        if (Config::HardwareFeatures::DS3231)
        {
            // Read the time from the RTC.
            dTimeNow = rtc.now();

            // Set the board time.
            setTime(dTimeNow.unixtime());
        }
        else if (!Config::Wifi::WORK_OFFLINE)
        {
            time_t now_system = time(nullptr);
            if (now_system > 1000000000L)
                setTime(now_system);
        }
    }

    // current time from the Time Library.
    time_t utc = now();
    struct tm *localTimeStruct = localtime(&utc);

    getCurrentTime(localTimeStruct, dTime, sizeof(dTime));
    getCurrentDate(localTimeStruct, dDate, sizeof(dDate));

    if (currentMillis - previousMillisDisplayData >= Config::Display::DISPLAY_TIMEOUT_MS && !gasDetected)
    {
        previousMillisDisplayData = currentMillis;

        // Avoid displaying data if an alarm can still be raised sequentially.
        if (Config::HardwareFeatures::SSD1306)
        {
            clearDisplay();

            if (Config::HardwareFeatures::DS3231 || !Config::Wifi::WORK_OFFLINE)
            {
                display.println(dTime);
                display.println(dDate);
            }
            else
            {
                uptime::calculateUptime();
                display.println("UPTIME");
                display.print(uptime::getDays());
                display.print("d");
                display.print(uptime::getHours());
                display.print("h");
                display.print(uptime::getMinutes());
                display.print(":");
                display.println(uptime::getSeconds());
            }

            if (Config::HardwareFeatures::DS3231 || Config::HardwareFeatures::DHT11)
                displayTemperatureCelsius(averageTemperatureReading, temperatureRelativeTendency);

            if (Config::HardwareFeatures::DHT11)
            {
                display.print(averageHumidityReading, 2);
                display.print(" %H ");
                display.println(humidityRelativeTendency);
            }
            
            display.display();
        }
    }
    
    if (currentMillis - previousMillis >= Config::Sensor::Setting::READING_TIMEOUT_MS)
    {
        previousMillis = currentMillis;

        if (Config::HardwareFeatures::SSD1306 && Config::Display::PERIODIC_COLOR_INVERT)
            invertDisplayColors();

        gasState = Config::HardwareFeatures::MQ_2 ? digitalRead(Config::Pins::Digital::MQ_2) : HIGH;
        float gasConcentration = Config::HardwareFeatures::MQ_2 ? (float) analogRead(Config::Pins::Analog::MQ_2) + Config::Sensor::MeasurmentOffset::GAS_VOLTS : 0.0;
        float temperatureCelsius, temperatureCelsiusDs3231, temperatureCelsiusDht11;
        getTemperatureCelsius(temperatureCelsius, temperatureCelsiusDs3231, temperatureCelsiusDht11);

        float humidityPercent = Config::HardwareFeatures::DHT11 ? dht.readHumidity() + Config::Sensor::MeasurmentOffset::HUMIDITY_PERCENT: 0.0;

        // Check if the DHT11 or DS3231 sensor have problems.
        if (isnan(temperatureCelsius) || isnan(humidityPercent))
        {
            // If there is a reading problem get the latest available value
            // which is presumably not NaN.
            prtn("Warning DHT11 or DS3231 is NaN");
            temperatureCelsius = temperatureSensorReadings[temperatureIndex];
            humidityPercent = humiditySensorReadings[humidityIndex];
        }

        updateSensorData(gasConcentration, gasSensorReadings, gasIndex, gasTotal, gasTendency, dummy2);
        averageGasReading = gasTotal / Config::Sensor::Setting::READINGS_BUFFER;
        updateSensorData(temperatureCelsius, temperatureSensorReadings, temperatureIndex, temperatureTotal, dummy, temperatureRelativeTendency);
        averageTemperatureReading = temperatureTotal / Config::Sensor::Setting::READINGS_BUFFER;
        updateSensorData(humidityPercent, humiditySensorReadings, humidityIndex, humidityTotal, dummy, humidityRelativeTendency);
        averageHumidityReading = humidityTotal / Config::Sensor::Setting::READINGS_BUFFER;

        // Transform into volts. The sensor is on the 5V rail.
        averageGasReadingVolts = (averageGasReading / 1023) * 5;
        snprintf(averageGasReadingNormalizedChar, sizeof(averageGasReadingNormalizedChar), "%.6f", averageGasReadingVolts);

        getCurrentISO8601(localTimeStruct, iso8601Timestamp, sizeof(iso8601Timestamp));
        // Config::HardwareFeatures::DS3231 ? iso8601Timestamp : "";

        prt("MQ2 sensor avg V: ");
        prtn(averageGasReadingVolts);
        prt("avg gas reading normalized: ");
        prtn(averageGasReadingNormalizedChar);
        prt("avg temp: ");
        prtn(averageTemperatureReading);
        prt("avg humd: ");
        prtn(averageHumidityReading);
        prt("DHT11 temp: ");
        prtn(temperatureCelsiusDht11);
        prt("DS3231 temp: ");
        prtn(temperatureCelsiusDs3231);
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

    if (PRINT_DEBUG && Config::HardwareFeatures::SSD1306 && currentMillis - previousMillisDisplayDebugData >= Config::Display::DISPLAY_DEBUG_DATA_FREQUENCY_MS)
    {
        previousMillisDisplayDebugData = currentMillis;
        /* 10 chars + '\0'. */
        char displayData[11];

        switch (printDebugCounter)
        {
            case 0:
                printDisplay("SERIAL DBG", 500, true);
                if (Config::Other::DISPLAY_EXTRA_DEBUG_INFO)
                    printDebugCounter += 1;
                break;
            case 1:
                snprintf(displayData, sizeof(displayData), "%d Bd", Config::Other::DEBUG_SPEED);
                printDisplay(displayData, 500, true);
                printDebugCounter += 1;
                break;
            case 2:
                char averageGasReadingNormalizedCharTmp[8];
                snprintf(averageGasReadingNormalizedCharTmp, sizeof(averageGasReadingNormalizedCharTmp), "%.2f", averageGasReadingVolts);
                snprintf(displayData, sizeof(displayData), "%s gas", averageGasReadingNormalizedCharTmp);
                printDisplay(displayData, 500, true);
                printDebugCounter += 1;
                break;
            case 3:
                uptime::calculateUptime();
                snprintf(displayData, sizeof(displayData), "%dd %dh up", uptime::getDays(), uptime::getHours());
                printDisplay(displayData, 500, true);
                printDebugCounter += 1;
                break;
            case 4:
                snprintf(displayData, sizeof(displayData), "SV %s", VERSION);
                printDisplay(displayData, 500, true);
                printDebugCounter += 1;
                break;
            case 5:
                printDisplay("TRIGGERS", 333, true);
                printDisplay("WITH GPIO", 333, true);
                snprintf(displayData, sizeof(displayData), "%d + GND", Config::Pins::Digital::SELF_TEST);
                printDisplay(displayData, 333, true);
                printDebugCounter += 1;
                break;
            case 6:
                if (Config::Wifi::WORK_OFFLINE)
                    printDisplay("OFFLN MODE", 500, true);
                else
                    printDisplay("ONLIN MODE", 500, true);

                // Rotate debug prints.
                printDebugCounter = 0;
                break;
            default:
                // Rotate debug prints.
                printDebugCounter = 0;
                break;
        }
    }

    if (!Config::Wifi::WORK_OFFLINE
       && Config::HardwareFeatures::DS3231
       && currentMillis - previousMillisSetUpdateRTC >= Config::Rtc::RTC_SET_UPDATE_FREQUENCY_MS
       && WiFi.status() == WL_CONNECTED)
        {
            previousMillisSetUpdateRTC = currentMillis;
            syncRTC();
        }

    if (!Config::Wifi::WORK_OFFLINE
       && currentMillis - previousMillisNetworkCheck >= Config::Network::SEND_DATA_FREQUENCY_MS)
    {
        previousMillisNetworkCheck = currentMillis;

        // Avoid busy waiting if not connected.
        if (WiFi.status() == WL_CONNECTED)
        {
            if (Config::Network::SEND_TO_API && apiClient != nullptr)
            {
                prt("connecting to ");
                prtn(Config::Network::HOST);
                if (!apiClient->connect(Config::Network::HOST, Config::Network::PORT))
                {
                    prtn("connection failed, ignoring...");
                    signal(4, false, 100, false);
                    apiClient->stop();
                }
                else
                {
                    JsonDocument doc;
                    buildJson(doc);
                    sendJSONRequest(*apiClient, doc);
                    #ifdef PRINT_DEBUG
                        sendJSONRequest(Serial, doc);
                        prtn();
                    #endif

                    if (Config::Network::DEBUG)
                    {
                        while (apiClient->connected() || apiClient->available())
                        {
                            if (apiClient->available())
                            {
                                char buffer[64];
                                /* Keep space for '\0'. */
                                size_t length = apiClient->readBytesUntil('\n', buffer, sizeof(buffer) - 1);
                                buffer[length] = '\0';
                                prtn(buffer);
                            }
                        }
                    }
                    else
                    {
                        // While the server is not available, or if there is no content yet, wait up to 2 seconds.
                        unsigned long timeout = millis();
                        while (apiClient->available() == 0 && millis() - timeout < 2000)
                            yield();

                        if (apiClient->available())
                        {
                            char HTTPstatusLine[16];
                            size_t n = apiClient->readBytes(HTTPstatusLine, 15);
                            HTTPstatusLine[n] = '\0';

                            if (!(
                            strstr(HTTPstatusLine, "200") != NULL
                            || strstr(HTTPstatusLine, "201") != NULL
                            || strstr(HTTPstatusLine, "202") != NULL)
                            )
                                signal(4, false, 100, false);

                            prt("network packet HTTP status: ");
                            prtn(HTTPstatusLine);
                        }
                        else
                            signal(4, false, 100, false);
                    }

                    apiClient->stop();
                }
            }
        }
        else
        {
            checkWifi++;
        }
    }

    if (!Config::Wifi::WORK_OFFLINE && checkWifi % 10 == 0 && WiFi.status() != WL_CONNECTED)
    {
        /* See also
         * https://github.com/tzapu/WiFiManager/issues/1588#issue-1670081323
         */
        wm.setConfigPortalTimeout(0);
        
        prtn("WiFi is disconnected");
        if (Config::HardwareFeatures::SSD1306)
            printDisplay("NO WiFi\nretrying...");
        WiFi.reconnect();
        if (Config::HardwareFeatures::SSD1306)
            clearDisplay();
        checkWifi = 1;
    }

    if (Config::Other::ENABLE_LED_HEARTBEAT
        && currentMillis - previousMillisBuiltinLedHeartbeat >= Config::Other::BUILTIN_LED_HEARTBEAT_INTERVAL)
    {
        previousMillisBuiltinLedHeartbeat = currentMillis;
        analogWrite(Config::Pins::Digital::LED, builtinLedBrightness);
        builtinLedBrightness = builtinLedBrightness + builtinLedFadeAmount;

        // Reverse the direction of the fading at the ends of the fade.
        if (builtinLedBrightness <= 0 || builtinLedBrightness >= Config::Other::BUILTIN_LED_MAX_PWM_VALUE)
            builtinLedFadeAmount = -builtinLedFadeAmount;
    }

    triggerFunctionWhileGrounded(Config::Pins::Digital::SELF_TEST);
}
