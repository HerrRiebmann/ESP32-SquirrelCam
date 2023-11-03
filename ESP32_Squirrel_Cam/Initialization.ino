void InitCam() {
  for (uint8_t counter = 0; counter <= 10; counter++) {
    CameraError = setupCamera(CamHighRes);
    if (CameraError == ESP_OK) {
      CameraActivated = true;
      return;
    }
    delay(200);
  }
  PrintMessage("Camera init failed with error 0x");
  PrintMessageLn(String(CameraError, HEX));
  CameraActivated = false;
}

// Function to set timezone
void setTimezone(String timezone) {
  Serial.printf("  Setting Timezone to %s\n", timezone.c_str());
  setenv("TZ", timezone.c_str(), 1); //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
  tzset();
}

// Connect to NTP server and adjust timezone
bool InitTime(String timezone) {
  struct tm timeinfo;
  if (!TimeInitialized) {
    PrintMessageLn("Setting up time");
    configTime(0, 0, "pool.ntp.org");    // First connect to NTP server, with 0 TZ offset
    if (!getLocalTime(&timeinfo)) {
      PrintMessageLn(" Failed to obtain time");
      return false;
    }
    PrintMessageLn("Got the time from NTP");
    // Now we can set the real timezone
    setTimezone(timezone);
    TimeInitialized = true;
  }
  getLocalTime(&timeinfo);
  char timeString[20];
  strftime(timeString, sizeof(timeString), "%H:%M:%S", &timeinfo);
  PrintMessageLn(timeString);
  return true;
}

void InitTelegramBot() {
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  // Make the bot wait for a new message for up to 60seconds
  //bot.longPoll = 60;
}

fs::FS GetFS() {
  switch (Filesystem) {
    case Spiffs:
    //return SPIFFS;
    case LittleFs:
    //return LITTLEFS;
    case Sd:
      return SD_MMC;
    default:
      //return SPIFFS;
      return SD_MMC;
  }
}
void InitFilesystem() {
  switch (Filesystem) {
    case None:
      FSinitialized = false;
      break;
    case Spiffs:
      FSinitialized = false;
      //FSinitialized = SPIFFS.begin(true);
      break;
    case LittleFs:
      FSinitialized = false;
      //true formats the FS
      //FSinitialized = LITTLEFS.begin(false);
      break;
    case Sd:
      FSinitialized = initMicroSDCard();;
      break;
  }
  if (!FSinitialized)
    PrintMessageLn("Filesystem " + GetFileSystemName() + " initialization failed!");
}
String GetFileSystemName() {
  switch (Filesystem) {
    case None:
      return "None";
    case Spiffs:
      return "Spiffs";
    case LittleFs:
      return "LittleFs";
    case Sd:
      return "Sd";
    default:
      return "Unknown";
  }
}
bool initMicroSDCard() {
  // Start Micro SD card
  PrintMessage("Starting SD Card: ");
  //Two less PINS when using "true". Allows to use PIR sensor
  //if(!SD_MMC.begin()){
  if (!SD_MMC.begin("/sd", true)) {
    PrintMessageLn("Mount Failed");
    return false;
  }
  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    PrintMessageLn("No SD Card attached");
    return false;
  }
  PrintMessageLn("Success!");
  return true;
}

void InitBotUser() {
  LoadBotUsers();
  if (!BotUserIsAdmin(MY_CHAT_ID))
    CreateAdminAccount();
}
