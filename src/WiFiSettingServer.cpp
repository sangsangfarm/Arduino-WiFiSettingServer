/**
 * @file WiFiSettingServer.cpp
 * @brief WiFi 설정 서버
 * @date 2019-08-27
 * @author Janghun Lee (jhlee@sangsang.farm)
 */

#include <WiFiSettingServer.h>

AsyncWebServer _server(80);

/**
 * @fn WiFiSettingServer::WiFiSettingServer(char *ap_name)
 * @brief WiFiSettingServer 생성자
 * @param ap_name WiFi 설정 서버 AP명
 * @return WiFi 설정 서버 클래스
 * @date 2019-08-27
 * @author Janghun Lee (jhlee@sangsang.farm)
 */
WiFiSettingServer::WiFiSettingServer(char *ap_name) { _ap_name = ap_name; }

/**
 * @fn void WiFiSettingServer::saveWiFiInfo(void)
 * @brief EEPROM 데이터 저장
 * @return void
 * @date 2019-08-27
 * @author Janghun Lee (jhlee@sangsang.farm)
 */
void WiFiSettingServer::saveWiFiInfo(void) {
  printf("[WiFiSettingServer] wifi 정보 EEPROM 주소: %d\n", _eeprom_address);
  EEPROM.put(_eeprom_address, _wifi_info);
  EEPROM.commit();
}

/**
 * @fn void WiFiSettingServer::loadWiFiInfo(void)
 * @brief EEPROM 데이터 불러오기
 * @return void
 * @date 2019-08-27
 * @author Janghun Lee (jhlee@sangsang.farm)
 */
void WiFiSettingServer::loadWiFiInfo(void) {
  printf("[WiFiSettingServer] wifi 정보 EEPROM 주소: %d\n", _eeprom_address);
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(_eeprom_address, _wifi_info);
  if (_wifi_info.ssid != NULL && _wifi_info.ssid[0] != '\0') {
    _is_get_wifi_info = true;
  }
  printf("[WiFiSettingServer] SSID: %s / PW: %s\n", _wifi_info.ssid,
         _wifi_info.password);
}

/**
 * @fn void WiFiSettingServer::setEEPROMAddress(int wifi_info_address)
 * @brief EEPROM 데이터 주소 설정
 * @param eeprom_address 설정할 EEPROM 데이터 주소
 * @return void
 * @date 2019-08-27
 * @author Janghun Lee (jhlee@sangsang.farm)
 */
void WiFiSettingServer::setEEPROMAddress(int eeprom_address) {
  _eeprom_address = eeprom_address;
}

/**
 * @fn void WiFiSettingServer::begin(void)
 * @brief WiFi 설정 서버 시작
 * @return void
 * @date 2019-08-27
 * @author Janghun Lee (jhlee@sangsang.farm)
 */
void WiFiSettingServer::begin(void) {
  println(F("[WiFiSettingServer] WiFi 설정 서버 시작"));
  if (!SPIFFS.begin(true)) {
    println(F("[WiFiSettingServer] SPIFFS를 마운팅하는 중 에러 발생"));
    return;
  }
  loadWiFiInfo();
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(_ap_name);

  IPAddress ip(192, 168, 0, 4);
  IPAddress mask(255, 255, 255, 0);
  WiFi.softAPConfig(ip, ip, mask);

  int net_count = WiFi.scanComplete();
  if (net_count == -2) {
    WiFi.scanNetworks(true);
  } else if (net_count) {
  }
  // favicon.ico file
  _server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/favicon.ico", "image/x-icon");
  });
  // style.css file
  _server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });
  // refresh.png file
  _server.on("/refresh.png", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/refresh.png", "image/png");
  });

  // WIFI 정보 저장 페이지
  _server.on("/", HTTP_GET, [&](AsyncWebServerRequest *request) {
    WiFi.disconnect();
    int net_count = WiFi.scanComplete();
    // WiFi 스캔 제대로 될때 까지 찾음
    while (net_count < 0) {
      if (net_count == -2) {
        WiFi.scanNetworks(true);
      }
      net_count = WiFi.scanComplete();
    }
    String ssid;
    String content = "<!DOCTYPE html><html><head>";
    content +=
        "<meta name='viewport' content='width=device-width, initial-scale=1' "
        "/>";
    content +=
        "<meta charset='UTF-8' /><link rel='shortcut icon' href='/favicon.ico' "
        "type='image/x-icon'/>";
    content += "<link rel='icon' href='/favicon.ico' type='image/x-icon'/>";
    content += "<link rel='stylesheet' type='text/css' href='/style.css'>";
    content += "<title>WiFi-Station</title>";
    content += "<script>function checkSSID() { ";
    content += "var formEl = document.forms.wifiForm;";
    content += "var formData = new FormData(formEl);";
    content += "var name = formData.get('ssid');";
    content += "if (name === 'Select your Wi-Fi') {";
    content += "alert('Please select your Wi-Fi.'); ";
    content += "return false; }";
    content += "}</script>";
    content += "</head><body><div class='container'>";
    content += "<div class='title'>";
    content += "<h1 style='text-align: center;'>WiFi-Station</h1>";
    content +=
        "<button class='wifi-reload' onclick='window.location.reload(); return "
        "false;'></button>";
    content += "</div>";
    content += "<form method='post' action='/save' id='wifiForm'>";
    content += "<div class='field-group'>";
    content += "<select name='ssid'>";
    content += "<option>Select your Wi-Fi</option>";
    printf("[WiFiSettingServer] network count: %d\n", net_count);
    for (int i = 0; i < net_count; ++i) {
      ssid = WiFi.SSID(i);
      content += "<option>";
      content += ssid;
      content += "</option>";
    }
    WiFi.scanDelete();
    content +=
        "<input name='password' type='text' length='64' "
        "onkeyup='inputKeyUp(event)' placeholder='Password' "
        "autocomplete='off'>";
    content += "</div><div class='button-container'>";
    content +=
        "<button type='submit' onclick='return checkSSID()'>Save</button>";
    content += "</div></form></div></body></html>";
    request->send(200, "text/html", content);
  });

  // WiFi 저장 확인 페이지
  _server.on("/save", HTTP_GET, [&](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/save.html", "text/html");
    _wifi_connect_try = MAX_TRY;  //기존 연결 중인 WiFi 종료 flag
  });

  // WiFi 정보 저장 페이지
  _server.on("/save", HTTP_POST, [&](AsyncWebServerRequest *request) {
    if (request->hasParam("ssid", true) &&
        request->hasParam("password", true)) {
      AsyncWebParameter *_ssid, *_password;
      _ssid = request->getParam("ssid", true);
      _password = request->getParam("password", true);

      // WiFi 정보 초기화
      memset(_wifi_info.ssid, 0, CHAR_ARRAY_LENGTH);
      memset(_wifi_info.password, 0, CHAR_ARRAY_LENGTH);

      // WiFi 정보 저장
      strncpy(_wifi_info.ssid, _ssid->value().c_str(), _ssid->value().length());
      strncpy(_wifi_info.password, _password->value().c_str(),
              _password->value().length());
      printf("[WiFiSettingServer] [%s] - [%s]\n", _wifi_info.ssid,
             _wifi_info.password);
      _is_get_wifi_info = true;
      saveWiFiInfo();

      // WiFi 정보 저장 확인 페이지로 리다이렉팅
      request->redirect("/save");
    }
  });

  _server.begin();
  _is_active = true;
}

