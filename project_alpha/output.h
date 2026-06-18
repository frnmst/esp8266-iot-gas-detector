#ifndef OUTPUT_H
#define OUTPUT_H

#include "common.h"
#include "config.h"

extern Adafruit_SSD1306 display;
extern bool inverted, gasDetected;
extern float averageGasReadingVolts, averageTemperatureReading, averageHumidityReading, temperatureCelsiusDht11, temperatureCelsiusDs3231;
extern int gasTendency, printDebugCounter;
extern char averageGasReadingNormalizedChar[32], iso8601Timestamp[32];
extern int builtinLedBrightness, builtinLedFadeAmount;

void signal(int repetitions = 2, bool invertColors = false, int lengthMs = Config::Sensor::Alarm::BUZZER_SENSOR_ERROR_DELAY_MS, bool buzzer = true);
bool activeBuzzerInit();
bool builtintLedInit();
void invertDisplayColors();
bool ssd1306Init(bool assertInit = true);
void ssd1306DeInit();
bool ssd1306SelfTest();
void clearDisplay(int text_size = 2);
void printDisplay(const char *text, int timeout = 5000, bool center = false, int textSize = 2);
void displayTemperatureCelsius(float averageTemperatureReading, char temperatureRelativeTendency);
void displayUptime(char *extraData);
void displayHumidityPercent(float averageHumidityReading, char humidityRelativeTendency);
void displayTimeDate(char *time, char *date);
void displayTimeCentered(char *time);
void showDebugDataOnSerial();
void showDebugDataOnDisplay();
void LEDHeartbeat();

#endif