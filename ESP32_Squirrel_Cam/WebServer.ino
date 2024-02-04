void WebserverBegin() {
  webServer.enableCORS(); //Allow use fetch-api
  webServer.on("/setup", handle_setup);
  webServer.on("/base", handle_base);
  webServer.on("/led", handle_led);
  webServer.on("/wifi", handle_wifi);
  webServer.on("/restart", handle_restart);
  webServer.on("/espinfo", handle_esp);
  webServer.on("/serial", handle_serial);
  webServer.on("/upload", HTTP_POST, handle_upload_finish, handle_upload);
  webServer.on("/images", handle_images);
  webServer.on("/flash", handle_flash);
  webServer.on("/sleep", handle_sleep);
  webServer.on("/photo", handle_photo);
  webServer.on("/user", handle_user);
  webServer.on("/mjpeg/1", HTTP_GET, handle_jpg_stream);
  webServer.on("/jpg", HTTP_GET, handle_jpg);

  //Allways redirect to captive portal. Request comes with IP (8.8.8.8) or URL (connectivitycheck.XXX / captive.apple / etc.)
  webServer.on("/generate_204", redirect);    //Android captive portal.
  webServer.on("/fwlink", redirect);   //Microsoft captive portal.

  webServer.on("/connecttest.txt", redirect); //www.msftconnecttest.com
  webServer.on("/hotspot-detect.html", redirect); //captive.apple.com

  webServer.on("/success.txt", handle_success); //detectportal.firefox.com/sucess.txt
  webServer.onNotFound(handleFileRead);

  const char* Headers[] = {"If-None-Match"};
  webServer.collectHeaders(Headers, sizeof(Headers) / sizeof(Headers[0]));

  webServer.begin();
  PrintMessageLn(F("HTTP webServer started"));
  delay(100);
  WebServerStarted = true;
}

void WebserverEnd() {
  webServer.stop();
  PrintMessageLn(F("HTTP webServer stopped"));
  WebServerStarted = false;
}

void WebserverHandle() {
  if (Webserver_Enabled)
    webServer.handleClient();
}

void DnsHandle() {
  if (DnsStarted)
    dnsServer.processNextRequest();
}

void handle_root() {
  // /index.html
  PrintMessageLn(F("Handle Root"));

  PrintIncomingRequest();

  String path = "/index.html";

  File f = GetFS().open(path, FILE_WRITE);
  if (!f)
    return;
  webServer.streamFile(f, "text/html");
  f.close();
}

void handle_setup() {
  // /setup
  PrintMessageLn(F("Handle Setup"));

  //With arguments:
  // /setup?x=123&y=321&inv=0&ap=1
  ProcessSetupArguments();

  String txt = "";
  txt.concat(String(Serial_Enabled));
  txt.concat("|");
  txt.concat(String(OTA_Enabled));
  txt.concat("|");
  txt.concat(String(Webserver_Enabled));
  txt.concat("|");
  txt.concat(String(Force_Accesspoint));
  txt.concat("|");
  txt.concat(ssid);
  txt.concat("|");
  txt.concat(password);
  webServer.send(200, "text/plain", txt);
}

void handle_base() {
  PrintMessageLn(F("Handle Base"));
  ProcessSetupArguments();

  String txt = String(CamHighRes);
  txt.concat("|");
  txt.concat(String(skipDeepsleep));
  txt.concat("|");
  txt.concat(String(secondsToSleep));
  txt.concat("|");
  txt.concat(String(PirActive));
  txt.concat("|");
  txt.concat(String(hourToSleep));
  txt.concat("|");
  txt.concat(String(hourToKeepAwake));
  PrintMessageLn(txt);
  webServer.send(200, "text/plain", txt);
}

void handle_led() {
  PrintMessageLn(F("Handle Led"));
  ProcessSetupArguments();
  SwitchOnBoardLed(!LedState);
  webServer.send(200, "text/plain", String(LedState));
}

void handle_flash() {
  PrintMessageLn(F("Handle Flash"));
  ProcessSetupArguments();
  flashState = !flashState;
  digitalWrite(FLASH_LED_PIN, flashState);
  webServer.send(200, "text/plain", String(LedState));
}

void handle_sleep() {
  PrintMessageLn(F("Handle Sleep"));
  ProcessSetupArguments();
  webServer.send(200, "text/plain", "Good night!");
  delay(100);
  SentToDeepSleep();
}

void handle_photo() {
  PrintMessageLn(F("Handle Photo"));
  ProcessSetupArguments();
  //Don´t send Message to Telegram
  photoWakeup = true;
  takeSavePhoto();
  photoWakeup = false;
  webServer.send(200, "text/plain", "OK");
}

