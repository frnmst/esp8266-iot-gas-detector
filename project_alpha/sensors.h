#ifndef SENSORS_H
#define SENSORS_H

#include "common.h"
#include "config.h"
#include "output.h"

extern DHT dht;
extern RTC_DS3231 rtc;

bool dht11SelfTest();
bool dht11Init();
void getTemperatureCelsius(float &temperatureCelsius, float &tempCelsiusDs3231, float &tempCelsiusDht11);
void getHumidityPercent(float &humidityPercent);

#endif