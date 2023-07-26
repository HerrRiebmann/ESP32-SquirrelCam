void SkipDeepsleep() {
  skipDeepsleep = true;
}

uint8_t GetWakeupReason() {
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  return wakeup_reason;
}

void print_wakeup_reason() {
  switch (GetWakeupReason())  {
    case ESP_SLEEP_WAKEUP_EXT0 : PrintMessageLn("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : PrintMessageLn("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : PrintMessageLn("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : PrintMessageLn("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : PrintMessageLn("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n", GetWakeupReason()); break;
  }
}

void CheckWakeupMode() {
  switch (GetWakeupReason())
  {
    case ESP_SLEEP_WAKEUP_EXT0 : //Wakeup caused by external signal using RTC_IO
    case ESP_SLEEP_WAKEUP_TOUCHPAD : //Wakeup caused by touchpad
    case ESP_SLEEP_WAKEUP_ULP : //Wakeup caused by ULP program
    case ESP_SLEEP_WAKEUP_EXT1 : //Wakeup caused by external signal using RTC_CNTL
      //Take a picture
      photoWakeup = true;
      takePhoto();
      idleTresholdSeconds = 20;
      break;
    case ESP_SLEEP_WAKEUP_TIMER : //Wakeup caused by timer
      idleTresholdSeconds = 30;
      break;
    default : //Wakeup was not caused by deep sleep: Reset/Startup      
      //Unintended start: Activate webserver (for at least a minute)
      startWebserver = true;
      idleTresholdSeconds = 60;
      break;
  }
}

void SentToDeepSleep() {
  skipDeepsleep = false;
  esp_sleep_enable_timer_wakeup((uint64_t)secondsToSleep * uS_TO_S_FACTOR);

  PrintMessageLn("Going to sleep now for " + String(secondsToSleep) + " Seconds");
  Serial.flush();

  if (WiFiConnected) {
    WiFi.disconnect();
  }
  SwitchOnBoardLed(LOW);
  delay(1000);
  esp_deep_sleep_start();
}

void SetupDeepSleep() {
  //Print the wakeup reason for ESP32
  print_wakeup_reason();

  //Pin 12 = RTC 15
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_12, TriggerState);
}
