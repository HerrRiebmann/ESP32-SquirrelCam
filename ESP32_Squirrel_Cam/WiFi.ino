void WiFiBegin() {
  //Manually change between WiFi and Accesspoint. AP will be used as a fallback, after 5 seconds
  if (Force_Accesspoint) {
    CreateAccessPoint();    
    WebserverBegin();
  }
  else
    ConnectToAccessPoint();
}

void WiFiStateLoop() {
  if (WiFiConnected || !Webserver_Enabled)
    return;

  if (WiFiStart == 0)
    WiFiStart = millis();

  WiFiConnected = WiFi.status() == WL_CONNECTED;

  if (WiFiConnected) {
    PrintMessage(F("WiFi "));
    PrintMessage(F("IP: "));
    PrintMessageLn(WiFi.localIP().toString());  //Show ESP32 IP on serial

    if (startWebserver)
      WebserverBegin();
    // Initialize time with timezone (try twice)
    if (!InitTime(myTimezone)) {
      delay(100);
      InitTime(myTimezone);
    }
    InitTelegramBot();
    SendStatus();
    ResetIdleTime();
    return;
  }

  if (millis() - WiFiStart > 10000) {
    PrintMessageLn("Wifi " + ssid + " not found!");
    CreateAccessPoint();    
    WebserverBegin();
  }
}

void ConnectToAccessPoint() {
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(deviceNameShort);
  WiFi.begin(ssid.c_str(), password.c_str());
}

void CreateAccessPoint() {
  WiFi.disconnect();
  IPAddress local_ip(8, 8, 8, 8);
  IPAddress gateway(8, 8, 8, 8);
  IPAddress subnet(255, 255, 255, 0);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(deviceNameShort);
  delay(500);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(500);

  PrintMessage(F("AP IP address: "));
  PrintMessageLn(WiFi.softAPIP().toString());


  /* Setup the DNS webServer redirecting all the domains to the apIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", local_ip);
  DnsStarted = true;
  WiFiConnected = true;
}
