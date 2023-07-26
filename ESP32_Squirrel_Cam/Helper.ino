void PrintMessage(const String &message) {
  if (Serial_Enabled)
    Serial.print(message);

  if(SerialData.length() < SerialDataMaxSize)
    SerialData.concat(message);
}

void PrintMessageLn() {
  if (Serial_Enabled)
    Serial.println();

  if(SerialData.length() < SerialDataMaxSize)
    SerialData.concat("\r");
}

void PrintMessageLn(const String &message) {
  PrintMessage(message);
  if (Serial_Enabled)
    Serial.println();

  if(SerialData.length() < SerialDataMaxSize)
    SerialData.concat("\r");
}

String GetCurrentTime(){
  struct tm timeinfo;
  if(!InitTime(myTimezone))
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
