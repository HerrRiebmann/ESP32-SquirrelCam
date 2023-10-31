void PrintMessage(const String &message) {
  if (Serial_Enabled)
    Serial.print(message);

  if (SerialData.length() < SerialDataMaxSize)
    SerialData.concat(message);
}

void PrintMessageLn() {
  if (Serial_Enabled)
    Serial.println();

  if (SerialData.length() < SerialDataMaxSize)
    SerialData.concat("\r");
}

void PrintMessageLn(const String &message) {
  PrintMessage(message);
  if (Serial_Enabled)
    Serial.println();

  if (SerialData.length() < SerialDataMaxSize)
    SerialData.concat("\r");
}

String GetCurrentTime() {
  struct tm timeinfo;
  if (!InitTime(myTimezone))
    return "";
  getLocalTime(&timeinfo);
  char timeString[20];
  strftime(timeString, sizeof(timeString), "%H:%M:%S - %d.%m.%y", &timeinfo);
  return String(timeString);
}

void SwitchOnBoardLed(bool state) {
  digitalWrite(LED_BUILTIN, state ? LOW : HIGH);
  PrintMessageLn(state ? "LED: On" : "LED: Off");
  LedState = state;
}

void CheckIdle() {
  if (photoWakeup && WiFiConnected) {
    savePhoto();
    if (lastPhotoFilename != "") {
      SendPhotoFromSD(lastPhotoFilename);
      photoWakeup = false;
      lastPhotoFilename = "";
    }
  }

  if (!skipDeepsleep)
    if (millis() - lastActionTime > idleTresholdSeconds * 1000)
      SentToDeepSleep();

  //5 Min on idle: Deepsleep!
  if (skipDeepsleep)
    if (millis() - lastActionTime > secondsToSleepOnIdle * 1000)
      SentToDeepSleep();
}

void ResetIdleTime() {
  lastActionTime = millis();
}

void TestSomething(String chatId) {
  String statusText;  
  uint16_t sec = GetSecondsToSleep();
  statusText = "Don´t wakup for: " + String((int)(sec / 60 / 60)) + ":" + String((int)(sec / 60 % 60));
  statusText += "\n" + String(sec) + "s";
  if(sec == secondsToSleep)
    statusText = "Time not in range!";
  bot.sendMessage(chatId, statusText, "");
}
