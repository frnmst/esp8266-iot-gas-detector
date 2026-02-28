#ifndef BOARDS_ESP32_H
#define BOARDS_ESP32_H

#ifndef LED_BUILTIN
    #define LED_BUILTIN 2
#endif

struct Pins {
    struct Digital {
        static const int BUZZER = 18;           // GPIO 18: D18
        static const int DHT_11 = 19;           // GPIO 19: D19
        static const int NOSE = 21;             // GPIO 14: D21
        static const int SELF_TEST = 22;        // GPIO 13: D22
        static const int LED = LED_BUILTIN;     // GPIO 2:  D2
    };
    struct Analog {
        static const int RTC = 999;
        #if defined(A4)
            static const int NOSE = A4;             // GPIO 32: D32 (ACD1_4)
        #else
            static const int NOSE = 32;
        #endif
    };
};

#endif