/**
 * @fn void WiFiSettingServer::end(void)
 * @brief WiFi 설정 서버 종료
 * @return void
 * @date 2019-08-27
 * @author Janghun Lee (jhlee@sangsang.farm)
 */
void WiFiSettingServer::end(void) {
  println(F("[WiFiSettingServer] WiFi 설정 서버 종료"));
  SPIFFS.end();
  // _server.end();
  WiFi.mode(WIFI_STA);
  _is_active = false;
  _is_get_wifi_info = false;
}
/**
 * @fn void WiFiSettingServer::connectWiFi(void)
 * @brief 60초간 WiFi 연결 시도
 * @return void
 * @date 2019-08-27
 * @author Janghun Lee (jhlee@sangsang.farm)
 */
void WiFiSettingServer::connectWiFi(void) { connectWiFi(60); }

/**
 * @fn void WiFiSettingServer::connectWiFi(int duration)
 * @brief WiFi 연결
 * @param duration WiFi 연결 할 시간(초)
 * @return void
 * @date 2019-08-27
 * @author Janghun Lee (jhlee@sangsang.farm)
 */
void WiFiSettingServer::connectWiFi(int duration) {
  while (WiFi.status() != WL_CONNECTED) {
    _connectWiFi(duration);
    delay(1000);
  }
}

/**
 * @fn void WiFiSettingServer::_connectWiFi(int connect_try)
 * @brief WiFi 연결
 * @param connect_try WiFi 연결 시도 횟수
 * @return void
 * @date 2019-08-27
 * @author Janghun Lee (jhlee@sangsang.farm)
 */
void WiFiSettingServer::_connectWiFi(int connect_try) {
  println(F("[WiFiSettingServer] WiFi 연결시작"));
  if (_is_get_wifi_info) {
    WiFi.disconnect();
    WiFi.begin(_wifi_info.ssid, _wifi_info.password);
    WiFi.setHostname(_ap_name);
    uint8_t mac[6];
    WiFi.macAddress(mac);
    _wifi_connect_try = 0;
    while ((WiFi.status() != WL_CONNECTED)) {
      if (_wifi_connect_try == 0) {
        printf(
            "[WiFiSettingServer] 디바이스(MAC address: "
            "%02x:%02x:%02x:%02x:%02x:%02x)가 WiFi %s에 연결 시도.\r\n",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], _wifi_info.ssid);
        print(F("[WiFiSettingServer] WiFi 연결중."));
      }
      if (_wifi_connect_try > connect_try * 10) {
        print(F("\r\n"));
        break;
      }
      delay(100);
      _wifi_connect_try++;
      print(F("."));
    }
    if (WiFi.status() == WL_CONNECTED) {
      printf("[WiFiSettingServer] WiFI %s에 연결됨.\r\n", _wifi_info.ssid);
      end();
    }
  } else if (!_is_active) {
    begin();
  }
}

/**
 * @fn void WiFiSettingServer::_connectWiFi(int connect_try)
 * @brief WiFi 설정 서버 작동 체크
 * @return WiFi 설정 서버 작동 유무 (true : On | false :Off)
 * @date 2019-08-27
 * @author Janghun Lee (jhlee@sangsang.farm)
 */
bool WiFiSettingServer::isActive(void) {
  println(F("[WiFiSettingServer] WiFi 연결시작"));
  return _is_active;
}