#ifndef BOARDS_ESP32_H
#define BOARDS_ESP32_H

#ifndef LED_BUILTIN
    #define LED_BUILTIN 2
#endif

/* Use GPIO pin ids.
 * GPIO ids are on the positive (+) side.
 */
struct Pins {
    struct Digital {
        static const int BUZZER = 18;
        static const int DHT_11 = 19;
        static const int MQ_2 = 21;
        static const int BUTTON_A = 22;
        static const int LED = LED_BUILTIN;
    };
    struct Analog {
        static const int RTC = 999;
        #if defined(A4)
            static const int MQ_2 = A4;
        #else
            static const int MQ_2 = 32;
        #endif
    };
};

#endif
