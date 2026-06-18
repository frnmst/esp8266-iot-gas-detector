#include "network_utils.h"

bool reconnectWifi()
{
    bool reconnect = false;

    /* See also
     * https://github.com/tzapu/WiFiManager/issues/1588#issue-1670081323
     */
    wm.setConfigPortalTimeout(0);

    prtn("WiFi is disconnected");
    if (Config::HardwareFeatures::SSD1306)
        printDisplay("NO WiFi\nretrying...");
    if (WiFi.reconnect())
    {
        prtn("starting WiFi reconnection...");
        reconnect = true;
    }
    if (Config::HardwareFeatures::SSD1306)
        clearDisplay();

    checkWifi = 1;

    return reconnect;
}

bool connectAutoWifi()
{
    if (Config::Wifi::Auto::RESET_WIFI_SETTINGS)
        wm.resetSettings();

    wm.setConfigPortalTimeout(
        Config::Wifi::WIFIMANAGER_CONFIG_PORTAL_TIMEOUT_SECONDS
    );

    defineCustomBoardSettingsWifiManager();

    if (Config::HardwareFeatures::SSD1306)
    {
        char accessPointString[64];

        printDisplay("WiFi\nauto\nconfig\n", 250);
        snprintf(accessPointString, sizeof(accessPointString), "WiFi\naccess\npoint\nfor %ds", Config::Wifi::WIFIMANAGER_CONFIG_PORTAL_TIMEOUT_SECONDS);
        printDisplay(accessPointString, 250);
        snprintf(accessPointString, sizeof(accessPointString), "NW: %s\nPW: %s", Config::Wifi::SSID, Config::Wifi::PASSWORD);
        printDisplay(accessPointString, 250);
    }

    // Config access point.
    bool wifi = wm.autoConnect(Config::Wifi::SSID, Config::Wifi::PASSWORD);
    getCustomBoardSettingsWifiManager();

    if (shouldSaveConfig)
    {
        writeJsonConfigFileAndReloadConfiguration();
        shouldSaveConfig = false;
    }

    if (wifi)
    {
        prtn("connected to WiFi");
        if (Config::HardwareFeatures::SSD1306)
            printDisplay("connected to WiFi", 100);
        signal(1);
    }
    else
    {
        prtn("unable to connect to WiFi");
        if (Config::HardwareFeatures::SSD1306)
            printDisplay("unable to connect to WiFi");
        signal(2);
    }

    /* Save updated variables from WiFiManager
     * in flash and reload them immediately in RAM. */
    if (shouldSaveConfig)
        writeJsonConfigFileAndReloadConfiguration();

    return wifi;
}

bool connectManualWifi()
{
    if (Config::HardwareFeatures::SSD1306)
        printDisplay("WiFi\nmanual\nWM config", 100);

    // Imposta l'IP statico nel WiFiManager
    wm.setSTAStaticIPConfig(
        Config::Wifi::Manual::LOCAL_IP, 
        Config::Wifi::Manual::GATEWAY, 
        Config::Wifi::Manual::SUBNET, 
        Config::Wifi::Manual::DNS
    );

    // Tentativo di connessione diretta con le credenziali salvate o fornite
    // Se IGNORE_MISSING_WIFI_ON_MANUAL è attivo, limitiamo i tentativi
    if (Config::Wifi::IGNORE_MISSING_WIFI_ON_MANUAL) {
        wm.setConnectTimeout(10); // Timeout in secondi prima di fallire
    }

    // Prova a connettersi. Se fallisce, non avvia l'AP automatico ma ritorna false.
    if (!wm.preloadWiFi(Config::Wifi::SSID, Config::Wifi::PASSWORD)) {
        prtn("Connessione fallita.");
        signal(2);
        return false;
    }

    signal(1);
    return true;
}