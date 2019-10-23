/**
 * @file WiFiSettingServer.h
 * @brief WiFi 설정 서버
 * @date 2019-08-27
 * @author Janghun Lee (jhlee@sangsang.farm)
 */

#ifndef WIFI_SETTING_SERVER_H_
#define WIFI_SETTING_SERVER_H_

#include <EEPROM.h>
#include <ESPAsyncWebServer.h>
#include <string.h>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <FS.h>
#elif defined(ESP32)
#include <SPIFFS.h>
#include <WiFi.h>
#endif

#ifndef EEPROM_SIZE
#define EEPROM_SIZE 4096
#endif

// Debug
#ifndef WIFI_SETTING_SERVER_DEBUG_PORT
#define WIFI_SETTING_SERVER_DEBUG_PORT Serial
#endif

#ifdef WIFI_SETTING_SERVER_DEBUG
#define print(...) WIFI_SETTING_SERVER_DEBUG_PORT.print(__VA_ARGS__)
#define printf(...) WIFI_SETTING_SERVER_DEBUG_PORT.printf(__VA_ARGS__)
#define println(...) WIFI_SETTING_SERVER_DEBUG_PORT.println(__VA_ARGS__)

#else
#define print(...)
#define printf(...)
#define println(...)
#endif

/** @def CHAR_ARRAY_LENGTH
 *  WiFi SSID, PW 글자 크기 */
#define CHAR_ARRAY_LENGTH 128

/** @def MAX_TRY
 *  WiFi 연결 최대 시도 수 */
#define MAX_TRY 99999999
/**
 * @struct WiFiInfo
 * @brief WiFi 정보
 * @date 2019-08-27
 * @author Janghun Lee (jhlee@sangsang.farm)
 */
typedef struct WiFiInfo {
  /** WiFi 이름*/
  char ssid[CHAR_ARRAY_LENGTH];
  /** WiFi 비밀번호*/
  char password[CHAR_ARRAY_LENGTH];
} WiFiInfo;

/**
 * @brief WiFi 설정 서버
 * @date 2019-08-27
 * @author Janghun Lee (jhlee@sangsang.farm)
 */
class WiFiSettingServer {
 private:
  char *_ap_name;
  bool _is_active = false;
  bool _is_get_wifi_info = false;
  int _wifi_connect_try = 0;
  int _eeprom_address = 0;

  WiFiInfo _wifi_info;
  void _connectWiFi(int connect_try);
  void saveWiFiInfo(void);
  void loadWiFiInfo(void);
  void end(void);

 public:
  WiFiSettingServer(char *ap_name);
  void setEEPROMAddress(int eeprom_address);
  void begin(void);
  void connectWiFi(void);
  void connectWiFi(int duration);
  bool isActive(void);
};
#endif /*WIFI_SETTING_SERVER_H_*/