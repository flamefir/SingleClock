#include <WifiConnect.hpp>

WifiConnect::WifiConnect(/* args */)
{
}

WifiConnect::~WifiConnect()
{
}

void WifiConnect::BeginWiFiConnection()
{
    WiFi.begin(ssid_, password_);
    while (WiFi.status() != WL_CONNECTED) 
    {
        digitalWrite(ledPin_, HIGH);
        delay(1000);
        digitalWrite(ledPin_, LOW);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println(WiFi.localIP());
}

void WifiConnect::ScanForWiFiSignal()
{
  // WiFi.scanNetworks will return the number of networks found
  networkList = WiFi.scanNetworks();
  Serial.println("scan done");
  if (networkList == 0) 
  {
    Serial.println("no networks found, retrying in 5 seconds");
  } 
  else 
  {
    Serial.print(networkList);
    Serial.println(" networks found");

    for (int i = 0; i < networkList; ++i) 
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
      delay(10);
    }
    // Wait a bit before scanning again
    delay(5000);
  }
}

void WifiConnect::ConnectToWiFi(const char *DesiredSSID, const char *PassPhrase)
{
  //check if desired SSID is in the list of scanned networks
  for (int i = 0; i < networkList; i++)
  {
    if(WiFi.SSID(i) == DesiredSSID)
    {
      WiFi.begin(DesiredSSID, PassPhrase);
    }
    if(WiFi.SSID(i) == "NETGEAR19")
    {
      WiFi.begin("NETGEAR19", "flamefire2.4");
      WiFiConnectStatus("NETGEAR19");
    }
    if(WiFi.SSID(i) == "NETGEAR19-5G")
    {
      WiFi.begin("NETGEAR19-5G", "flamefire5");
      WiFiConnectStatus("NETGEAR19-5G");
    }
    if(WiFi.SSID(i) == "Stofa24867")
    {
      WiFi.begin("Stofa24867", "stile13fancy15");
      WiFiConnectStatus("Stofa24867");
    }
  }
}

void WifiConnect::WiFiConnectStatus(const char* ssid)
{
  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.print(" ..");
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print('.');
    delay(1000);
  }
  
  Serial.println();
  Serial.print(ssid);
  Serial.print(" with IP: ");
  Serial.print(WiFi.localIP());
  Serial.print(" -- OK");
}