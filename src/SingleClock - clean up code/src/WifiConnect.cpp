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
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
      delay(10);
    }
    // Wait a bit before scanning again
    delay(5000);
  }
}

void WifiConnect::ConnectToWiFi()
{
  while (WiFi.status() != WL_CONNECTED)
  {
    // check if desired SSID is in the list of scanned networks
    for (int i = 0; i < networkList; i++)
    {
      if (WiFi.SSID(i) == "Flamefire" && WiFi.status() != WL_CONNECTED)
      {
        WiFi.begin("Flamefire", "12345678");
        WiFiConnectStatus("Flamefire", "12345678");
      }
      if (WiFi.SSID(i) == "NETGEAR19" && WiFi.status() != WL_CONNECTED)
      {
        WiFi.begin("NETGEAR19", "flamefire2.4");
        WiFiConnectStatus("NETGEAR19", "flamefire2.4");
      }
    }
  }
}

void WifiConnect::WiFiConnectStatus(const char *ssid, const char *password)
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
  Serial.println(" -- OK");
}

void WifiConnect::DisconnectFromWiFi()
{
  Serial.print("Disconnecting from ");
  Serial.print(ssid_);
  Serial.print(" ..");
  WiFi.disconnect();
  while (WiFi.status() != WL_DISCONNECTED)
  {
    Serial.print('.');
    delay(1000);
  }

  Serial.println();
  Serial.print(ssid_);
  Serial.print(" with IP: ");
  Serial.print(WiFi.localIP());
  Serial.println(" -- OK");
}