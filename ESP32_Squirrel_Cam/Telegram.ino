void TelegramLoop() {
  if (!WiFiConnected)
    return;
  if (millis() - bot_lasttime > BOT_MTBS) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    bot_lasttime = millis();
  }
  if (sendToDeepsleepAfterMsgConfirm) {
    delay(50);
    SentToDeepSleep();
  }
}

void handleNewMessages(int numNewMessages) {
  PrintMessage("handleNewMessages: ");
  PrintMessageLn(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    currentChat_Id = chat_id;

    if (!BotUserIsAdmin(chat_id)) {
      bot.sendPhoto(chat_id, "https://i.kym-cdn.com/photos/images/newsfeed/000/631/254/eda.jpg", "Unauthorized user\nYou have no power here!");
      PrintMessageLn("Unauthorized user" + String(chat_id));

      bot.sendMessage(MY_CHAT_ID, "Unauthorized user " + bot.messages[i].from_name + " (" + String(chat_id) + ")\n" + text, "");

      if (!BotUserExists(chat_id.toInt()))
        AddBotUser(chat_id.toInt(), Undefined, bot.messages[i].from_name);
      continue;
    }

    String from_name = bot.messages[i].from_name;
    if (from_name == "")
      from_name = "Guest";

    if (text == "/flash")
    {
      flashState = !flashState;
      digitalWrite(FLASH_LED_PIN, flashState);
      String msg = "LED is *";
      msg += flashState ? "On*" : "Off*";
      bot.sendMessage(chat_id, msg, "MarkdownV2");
    }

    if (text == "/photo") {
      fb = NULL;
      // Take Picture with Camera
      fb = esp_camera_fb_get();
      if (!fb) {
        PrintMessageLn("Camera capture failed");
        bot.sendMessage(chat_id, "Camera capture failed", "");
        return;
      }
      sendPhotoTelegram();
      PrintMessageLn("done!");
      esp_camera_fb_return(fb);
    }

    if (text == "/wakeup") {
      SkipDeepsleep();
      bot.sendMessage(chat_id, "Squirrel Cam is alive!", "");
    }

    if (text == "/sleep") {
      bot.sendMessage(chat_id, "Good night!", "");
      if (WebServerStarted) {
        WebserverEnd();
        skipDeepsleep = true;
      }
      sendToDeepsleepAfterMsgConfirm = true;
    }

    if (text == "/webserver") {
      if (!WebServerStarted) {
        WebserverBegin();
        bot.sendMessage(chat_id, "Webserver started: http://" + WiFi.localIP().toString(), "");
        skipDeepsleep = true;
      }
      else {
        WebserverEnd();
        bot.sendMessage(chat_id, "Webserver stopped!", "");
        skipDeepsleep = true;
      }
    }

    if (text == "/pir") {
      PirActive = !PirActive;
      StoreSettings();
      bot.sendMessage(chat_id, "PIR is now " + String(!PirActive ? "de" : "") + "activated!", "");

    }

    if (text == "/last")
      SendLastPhoto();

    if (text == "/test")
      TestSomething(chat_id);

    if (text == "/state")
      SendStatus(chat_id, "Current ESP State");

    if (text == "/user")
      SendUser(chat_id);

    if (text == "/start" || text == "/info")    {
      String welcome = "Welcome to the *ESP32 Squirrel-Cam* Telegram Bot.\n\n";
      welcome += "/photo :  will take a photo\n";
      welcome += "/last :      sends last photo taken\n";
      welcome += "/flash :    toggle flash LED (_VERY BRIGHT!_)\n";
      welcome += "/state :    sends current ESP32 state\n";
      welcome += "/pir :       (de-)activate the presence detector\n";
      welcome += "/sleep :   send ESP into deepsleep\n";
      welcome += "/wakeup : waking ESP up, on next iteration\n";
      welcome += "/webserver : start (_or end_) to access images and settings\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }
  }
  ResetIdleTime();
}
String sendPhotoTelegram() {
  if (initTelegramTransfer()) {
    SendTelegramHeader(fb->len);
    SendTelegramPhotoBuffer();
    SendTelegramFooter();
    ReadTelegramResponse();
  }
  return "";
}

File myFile;
bool isMoreDataAvailable();
byte getNextByte();

bool isMoreDataAvailable()
{
  return myFile.available();
}
byte getNextByte()
{
  return myFile.read();
}
void SendPhotoFromSD(String filepath) {
  PrintMessageLn("Filetransfer: " + filepath);
  myFile = GetFS().open(filepath, FILE_READ);
  if (!myFile) {
    PrintMessageLn("Open file failed!");
    return;
  }

  if (initTelegramTransfer()) {
    SendTelegramHeader(myFile.size());
    SendTelegramPhotoFromFile();
    SendTelegramFooter();
    ReadTelegramResponse();
  }
  myFile.close();
}

void SendStatus() {
  if (CameraActivated && FSinitialized && TimeInitialized) {
    if (LastError != "") {
      LastError = "";
      StoreSettings();
    }
    return;
  }

  String CurrentError;
  if (!CameraActivated)
    CurrentError = "Camera " + String(CameraError);
  if (!FSinitialized)
    CurrentError = "SD-Card";
  if (!TimeInitialized)
    CurrentError = "Timeserver";

  if (CurrentError != "" && CurrentError != LastError)
    SendStatus(MY_CHAT_ID, "Init failed!");

  LastError = CurrentError;
  StoreSettings();
}
void SendStatus(String chatId, String Text) {
  String statusText;
  if (Text != "")
    statusText.concat("<strong><i>" + Text + "</i></strong>\n\n");
  statusText.concat("Webserver: <b>");
  statusText.concat(WebServerStarted ? "Started" : "Stopped");
  statusText.concat("</b>\n");
  statusText.concat("Camera: <b>");
  statusText.concat(CameraActivated ? "Activated" : "Failed");
  if (CameraError == ESP_OK)
    statusText.concat("</b>\n");
  else
    statusText.concat("</b> 0x" + String(CameraError, HEX) + "\n");
  statusText.concat("SD-Card: <b>");
  statusText.concat(FSinitialized ? "Initialized" : "Failed");
  statusText.concat("</b>\n");
  statusText.concat("PIR-Sensor: <b>");
  statusText.concat(PirActive ? "Active" : "Deactivated");
  statusText.concat("</b>\n");
  statusText.concat("Time: <b>");
  statusText.concat(TimeInitialized ? "Initialized" : "Failed");
  if (TimeInitialized)
    statusText.concat(" (" + GetCurrentTime() + ")");
  statusText.concat("</b>\n");
  statusText.concat("Wakeup-Time: <b>");
  statusText.concat(String(hourToKeepAwake) + " - " + String(hourToSleep));
  statusText.concat("</b> (" + String(GetSecondsToSleep()) + "s)\n");
  statusText.concat("Deepsleep: <b>");
  statusText.concat(skipDeepsleep ? "Skipped" : "Active");
  statusText.concat("</b>");
  statusText.concat(" (" + String(secondsToSleep) + "sec)");
  if (skipDeepsleep) {
    statusText.concat("\nAlive: <b>");
    statusText.concat(String((millis() - lastActionTime) / 1000) + "/" + String(secondsToSleepOnIdle));
    statusText.concat("</b>");
  }
  if (fb != NULL) {
    statusText.concat("\nFileBuffer: <b>");
    statusText.concat(fb->len);
    statusText.concat("</b>");
  }
  if (photoWakeup) {
    statusText.concat("\nPhoto: <b>");
    statusText.concat(lastPhotoFilename);
    statusText.concat("</b>");
  }
  if (LastError != "") {
    statusText.concat("\nLastError: <b>");
    statusText.concat(LastError);
    statusText.concat("</b>");
  }
  bot.sendMessage(chatId, statusText, "HTML");
  //bot.sendMessage(chatId, statusText, "Markdown");
}

void SendUser(String chatId) {
  String txt;
  txt.concat("<pre>|   Chat Id  |    Type    |    Name    | No |\n");
  txt.concat(     "|------------|------------|------------|----|\n");
  for (uint8_t i = 0; i < MAX_BOT_USER; i++)
    if (users[i].chatId > 0) {
      txt.concat("| ");
      for (uint16_t c = String(users[i].chatId).length(); c < 10; c++)
        txt.concat(" ");
      txt.concat(String(users[i].chatId));
      txt.concat(" | ");
      switch (users[i].userType) {
        case Admin:
          txt.concat("   Admin  ");
          break;
        case Subscriber:
          txt.concat("Subscriber");
          break;
        case Undefined:
          txt.concat(" Undefined");
          break;
        default:
          txt.concat("   Empty  ");
      }
      txt.concat(" | ");
      txt.concat(String(users[i].userName));
      if(strlen(users[i].userName) < 10)
        txt.concat(String("          ").substring(0, 10 - strlen(users[i].userName)));
      if (i < 10)
        txt.concat(" |  " + String(i));
      else
        txt.concat(" | " + String(i));
      txt.concat(" |\n");
    }
  txt.concat("</pre>");
  bot.sendMessage(chatId, txt, "HTML");
}
