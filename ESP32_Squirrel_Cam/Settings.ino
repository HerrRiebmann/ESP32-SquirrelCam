void InitSettings() {
  //Readonly: false
  settings.begin("settings", false);
  //Variable Name: Max 15 chars!!!!
  Serial_Enabled = settings.getBool("Serial_Enabled", Serial_Enabled);
  OTA_Enabled = settings.getBool("OTA_Enabled", OTA_Enabled);
  Webserver_Enabled = settings.getBool("WS_Enabled", Webserver_Enabled);
  Force_Accesspoint = settings.getBool("Force_AP", Force_Accesspoint);

  ssid = settings.getString("ssid", ssid);
  password = settings.getString("password", password);

  CamHighRes = settings.getBool("highRes", CamHighRes);  
  secondsToSleep = settings.getInt("deepSleep", secondsToSleep);
  PirActive = settings.getBool("PirActive", PirActive);

  hourToSleep = settings.getInt("hourToSleep", hourToSleep);
  hourToKeepAwake = settings.getInt("hourToKeepAwake", hourToKeepAwake);

  settings.end();
  PrintMessageLn("Setting Loaded");
  PrintSettings();
}

void StoreSettings() {
  settings.begin("settings", false);
  settings.putBool("Serial_Enabled", Serial_Enabled);
  settings.putBool("OTA_Enabled", OTA_Enabled);
  settings.putBool("WS_Enabled", Webserver_Enabled);
  settings.putBool("Force_AP", Force_Accesspoint);

  settings.putString("ssid", ssid);
  settings.putString("password", password);

  settings.putBool("highRes", CamHighRes);  
  settings.putInt("deepSleep", secondsToSleep);
  settings.putBool("PirActive", PirActive);    
  settings.putInt("hourToSleep", hourToSleep);
  settings.putInt("hourToKeepAwake", hourToKeepAwake);

  settings.end();
  PrintMessageLn("Settings Stored");
  PrintSettings();
}

void PrintSettings() {
  if (!Force_Accesspoint) {
    PrintMessage("SSID: ");
    PrintMessageLn(ssid);
  }
  else
    PrintMessageLn("AccessPoint");
  PrintMessage("High Resolution: ");
  PrintMessageLn(CamHighRes ? "True" : "False");
  PrintMessage("DeepSleep: ");
  PrintMessageLn(String(secondsToSleep));
}
