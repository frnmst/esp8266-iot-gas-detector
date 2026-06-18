#include "output.h"

void signal(int repetitions, bool invertColors, int lengthMs, bool buzzer)
{
    for (int i = 0; i < repetitions; i++)
    {
        if (buzzer)
            digitalWrite(Config::Pins::Digital::BUZZER, HIGH);
        // Turn the LED on (Note that LOW is the voltage level but actually the LED is on).
        digitalWrite(Config::Pins::Digital::LED, LOW);

        delay(lengthMs);

        if (buzzer)
            digitalWrite(Config::Pins::Digital::BUZZER, LOW);

        // Turn the LED off by making the voltage HIGH.
        digitalWrite(Config::Pins::Digital::LED, HIGH);

        // Avoid the last useless delay.
        if (i < repetitions - 1)
            delay(lengthMs);

        if (Config::HardwareFeatures::SSD1306 && invertColors)
            invertDisplayColors();
    }
}

bool activeBuzzerInit()
{
    digitalWrite(Config::Pins::Digital::BUZZER, HIGH);
    delay(Config::Init::HARDWARE_OUTPUT_DELAY_MS);
    digitalWrite(Config::Pins::Digital::BUZZER, LOW);
    if (Config::HardwareFeatures::SSD1306)
        printDisplay("BUZZER OK", Config::Init::HARDWARE_OUTPUT_DELAY_MS, true);
    return true;
}

bool builtintLedInit()
{
    digitalWrite(Config::Pins::Digital::LED, HIGH);
    digitalWrite(Config::Pins::Digital::LED, LOW);
    delay(Config::Init::HARDWARE_OUTPUT_DELAY_MS);
    digitalWrite(Config::Pins::Digital::LED, HIGH);
    if (Config::HardwareFeatures::SSD1306)
        printDisplay("LED OK", Config::Init::HARDWARE_OUTPUT_DELAY_MS, true);
    return true;
}

void invertDisplayColors()
{
    inverted = !inverted;
    if (inverted)
        display.invertDisplay(true);
    else
        display.invertDisplay(false);
}

bool ssd1306Init(bool assertInit)
{
    if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        /* Transmission can begin even without display connected. */
        Wire.beginTransmission(0x3C);
        if (Wire.endTransmission() == 0)
        {
            display.setRotation(Config::Display::ROTATION);
        }
        else
        {
            prtn("SSD1306 allocation failed");
        }
        if (assertInit)
            return ssd1306SelfTest();
        else
            return true;
    }
    else
    {
        prtn("SSD1306 allocation failed");
        if (assertInit)
        {
            /* This will always return false. */
            return ssd1306SelfTest();
        }
        else
        {
            return false;
        }
    }
}

void ssd1306DeInit()
{
    // 1. Spegne fisicamente i pixel e il controller interno (risparmio energetico)
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    
    // Clean the display.
    display.clearDisplay();
    display.display();
}

bool ssd1306SelfTest()
{
    Wire.beginTransmission(0x3C);
    if (Wire.endTransmission() == 0)
    {
        printDisplay("SSD1306 OK", Config::Init::HARDWARE_OUTPUT_DELAY_MS, true);
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

void clearDisplay(int text_size)
{
    display.clearDisplay();
    display.setTextSize(text_size);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
}

void printDisplay(const char *text, int timeout, bool center, int textSize)
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

void displayTemperatureCelsius(float averageTemperatureReading, char temperatureRelativeTendency)
{
    display.print(averageTemperatureReading, 2);
    display.print(" ");
    display.print((char)247);
    display.print("C ");
    display.println(temperatureRelativeTendency);
}

void displayUptime(char *extraData)
{
    uptime::calculateUptime();
    display.print(uptime::getDays());
    display.print("d");
    display.print(uptime::getHours());
    display.print("h");
    display.print(uptime::getMinutes());
    display.print(":");
    display.println(uptime::getSeconds());
    display.println(extraData);
}

void displayHumidityPercent(float averageHumidityReading, char humidityRelativeTendency)
{
    display.print(averageHumidityReading, 2);
    display.print(" %H ");
    display.println(humidityRelativeTendency);
}

void displayTimeDate(char *time, char *date)
{
    display.println(time);
    display.println(date);
}

void displayTimeCentered(char *time)
{
    printDisplay(time, 0, true, 2);
}

void showDebugDataOnSerial()
{
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
    if (Config::Wifi::WORK_ONLINE && WiFi.status() == WL_CONNECTED)
    {
        prt("SSID: ");
        prtn(WiFi.SSID());
        prt("RSSI: ");
        prtn(WiFi.RSSI());
        prt("IP: ");
        prtn(WiFi.localIP());
    }
    if (gasDetected)
        prtn("Gas IS present!");
    else
        prtn("The gas is NOT present");
}

void showDebugDataOnDisplay()
{
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
            snprintf(displayData, sizeof(displayData), "%d + GND", Config::Pins::Digital::BUTTON_A);
            printDisplay(displayData, 333, true);
            printDebugCounter += 1;
            break;
        case 6:
            if (Config::Wifi::WORK_ONLINE)
                printDisplay("ONLIN MODE", 500, true);
            else
                printDisplay("OFFLN MODE", 500, true);

            // Rotate debug prints.
            printDebugCounter = 0;
            break;
        default:
            // Rotate debug prints.
            printDebugCounter = 0;
            break;
    }
}

void LEDHeartbeat()
{
    analogWrite(Config::Pins::Digital::LED, builtinLedBrightness);
    builtinLedBrightness = builtinLedBrightness + builtinLedFadeAmount;

    // Reverse the direction of the fading at the ends of the fade.
    if (builtinLedBrightness <= 0 || builtinLedBrightness >= Config::Other::BUILTIN_LED_MAX_PWM_VALUE)
        builtinLedFadeAmount = -builtinLedFadeAmount;
}
