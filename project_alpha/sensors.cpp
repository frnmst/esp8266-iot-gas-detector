#include "sensors.h"

bool dht11SelfTest(bool boot)
{
    unsigned long int start = millis();
    if (boot)
    {
        // Busy wait on boot: give enough time so that the DHT11 sensor is ready.
        while (millis() - start < 4000 && isnan(dht.readHumidity()))
            delay(250);
    }

    if (isnan(dht.readHumidity()))
    {
        if (Config::HardwareFeatures::SSD1306)
            printDisplay("DHT11 KO", Config::Init::HARDWARE_OUTPUT_DELAY_MS, true);
        signal(2);
        return false;
    }
    else
    {
        if (Config::HardwareFeatures::SSD1306)
            printDisplay("DHT11 OK", Config::Init::HARDWARE_OUTPUT_DELAY_MS, true);
        return true;
    }
}

bool dht11Init(bool boot)
{
    dht.begin();
    return dht11SelfTest(boot);
}

void getTemperatureCelsius(float &temperatureCelsius, float &tempCelsiusDs3231, float &tempCelsiusDht11)
{
    tempCelsiusDs3231 = Config::HardwareFeatures::DS3231 ? rtc.getTemperature() + Config::Sensor::MeasurmentOffset::TEMPERATURE_CELSIUS_DS3231: 0.0;
    tempCelsiusDht11 = Config::HardwareFeatures::DHT11 ? dht.readTemperature() + Config::Sensor::MeasurmentOffset::TEMPERATURE_CELSIUS_DHT11 : 0.0;
    if (Config::HardwareFeatures::DS3231 && Config::HardwareFeatures::DHT11)
        temperatureCelsius = (tempCelsiusDs3231 + tempCelsiusDht11) / 2;
    else if (!Config::HardwareFeatures::DS3231 && Config::HardwareFeatures::DHT11)
        temperatureCelsius = tempCelsiusDht11;
    else if (Config::HardwareFeatures::DS3231 && !Config::HardwareFeatures::DHT11)
        temperatureCelsius = tempCelsiusDs3231;
}

void getHumidityPercent(float &humidityPercent)
{
  humidityPercent = Config::HardwareFeatures::DHT11 ? dht.readHumidity() + Config::Sensor::MeasurmentOffset::HUMIDITY_PERCENT: 0.0;
}
