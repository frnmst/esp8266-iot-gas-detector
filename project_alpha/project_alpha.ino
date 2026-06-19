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

/* Embedded libraries. */
#include "common.h"
/* Local includes. */
#include "config.h"

/* Local code includes. */
#include "network_utils.h"
#include "output.h"
#include "utils.h"
#include "sensors.h"

// Devices.
RTC_DS3231 rtc;
Adafruit_SSD1306 display = Adafruit_SSD1306(Config::Display::WIDTH, Config::Display::HEIGHT, &Wire);
DHT dht(Config::Pins::Digital::DHT_11, Config::Sensor::Setting::DHT_TYPE);

// Data structures.
WiFiManager wm;
float gasSensorReadings[Config::Sensor::Setting::READINGS_BUFFER], temperatureSensorReadings[Config::Sensor::Setting::READINGS_BUFFER], humiditySensorReadings[Config::Sensor::Setting::READINGS_BUFFER];
float averageGasReading, averageTemperatureReading, averageHumidityReading, gasTotal = 0.0, temperatureTotal = 0.0, humidityTotal = 0.0;
float averageGasReadingVolts;
float temperatureCelsiusDs3231, temperatureCelsiusDht11;
char averageGasReadingNormalizedChar[32];
static int gasIndex = 0, temperatureIndex = 0, humidityIndex = 0;
int gasState, gasTendency = 0;
char temperatureRelativeTendency = 0, humidityRelativeTendency = 0;
bool pomodoroSignal = false, inverted = false, gasDetected = false, showCurrentTimeOnly = false, pomodoroTimer = false, firstSamplingArrayIteration = true, firstGasSamplingArrayIteration = true;
unsigned short int loops = 0;
unsigned long pomodoroTimerMsLength = 25 * 60 * 1000, pomodoroTimerStart, previousMillis = 0, previousMillisNetworkCheck = 0, previousMillisBuiltinLedHeartbeat = 0, previousMillisTimestampUpdate = 0, previousMillisDisplayData = 0, previousMillisDisplayDebugData = 0, previousMillisSetUpdateRTC = 0;
int checkWifi = 1, printDebugCounter = 0;
int builtinLedBrightness = 0, builtinLedFadeAmount = 5;
const long builtinLedHeartbeatInterval = 50;
char dTime[16] = {0}, dDate[32] = {0}, iso8601Timestamp[32] = {0};
DateTime dTimeNow = DateTime();
WiFiClient* apiClient = nullptr;
const int numTriggers = sizeof(buttonTriggers) / sizeof(buttonTriggers[0]);
bool shouldSaveConfig = false;
const char *filePath = "/data";

/* WifiManager custom values*/
JsonDocument configDoc;
char display_rotation[2], time_format[32], date_format[32], timezone_posix_string[50];
char ds3231[2] = {0}, ssd1306[2] = {0}, dht11[2] = {0}, mq_2[2] = {0}, mq_135[2] = {0};
char offset_temperature_celsius_dht11[21];

WiFiManagerParameter *custom_display_rotation;
WiFiManagerParameter *custom_time_format;
WiFiManagerParameter *custom_date_format;
WiFiManagerParameter *custom_timezone_posix_string;
WiFiManagerParameter *custom_ds3231;
WiFiManagerParameter *custom_ssd1306;
WiFiManagerParameter *custom_dht11;
WiFiManagerParameter *custom_mq_2;
WiFiManagerParameter *custom_mq_135;
WiFiManagerParameter *custom_offset_temperature_celsius_dht11;

/* Initializes the file system. */
bool initFileSystem()
{
    if (!LittleFS.begin())
    {
        prtn("littlefs mount failed");
        return false;
    }
    return true;
}

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
    board["debug"] = bool(Config::Debug::PRINT);
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

void setPins()
{
    pinMode(Config::Pins::Digital::MQ_2, INPUT);
    pinMode(Config::Pins::Digital::DHT_11, INPUT);
    pinMode(Config::Pins::Digital::BUZZER, OUTPUT);
    pinMode(Config::Pins::Digital::LED, OUTPUT);
    pinMode(Config::Pins::Digital::BUTTON_A, INPUT_PULLUP);
}

bool ds3231Init()
{
    if (!rtc.begin())
    {
        prtn("RTC error");
        if (Config::HardwareFeatures::SSD1306)
            printDisplay("DS3231 KO", 0, true);
        signal(4);
        return false;
    }
    else
    {
        if (Config::HardwareFeatures::SSD1306)
            printDisplay("DS3231 OK", 100, true);
        return true;
    }
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
        printDisplay("HW TEST", 500, true);

    if (Config::HardwareFeatures::SSD1306)
        ok = ok & ssd1306SelfTest();
    if (Config::HardwareFeatures::DHT11)
        ok = ok & dht11SelfTest();
    if (Config::HardwareFeatures::DS3231)
        ok = ok & ds3231SelfTest();
    ok = ok & activeBuzzerSelfTest();
    ok = ok & builtinLedSelfTest();

    if (Config::HardwareFeatures::SSD1306)
        printDisplay("TEST END", 500, true);

    return ok;
}

