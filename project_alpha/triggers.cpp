#include "triggers.h"

bool toggleDisplay()
{
    Config::HardwareFeatures::SSD1306 = !Config::HardwareFeatures::SSD1306;
    if (Config::HardwareFeatures::SSD1306)
    {
        prtn("initializing SSD1306");
        return ssd1306Init();
    }
    else
    {
        prtn("de-initializing SSD1306");
        ssd1306DeInit();
        return true;
    }
}

bool toggleShowCurrentTimeOnly()
{
    showCurrentTimeOnly = !showCurrentTimeOnly;
    return true;
}

bool togglePrintDebug()
{
    Config::Debug::PRINT = !Config::Debug::PRINT;

    if (Config::Debug::PRINT)
    {
        /* Serial might not be initialized here yet.
         * This happens when Config::Debug::PRINT is false at compile time.
         */
        if (!Serial)
        {
            Serial.begin(Config::Other::DEBUG_SPEED);
            Serial.flush();
        }
        
        prt("DEBUG: ");
        prtn("enabled");
    }

    return true;
}

bool toggleConfigPortal()
{
    bool offline = false;
    if (WiFi.status() == WL_CONNECTED)
    {
        /* IPv4 only. */
        char URL[48];
        IPAddress localIP = WiFi.localIP();
        snprintf(URL, sizeof(URL), "connect to:\nhttp://%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);
        prt(URL);
        printDisplay(URL, 10, false ,1);
    }
    else
    {
        offline = true;
        // FIXME
        // WiFi.mode(WIFI_AP);
        // WiFi.softAP(Config::Wifi::SSID, Config::Wifi::PASSWORD);
        // delay(100);
        /* FIXME: Params are not shown offline. */
        /* FIXME: User must manually type IP 192.168.4.1 instead of auto captive portal. */
        // std::vector<const char *> menu = {"wifi", "param", "info", "restart"};
        // wm.setMenu(menu);
    }

    updateConfigFromJsonConfigFile(LittleFS, filePath, configDoc);
    wm.startWebPortal();

    while (!shouldSaveConfig) { 
        wm.process();
        yield(); 
    }
    getCustomBoardSettingsWifiManager();
    if (shouldSaveConfig)
    {
        writeJsonConfigFileAndReloadConfiguration();
        shouldSaveConfig = false;
    }

    delay(1000);
    wm.stopWebPortal();

    /*
    if (offline)
    {
        WiFi.mode(WIFI_OFF);
    }
    */

    return true;
}

bool doOTA()
{
    ArduinoOTA.setPasswordHash(Config::OTA::Auth::PASSWORD_MD5_HASH);
    ArduinoOTA.setHostname(Config::OTA::Generic::HOSTNAME);

    ArduinoOTA.onStart([]() {
        const char *type;

        if (ArduinoOTA.getCommand() == U_FLASH)
            type = "sketch";
        else
            // U_FS
            type = "filesystem";

        // NOTE: if updating FS this would be the place to unmount FS using FS.end()
        prt("OTA start ");
        prtn(type);
        if (Config::HardwareFeatures::SSD1306)
        {
            char OTAStartString[20];
            snprintf(OTAStartString, sizeof(OTAStartString), "OTA\nstart\n%s", type);
            printDisplay(OTAStartString, 500, false);
            signal(1, false, 10);
        }
    });
    ArduinoOTA.onEnd([]() {
        prtn("OTA end");
        if (Config::HardwareFeatures::SSD1306)
        {
            printDisplay("OTA END", 500, true);
            signal(1);
        }
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        char pgr[10];
        int percentage = progress / (total / 100);
        snprintf(pgr, sizeof(pgr), "OTA %u%%", percentage);
        prtn(pgr);
        if (Config::HardwareFeatures::SSD1306)
            printDisplay(pgr, 10, true);
        signal(1, false, 10);
    });
    ArduinoOTA.onError([](ota_error_t error) {
        char otaError[32];
        const char *err;

        if (error == OTA_AUTH_ERROR)
            err = "Auth Failed";
        else if (error == OTA_BEGIN_ERROR)
            err = "Begin Failed";
        else if (error == OTA_CONNECT_ERROR)
            err = "Connect Failed";
        else if (error == OTA_RECEIVE_ERROR)
            err = "Receive Failed";
        else if (error == OTA_END_ERROR)
            err = "End Failed";

        snprintf(otaError, sizeof(otaError), "OTA error[%u]: ", error);
        prt(otaError);
        prtn(err);

        if (Config::HardwareFeatures::SSD1306)
        {
            printDisplay(otaError, 1000, false);
            printDisplay(err, 1000, false);
        }
    });

    ArduinoOTA.begin();

    unsigned long startOTA = millis();
    prtn("waiting 10m for OTA");
    if (Config::HardwareFeatures::SSD1306)
        printDisplay("OTA 10 MIN", 1000, true);

    while (true)
    {
        ArduinoOTA.handle();

        if (millis() - startOTA > 600000)
        {
            prtn("OTA update was not started");
            if (Config::HardwareFeatures::SSD1306)
                printDisplay("NO OTA", 1000, true);
            return false;
        }
        yield();
    }

    return true;
}

bool credits()
{
    printDisplay("BITMAP IMG");
    return true;
}

bool togglePomodoroTimer()
{
    prtn(pomodoroTimer);
    pomodoroTimer = !pomodoroTimer;
    pomodoroTimerStart = millis();
    prtn(pomodoroTimer);
    prtn(pomodoroTimerStart);
    return true;
}

bool noop()
{
    return true;
}