#ifndef BOARDS_ESP8266_H
#define BOARDS_ESP8266_H

struct Pins {
    struct Digital {
        /* + is D*, - is GND on the ESP8266 D1 mini. */
        static const int BUZZER = 0;            // GPIO  0: D3
        static const int DHT_11 = 12;           // GPIO 12: D6
        static const int MQ_2 = 14;             // GPIO 14: D5
        static const int SELF_TEST = 13;        // GPIO 13: D7
        static const int LED = LED_BUILTIN;     // GPIO 2:  D4
    };
    struct Analog {
        static const int RTC = 999;
        static const int MQ_2 = A0;
    };
};

#endif
