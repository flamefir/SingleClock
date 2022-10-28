#include "WiFi.h"

class WifiConnect
{
private:
    const char* ssid_ = "Stofa24867"; 
    const char* password_ =  "stile13fancy15";
    int networkList = 0;
    int ledPin_ = 2;
public:
    WifiConnect(/* args */);
    ~WifiConnect();
    void BeginWiFiConnection();
    void ScanForWiFiSignal();
    void ConnectToWiFi(const char *DesiredSSID, const char *PassPhrase);
    void WiFiConnectStatus(const char* ssid);
};