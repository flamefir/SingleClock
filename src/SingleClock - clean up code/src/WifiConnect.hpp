#include "WiFi.h"

class WifiConnect
{
private:
    int networkList = 0;
    int ledPin_ = 2;

public:
    WifiConnect(/* args */);
    ~WifiConnect();
    void BeginWiFiConnection();
    void ScanForWiFiSignal();
    void ConnectToWiFi();
    void WiFiConnectStatus(const char *ssid, const char *password);
    void DisconnectFromWiFi();
    const char *ssid_ = "Flamefire";
    const char *password_ = "12345678";
};