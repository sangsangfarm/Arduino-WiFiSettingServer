# Arduino-WiFi-Setting-Server

WiFi setting server in Arduino.

## Support platform

- esp8226
- esp32

## Usage

```cpp
char *ap_name = "my wifi setting server";
WiFiSettingServer server = WiFiSettingServer(ap_name);
void setup() {
  int eeprom_address = 1000;
  server.setEEPROMAddress(1000); //for loading wifi info
  server.begin();

  while(WiFi.status() != WL_CONNECTED)
    server.connectWiFi();
}
void loop() {
  //If wifi is disconnected, wifi setting server begin.
  if (WiFi.status() != WL_CONNECTED)
  {
    if (!server.isActive())
      server.begin();
    server.connectWiFi();
  }
}
```
