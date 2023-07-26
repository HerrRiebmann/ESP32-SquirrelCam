void SetupOTA() {
  ArduinoOTA.setHostname(deviceNameShort);
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else { // U_SPIFFS
      type = "filesystem";
      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      //SPIFFS.end();
    }
    PrintMessageLn("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    PrintMessageLn("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    char msgString[30];
    sprintf(msgString, "Progress: %u%%\r", (progress / (total / 100)));
    PrintMessageLn(String(msgString));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    char msgString[20];
    sprintf(msgString, "Error[%u]: ", error);
    PrintMessage(String(msgString));
    if (error == OTA_AUTH_ERROR) PrintMessageLn("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) PrintMessageLn("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) PrintMessageLn("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) PrintMessageLn("Receive Failed");
    else if (error == OTA_END_ERROR) PrintMessageLn("End Failed");
  });
  ArduinoOTA.begin();
}

void handle_update(HTTPUpload& upload) {
  if (upload.status == UPLOAD_FILE_START) {
    PrintMessageLn("Update: " + String(upload.filename.c_str()));
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    /* flashing firmware to ESP*/
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) { //true to set the size to the current progress
      //Kernel panic using upload.totalSize
      //PrintMessageLn("Update Success: " + upload.totalSize);
    } else {
      Update.printError(Serial);
    }
  }
}
void handle_update_finish() {
  uint16_t refreshdelay = 1000;
  PrintMessageLn("Update finish");
  webServer.sendHeader("Connection", "close");
  String response = "<html><head><title>OTA</title><meta http-equiv='refresh' content='" + String(refreshdelay / 100) + "; URL=/' /><script>";
  response.concat("setInterval(function(){document.getElementById('countdown').innerHTML -= 1;}, 1000);");
  response.concat("</script></head><body>");
  response.concat(Update.hasError() ? "Update failed!" : "Update OK");
  response.concat(" - redirect in <label id='countdown'>" + String(refreshdelay / 100) + "</label> Sec.</body></html>");
  if (Update.hasError())
    webServer.send(500, "text/html", response);
  else
    webServer.send(200, "text/html", response);
  delay(refreshdelay);
  ESP.restart();
}

void OTA_Handle() {
  if (OTA_Enabled)
    ArduinoOTA.handle();
}
