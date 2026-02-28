# esp8266-iot-gas-detector

## Electric schema

![./kicad/](kicad/gas_sensor.png)

## Libraries

All these libraries are required as dependencies and can be installed via the
Arduino IDE:

| Library name | Author |
|--------------|--------|
| [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306) | [Adafruit](https://www.adafruit.com/) |
| [DHT sensor library](https://github.com/adafruit/DHT-sensor-library) | [Adafruit](https://www.adafruit.com/) |
| [ArduinoJson](https://arduinojson.org) | [Benoît Blanchon](https://github.com/bblanchon) |
| [NTPClient](https://github.com/arduino-libraries/NTPClient) | [Fabrice Weinberg](https://github.com/arduino-libraries) |
| [WiFiManager](https://github.com/tzapu/WiFiManager) | [tzapu](https://github.com/tzapu/) |
| [Timezone](https://github.com/JChristensen/Timezone) | [Jack Christensen](https://github.com/JChristensen)
| [RTCLib](https://github.com/adafruit/RTClib) | [Adafruit](https://www.adafruit.com/) |
| [Uptime-Library](https://github.com/YiannisBourkelis/Uptime-Library) | [Yiannis Bourkelis](https://github.com/YiannisBourkelis) |

## Boards

### ESP8266 D1 mini

#### Special pin triggers

Connect the two pins with a wire to trigger the function.

| Positive lead pin name | Positive lead GPIO id | Ground pin name | Function name |
|------------------------|-----------------------|-----------------|---------------|
| D7 | 13 | GND | `selfTest()` |

#### Power usage

| Components | mA   |
|------------|-----:|
| All        | 165  |
| DS3231     | 7    |
| DHT11      | 2    |
| SSD1306    | 5    |
| MQ-2       | 133  |

### ESP32

The sketch compiles and can run on an ESP32, although it has been tested
without any sensor nor device. 

### IDE Setup

This project uses an ESP8266 board. Follow [these](https://arduino-esp8266.readthedocs.io/en/latest/installing.html) instructions to set it up.

### USB modem setup

#### Debian GNU/Linux 13

1. add your user to the dialout group

   ```shell
   sudo usermod -a -G dialout $USER
   ```

2. reboot

## Version

This project follows [Semver 2.0](https://semver.org/). Please look at the
[project_alpha/version.h](./project_alpha/version.h) file.

Bump the version by:

1. updating the version number in `./project_alpha/version.h`
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