void handle_user() {
  PrintMessageLn(F("Handle User"));
  //user?chatId=123456789&userType=1

  ProcessSetupArguments();

  String txt;
  for (uint8_t i = 0; i < MAX_BOT_USER; i++)
    if (users[i].chatId > 0) {
      txt.concat(String(users[i].chatId));
      txt.concat("|");
      txt.concat(String(users[i].userType));
      txt.concat("|");
      txt.concat(String(users[i].userName));
      txt.concat(";");
    }
  webServer.send(200, "text/plain", txt);
}

void handle_wifi() {
  int n = WiFi.scanNetworks(false, false); //WiFi.scanNetworks(async, show_hidden)
  String temp;
  for (int i = 0; i < n; i++) {
    temp += WiFi.SSID(i) + ";";
    temp += " (" + GetEncryptionType(WiFi.encryptionType(i)) + ")";
    temp += "|";
  }
  webServer.send(200, "text/plain", temp);
}

void handle_restart() {
  webServer.send(200, "text/plain", "OK");
  delay(500);
  ESP.restart();
}

void handleNotFound() {
  PrintMessageLn(F("HandleNotFound"));

  PrintIncomingRequest();

  if (captivePortal())
    return;
  String message = F("File Not Found\n\n");
  message += F("URI: ");
  message += webServer.uri();
  message += F("\nMethod: ");
  message += (webServer.method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += webServer.args();
  message += F("\n");

  for (uint8_t i = 0; i < webServer.args(); i++) {
    message += String(F(" ")) + webServer.argName(i) + F(": ") + webServer.arg(i) + F("\n");
  }
  webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  webServer.sendHeader("Pragma", "no-cache");
  webServer.sendHeader("Expires", "-1");
  webServer.send(404, "text/plain", message);
}

void handle_success() {
  PrintMessageLn(F("Handle success.txt"));
  webServer.send(200, "text/plain", "success");
}

void handle_upload_finish() {
  if (UploadIsOTA)
    handle_update_finish();
  else
    webServer.send(200);
}

void handle_upload() {
  HTTPUpload& upload = webServer.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    UploadIsOTA = filename.endsWith(".bin");
  }
  if (UploadIsOTA)
    handle_update(upload);
  else
    handle_fileupload(upload);
}

#include <rom/rtc.h>
const char* const PROGMEM flashChipMode[] = {"QIO", "QOUT", "DIO", "DOUT", "Unbekannt"};
const char* const PROGMEM resetReason[] = {"ERR", "Power on", "Unknown", "Software", "Watch dog", "Deep Sleep", "SLC module", "Timer Group 0", "Timer Group 1",
                                           "RTC Watch dog", "Instrusion", "Time Group CPU", "Software CPU", "RTC Watch dog CPU", "Extern CPU", "Voltage not stable", "RTC Watch dog RTC"
                                          };
void handle_esp() {
  String temp = "CPU Temp;" + String(temperatureRead()) +
                "|Runtime;" + runtime(millis()) +
                "|Build;" +  (String)__DATE__ + " " + (String)__TIME__ +
                "|SketchSize;" + formatBytes(ESP.getSketchSize()) +
                "|SketchSpace;" + formatBytes(ESP.getFreeSketchSpace()) +
                "|LocalIP;" +  WiFi.localIP().toString() +
                "|Hostname;" + WiFi.getHostname() +
                "|SSID;" + WiFi.SSID() +
                "|RSSI;" + WiFi.RSSI() +
                "|GatewayIP;" +  WiFi.gatewayIP().toString() +
                "|Channel;" +  WiFi.channel() +
                "|MacAddress;" +  WiFi.macAddress() +
                "|SubnetMask;" +  WiFi.subnetMask().toString() +
                "|BSSID;" +  WiFi.BSSIDstr() +
                "|ClientIP;" + webServer.client().remoteIP().toString() +
                "|DnsIP;" + WiFi.dnsIP().toString() +
                "|ChipModel;" + ESP.getChipModel() +
                "|Reset1;" + resetReason[rtc_get_reset_reason(0)] +
                "|Reset2;" + resetReason[rtc_get_reset_reason(1)] +
                "|CpuFreqMHz;" + ESP.getCpuFreqMHz() +
                "|HeapSize;" + formatBytes(ESP.getHeapSize()) +
                "|FreeHeap;" + formatBytes(ESP.getFreeHeap()) +
                "|MinFreeHeap;" + formatBytes(ESP.getMinFreeHeap()) +
                "|ChipSize;" +  formatBytes(ESP.getFlashChipSize()) +
                "|ChipSpeed;" + ESP.getFlashChipSpeed() / 1000000 +
                "|ChipMode;" + flashChipMode[ESP.getFlashChipMode()] +
                "|IdeVersion;" + ARDUINO +
                "|SdkVersion;" + ESP.getSdkVersion();
  webServer.send(200, "text/plain", temp);
}

void handle_serial() {
  ResetIdleTime();
  webServer.send(200, "text/plain", SerialData);
  SerialData = "";
}
void handle_images() {
  PrintMessageLn(F("Handle Images"));
  ProcessSetupArguments();
  sendImages();
}

