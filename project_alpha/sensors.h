#ifndef SENSORS_H
#define SENSORS_H

#include "common.h"
#include "config.h"
#include "output.h"

extern DHT dht;
extern RTC_DS3231 rtc;

bool dht11SelfTest(bool boot = false);
bool dht11Init(bool boot = false);
void getTemperatureCelsius(float &temperatureCelsius, float &tempCelsiusDs3231, float &tempCelsiusDht11);
void getHumidityPercent(float &humidityPercent);

#endif