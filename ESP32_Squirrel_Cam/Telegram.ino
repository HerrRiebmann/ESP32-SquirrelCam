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
    if (chat_id != String(MY_CHAT_ID)) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      PrintMessageLn("Unauthorized user" + String(chat_id));
      continue;
    }
    String text = bot.messages[i].text;
    currentChat_Id = chat_id;

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

    if (text == "/webserver"){
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
    
    if (text == "/last")
      SendLastPhoto();

    if (text == "/state")
      SendStatus(chat_id, "Current ESP State");

    if (text == "/start" || text == "/info")    {
      String welcome = "Welcome to the *ESP32 Squirrel-Cam* Telegram Bot.\n\n";
      welcome += "/photo :  will take a photo\n";
      welcome += "/last :      sends last photo taken\n";
      welcome += "/flash :    toggle flash LED (_VERY BRIGHT!_)\n";
      welcome += "/state :    sends current ESP32 state\n";      
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
  if (CameraActivated && FSinitialized && TimeInitialized)
    return;
  SendStatus(MY_CHAT_ID, "Init failed!");
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
  statusText.concat("</b>\n");
  statusText.concat("SD-Card: <b>");
  statusText.concat(FSinitialized ? "Initialized" : "Failed");
  statusText.concat("</b>\n");
  statusText.concat("Time: <b>");
  statusText.concat(TimeInitialized ? "Initialized" : "Failed");
  if (TimeInitialized)
    statusText.concat(" (" + GetCurrentTime() + ")");
  statusText.concat("</b>\n");
  statusText.concat("Deepsleep: <b>");
  statusText.concat(skipDeepsleep ? "Skipped" : "Active");
  statusText.concat("</b>");
  statusText.concat(" (" + String(secondsToSleep) + "sec)");
  if(skipDeepsleep){
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
  
  bot.sendMessage(chatId, statusText, "HTML");
  //bot.sendMessage(chatId, statusText, "Markdown");

  //Test:
  //  if (!WebServerStarted)
  //    WebserverBegin();
  //  skipDeepsleep = true;
}
