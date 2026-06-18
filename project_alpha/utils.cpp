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
#include "utils.h"

bool getCurrentDateTime(const struct tm* now,  char *buffer, size_t length, const char* format)
{
    if (now == NULL || buffer == NULL || format == NULL || length == 0)
    {
        prtn("strftime buffer error");
        if (buffer != NULL && length > 0)
            buffer[0] = '\0';

        return false;
    }
    if (strcmp(format, "iso8601") == 0)
        format = "%Y-%m-%d %H:%M:%S %z";

    if (strftime(buffer, length, format, now) == 0)
    {
        prtn("strftime buffer error");
        return false;
    }
    else
        return true;
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

bool sketchJustUploaded()
{
    /* TODO FIXME: this function should use LittleFS instead. */
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

void checkPomodoroTimer()
{
    if (millis() - pomodoroTimerStart >= pomodoroTimerMsLength)
        pomodoroSignal = true;
}

void executePomodoroSignal()
{
    signal(100, true, 25, true);
    // Reset.
    pomodoroSignal = false;
    pomodoroTimer = false;
}

void getPomodoroString(char *outputString)
{
    unsigned long totalTimeoutSeconds = pomodoroTimerMsLength / 1000;
    unsigned long secondsElapsed = (millis() - pomodoroTimerStart) / 1000;

    if (secondsElapsed >= totalTimeoutSeconds) 
        snprintf(outputString, 10, "00:00  %c", 0x0f);
    else 
    {
        unsigned long totalSecondsRemaining = totalTimeoutSeconds - secondsElapsed;
        unsigned long minutesRemaining = totalSecondsRemaining / 60;
        unsigned long secondsRemaining = totalSecondsRemaining % 60;

        // Show icon every other second.
        char pomodoroIcon = '\0';
        if (secondsRemaining % 2 == 0)
            pomodoroIcon = 0x0f;

        // Override date with timer: "00:00 <char 0x0e>"
        snprintf(outputString, 10, "%02lu:%02lu  %c", minutesRemaining, secondsRemaining, pomodoroIcon);
    }
}