void raiseGasAlarm()
{
    prtn("The gas is present!");
    if (Config::HardwareFeatures::SSD1306)
        printDisplay(Config::Other::GAS_ALARM_TEXT, 0, true);
    signal(Config::Sensor::Alarm::BUZZER_SENSOR_ERROR_ITERATIONS, true);

    if (Config::HardwareFeatures::SSD1306)
        // Reset display colors.
        display.invertDisplay(false);
}

void triggerFunctionWhileGrounded(int pin)
{
    /* Debounce. */
    delay(50);

    if (digitalRead(pin) == HIGH)
        return;

    /* Measure time span. */
    unsigned long start = millis();
    unsigned long timeSpan;
    int triggerId = -1;
    bool deInitDisplay = false;
    if (!Config::HardwareFeatures::SSD1306)
    {
        deInitDisplay = true;
        ssd1306Init();
    }

    while (digitalRead(pin) == LOW)
    {
        prtn("trigger mode");
        if (Config::HardwareFeatures::SSD1306 && triggerId == -1)
            printDisplay("TRIG MODE", 1, true);

        timeSpan = millis() - start;
        int i = numTriggers - 1;
        while (digitalRead(pin) == LOW)
        {
            timeSpan = millis() - start;
            if (timeSpan >= (numTriggers - i) * triggerThresholdMs)
            {
                if (buttonTriggers[i].action == NULL)
                {
                    prt("trigger ");
                    prt(buttonTriggers[i].name);
                    prtn(" is undefined!");
                }
                else
                    triggerId = i;

                i--;
            }

            if (triggerId >= 0)
            {
                char displayData[64];
                snprintf(displayData, sizeof(displayData), "%s\n[%d/%d]", buttonTriggers[triggerId].displayName, abs(numTriggers - triggerId), numTriggers);
                printDisplay(displayData, 1, false);
            }

            if (i < 0)
            {
                /* Reset data. */
                i = numTriggers - 1;
                start = millis();
                timeSpan = millis() - start;
            }
        }

        /* Wait until the button is released. */
        yield();
    }
    if (deInitDisplay)
        ssd1306DeInit();

    if (triggerId >= 0)
    {
        delay(250);
        if (buttonTriggers[triggerId].needsWiFi && WiFi.status() == WL_CONNECTED)
        {
            buttonTriggers[triggerId].action();
            prtn(buttonTriggers[triggerId].name);
        }
        else if (!buttonTriggers[triggerId].needsWiFi)
        {
            buttonTriggers[triggerId].action();
            prtn(buttonTriggers[triggerId].name);
        }
        else if (buttonTriggers[triggerId].needsWiFi && WiFi.status() != WL_CONNECTED)
        {
            /* Needs wifi and is not connected, just return.*/
            prt("trigger ");
            prt(buttonTriggers[triggerId].name);
            prtn(" needs WiFi and is not connected!");

            if (Config::HardwareFeatures::SSD1306)
            {
                printDisplay("CHECK WiFi", 250, true);
                printDisplay("TO TRIGGER", 250, true);
                printDisplay(buttonTriggers[triggerId].displayName, 250, false);
            }
            return;
        }
    }
}

#if defined(RUN_UNIT_TESTS)
void setup()
{
    Serial.begin(115200); 
    while (!Serial);
    delay(5000);

    prtn("unit tests");
}

