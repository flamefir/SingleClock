#include "WiFi.h"

class WifiConnect
{
private:
    const char* ssid_ = "TCHLWA2971"; 
    const char* password_ =  "qMF5T3EMmFhX4G2vjd";
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