boolean captivePortal() {
  PrintMessage(F("Captive Check: "));
  PrintMessageLn(webServer.hostHeader());
  if (!isIp(webServer.hostHeader())) {
    PrintMessageLn("-Request redirected to captive portal");
    redirect();
    return true;
  }
  return false;
}

void redirect() {
  webServer.sendHeader("Location", String("http://") + toStringIp(webServer.client().localIP()), true);
  webServer.send(302, "text/plain", "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
  webServer.client().stop(); // Stop is needed because we sent no content length
}

void PrintIncomingRequest() {
  ResetIdleTime();

  PrintMessageLn(webServer.hostHeader());
  PrintMessage("  ");
  PrintMessageLn(webServer.uri());

  for (uint8_t i = 0; i < webServer.args(); i++)
    PrintMessageLn(String(F(" ")) + webServer.argName(i) + F(": ") + webServer.arg(i));

  for (int i = 0; i < webServer.headers(); i++)
    PrintMessageLn(String(F("\t")) + webServer.headerName(i) + F(": ") + webServer.header(i));
}

boolean isIp(String str) {
  PrintMessage("-IsIP: ");
  for (size_t i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      PrintMessageLn("false");
      return false;
    }
  }
  PrintMessageLn("true");
  return true;
}

void ProcessSetupArguments() {
  ResetIdleTime();

  unsigned long chatId = 0;
  int userType = 0;
  String userName;

  bool valuesChanged = false;
  for (uint8_t i = 0; i < webServer.args(); i++) {
    PrintMessageLn(String(F(" ")) + webServer.argName(i) + F(": ") + webServer.arg(i));

    if (webServer.argName(i).compareTo(F("serial_enabled")) == 0)
      Serial_Enabled = webServer.arg(i) == "1";
    if (webServer.argName(i).compareTo(F("ota_enabled")) == 0)
      OTA_Enabled = webServer.arg(i) == "1";
    if (webServer.argName(i).compareTo(F("webserver_enabled")) == 0)
      Webserver_Enabled = webServer.arg(i) == "1";
    if (webServer.argName(i).compareTo(F("force_accesspoint")) == 0)
      Force_Accesspoint = webServer.arg(i) == "1";

    if (webServer.argName(i).compareTo(F("ssid")) == 0)
      ssid = webServer.arg(i);
    if (webServer.argName(i).compareTo(F("password")) == 0)
      password = webServer.arg(i);

    if (webServer.argName(i).compareTo(F("highRes")) == 0)
      CamHighRes = webServer.arg(i) == "1";
    if (webServer.argName(i).compareTo(F("skipDeepsleep")) == 0)
      skipDeepsleep = webServer.arg(i) == "1";
    if (webServer.argName(i).compareTo(F("deepSleep")) == 0)
      secondsToSleep = webServer.arg(i).toInt();
    if (webServer.argName(i).compareTo(F("pirActive")) == 0)
      PirActive = webServer.arg(i) == "1";
    if (webServer.argName(i).compareTo(F("hourToSleep")) == 0)
      hourToSleep = webServer.arg(i).toInt();
    if (webServer.argName(i).compareTo(F("hourToKeepAwake")) == 0)
      hourToKeepAwake = webServer.arg(i).toInt();

    if (webServer.argName(i).compareTo(F("chatId")) == 0)
      chatId = webServer.arg(i).toInt();
    if (webServer.argName(i).compareTo(F("userType")) == 0)
      userType = webServer.arg(i).toInt();
    if (webServer.argName(i).compareTo(F("userName")) == 0)
      userName = webServer.arg(i);

    valuesChanged = true;
  }
  if (valuesChanged)
    StoreSettings();
  if (chatId > 0)
    ProcessBotUser(chatId, userType, userName);
}

String toStringIp(IPAddress ip) {
  PrintMessageLn("IptoString");
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}
String GetEncryptionType(byte thisType) {
  // read the encryption type and print out the name:
  switch (thisType) {
    case 0 :
      return "OPEN";
    case 1 :
      return "WEP";
    case 2 :
      return "WPA_PSK";
    case 3 :
      return "WPA2_PSK";
    case 4 :
      return "WPA_WPA2_PSK";
    case 5 :
      return "WPA2_ENTERPRISE";
    case 6 :
      return "AUTH_MAX";
  }
  return "Unknown: " + String(thisType);
}
String runtime(uint32_t currentMillis) {
  uint32_t sec {(currentMillis / 1000)};
  char buf[20];
  if (sec / 86400 > 1)
    snprintf(buf, sizeof(buf), "%d Tag%s %02d:%02d:%02d", sec / 86400, sec < 86400 || sec >= 172800 ? "e" : "", sec / 3600 % 24, sec / 60 % 60, sec % 60);
  else
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d", sec / 3600 % 24, sec / 60 % 60, sec % 60);
  return buf;
}