void loop()
{
    aunit::TestRunner::run();
}
#else
void setup()
{
    int i;
    bool ok, done = false;

    if (Config::Debug::PRINT)
    {
        Serial.begin(Config::Other::DEBUG_SPEED);
        Serial.flush();
    }
    Wire.begin();
    setPins();

    initFileSystem();
    configDoc.clear();
    updateConfigFromJsonConfigFile(LittleFS, filePath, configDoc);
    /* We need to overwrite hardcoded defaults with the values
     * present in the flash memory.
     */
    addCustomBoardSettingsWifiManager();

    while (!done)
    {
        ok = true;
        if (Config::HardwareFeatures::SSD1306)
            ok = ok & ssd1306Init();
        else
        {
            /* Turn off the display.
             * It would still be on if a simple reset was performed:
             * the last data remains printed on screen
             * To be able to de-init we have to allocate the object first.
             */
            if (ssd1306Init(false))
                ssd1306DeInit();
        }
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
        printDisplay(versionString, 500, false);
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

    if (Config::Wifi::WORK_ONLINE)
    {
        if (Config::HardwareFeatures::SSD1306)
            printDisplay("WiFi init", 500, true);

        bool wifi = Config::Wifi::MANUAL ? connectManualWifi() : connectAutoWifi();
        if (wifi)
        {
            WiFi.setAutoReconnect(true);
            if (Config::HardwareFeatures::SSD1306)
            {
                char wifiString[64];
                snprintf(wifiString, sizeof(wifiString), "SSID: %s\nRSSI: %d", WiFi.SSID(), WiFi.RSSI());
                printDisplay(wifiString);
            }
            if (Config::HardwareFeatures::DS3231)
                syncRTC();
            apiClient = getHttpClient();
        }
        else
        {
            if (Config::HardwareFeatures::SSD1306)
                printDisplay("NO WiFi\nskipping...", 500);
        }
    }
}

void loop()
{
    int dummy = 0;
    char dummy2 = 0;
    unsigned long currentMillis = millis();
    time_t utc;
    struct tm *localTimeStruct;

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
        else if (Config::Wifi::WORK_ONLINE)
        {
            time_t now_system = time(nullptr);
            if (now_system > 1000000000L)
                setTime(now_system);
        }
    }

    if (currentMillis - previousMillisDisplayData >= Config::Display::DISPLAY_TIMEOUT_MS && !gasDetected)
    {
        previousMillisDisplayData = currentMillis;
        if (pomodoroTimer)
            checkPomodoroTimer();

        // Avoid displaying data if an alarm can still be raised sequentially.
        if (Config::HardwareFeatures::SSD1306)
        {
            clearDisplay();

            // Current time from the Time Library.
            utc = now();
            localTimeStruct = localtime(&utc);

            getCurrentDateTime(localTimeStruct, dTime, sizeof(dTime), Config::DateTime::Format::TIME);
            getCurrentDateTime(localTimeStruct, dDate, sizeof(dDate), Config::DateTime::Format::DATE);

            if (showCurrentTimeOnly)
                displayTimeCentered(dTime);
            else
            {
                if (pomodoroTimer)
                    getPomodoroString(dDate);

                if (Config::HardwareFeatures::DS3231 || Config::Wifi::WORK_ONLINE)
                    displayTimeDate(dTime, dDate);
                else
                {
                    if (!pomodoroTimer)
                        // Keep the second line empty for future use in non time tracking scenarios.
                        snprintf(dDate, 1, "%c", '\0');
                    displayUptime(dDate);
                }

                if (Config::HardwareFeatures::DS3231 || Config::HardwareFeatures::DHT11)
                    displayTemperatureCelsius(averageTemperatureReading, temperatureRelativeTendency);

                if (Config::HardwareFeatures::DHT11)
                    displayHumidityPercent(averageHumidityReading, humidityRelativeTendency);
            }
            
            display.display();
        }

        if (pomodoroSignal)
            executePomodoroSignal();
    }
    
    if (currentMillis - previousMillis >= Config::Sensor::Setting::READING_TIMEOUT_MS)
    {
        previousMillis = currentMillis;

        if (Config::HardwareFeatures::SSD1306 && Config::Display::PERIODIC_COLOR_INVERT)
            invertDisplayColors();

        float temperatureCelsius;
        float humidityPercent;
        getTemperatureCelsius(temperatureCelsius, temperatureCelsiusDs3231, temperatureCelsiusDht11);
        getHumidityPercent(humidityPercent);

        // Check if the DHT11 or DS3231 sensor have problems.
        if (isnan(temperatureCelsius) || isnan(humidityPercent))
        {
            /* If there is a reading problem get the latest available value
             * which is presumably not NaN.
             */
            prtn("Warning DHT11 or DS3231 is NaN");
            temperatureCelsius = temperatureSensorReadings[temperatureIndex];
            humidityPercent = humiditySensorReadings[humidityIndex];
        }
        updateSensorData(temperatureCelsius, temperatureSensorReadings, temperatureIndex, temperatureTotal, dummy, temperatureRelativeTendency);
        updateSensorData(humidityPercent, humiditySensorReadings, humidityIndex, humidityTotal, dummy, humidityRelativeTendency);

        /* Avoid the sensor sampling phase by computing a moving average in the first loop. */
        unsigned int r = Config::Sensor::Setting::READINGS_BUFFER;
        if (firstSamplingArrayIteration)
        {
            loops++;
            r = loops;
            if (loops == Config::Sensor::Setting::READINGS_BUFFER)
                firstSamplingArrayIteration = false;
        }
        
        float gasConcentration = 0.0;
        gasState = Config::HardwareFeatures::MQ_2 ? LOW : HIGH;
        averageGasReading = 0.0;
        averageGasReadingVolts = 0.0;
        snprintf(averageGasReadingNormalizedChar, 2, "%s", "0.0");
        if (Config::HardwareFeatures::MQ_2
           && firstGasSamplingArrayIteration
           && digitalRead(Config::Pins::Digital::MQ_2) == LOW)
        {
            prtn("warning: MQ still not in temperature, check potentiometer on the back!");
            signal(1, false, 10, true);
            gasState = HIGH;
        }
        else if (Config::HardwareFeatures::MQ_2
           && firstGasSamplingArrayIteration
           && digitalRead(Config::Pins::Digital::MQ_2) == HIGH)
        {
            prtn("gas sensor in temperature");
            firstGasSamplingArrayIteration = false;
            gasState = HIGH;
        }
        else if (Config::HardwareFeatures::MQ_2)
        {
            prtn("gas sensor computing...");
            gasState = Config::HardwareFeatures::MQ_2 ? digitalRead(Config::Pins::Digital::MQ_2) : HIGH;
            gasConcentration = Config::HardwareFeatures::MQ_2 ? (float) analogRead(Config::Pins::Analog::MQ_2) + Config::Sensor::MeasurmentOffset::GAS_VOLTS : 0.0;
            updateSensorData(gasConcentration, gasSensorReadings, gasIndex, gasTotal, gasTendency, dummy2);
            averageGasReading = gasTotal / r;

            // Transform into volts. The sensor is on the 5V rail.
            averageGasReadingVolts = (averageGasReading / 1023) * 5;
            snprintf(averageGasReadingNormalizedChar, sizeof(averageGasReadingNormalizedChar), "%.6f", averageGasReadingVolts);
        }

        averageTemperatureReading = temperatureTotal / r;
        averageHumidityReading = humidityTotal / r;

        // current time from the Time Library.
        utc = now();
        localTimeStruct = localtime(&utc);
        getCurrentDateTime(localTimeStruct, iso8601Timestamp, sizeof(iso8601Timestamp), "iso8601");

        if (gasState == LOW
            || (!Config::Triggers::IGNORE_GAS_VOLTAGE && averageGasReadingVolts >= Config::Triggers::GAS_ALARM_VOLTS_THRESHOLD)
            || (!Config::Triggers::IGNORE_GAS_TENDENCY && gasTendency >= Config::Triggers::GAS_ALARM_TENDENCY_ITERATIONS)
        )
        {
            prtn(gasState);
            prtn(!Config::Triggers::IGNORE_GAS_VOLTAGE);
            prtn(!Config::Triggers::IGNORE_GAS_TENDENCY);
            gasDetected = true;
            raiseGasAlarm();
        }
        else
            gasDetected = false;

        showDebugDataOnSerial();
    }

    if (Config::Debug::PRINT && Config::HardwareFeatures::SSD1306 && currentMillis - previousMillisDisplayDebugData >= Config::Display::DISPLAY_DEBUG_DATA_FREQUENCY_MS)
    {
        previousMillisDisplayDebugData = currentMillis;
        showDebugDataOnDisplay();
    }

    if (Config::Wifi::WORK_ONLINE
       && Config::HardwareFeatures::DS3231
       && currentMillis - previousMillisSetUpdateRTC >= Config::Rtc::RTC_SET_UPDATE_FREQUENCY_MS
       && WiFi.status() == WL_CONNECTED)
        {
            previousMillisSetUpdateRTC = currentMillis;
            syncRTC();
        }

    if (Config::Wifi::WORK_ONLINE
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
                    if (Config::Debug::PRINT)
                    {
                        if (Config::Network::FULL_DEBUG || Config::Network::BASIC_DEBUG)
                        {
                            sendJSONRequest(Serial, doc);
                            prtn();
                        }
                    }

                    if (Config::Network::FULL_DEBUG)
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

                            if (Config::Network::BASIC_DEBUG)
                            {
                                prt("network packet HTTP status: ");
                                prtn(HTTPstatusLine);
                            }
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

    if (Config::Wifi::WORK_ONLINE && checkWifi % 10 == 0 && WiFi.status() != WL_CONNECTED)
        reconnectWifi();

    if (Config::Other::ENABLE_LED_HEARTBEAT
        && currentMillis - previousMillisBuiltinLedHeartbeat >= Config::Other::BUILTIN_LED_HEARTBEAT_INTERVAL)
    {
        previousMillisBuiltinLedHeartbeat = currentMillis;
        LEDHeartbeat();
    }

    triggerFunctionWhileGrounded(Config::Pins::Digital::BUTTON_A);
}
#endif
