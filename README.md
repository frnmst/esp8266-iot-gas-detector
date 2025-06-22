# esp8266-iot-gas-detector

## Electric schema

![./kicad/](kicad/gas_sensor.png)

## Libraries

All these libraries are required as dependencies and can be installed via the
Arduino IDE:

- [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306)
- [DHT sensor library](https://github.com/adafruit/DHT-sensor-library)
- [ArduinoJson](https://arduinojson.org)
- [NTPClient](https://github.com/arduino-libraries/NTPClient)
- [WiFiManager](https://github.com/tzapu/WiFiManager)
- [Timezone](https://github.com/JChristensen/Timezone)
- [RTCLib](https://github.com/adafruit/RTClib)

## Version

This project follows [Semver 2.0](https://semver.org/). Please look at the
[src/version.h](./src/version.h) file.

Bump the version by:

1. updating the version number in `./src/version.h`
2. running `make`

## License

MIT License

Copyright (c) 2024-2025 [Franco Masotti](https://blog.franco.net.eu.org/about/#contacts)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
