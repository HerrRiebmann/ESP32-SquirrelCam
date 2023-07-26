const String boundary = "b0und4ry123";

bool initTelegramTransfer() {
  secured_client.stop();
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  secured_client.setTimeout(10000);
  PrintMessage("Connect to " + String(TelegramDomain));
  bool success = secured_client.connect(TelegramDomain, 443);
  PrintMessageLn(success ? " Sucessful" : " Failed");
  return success;
}

void SendTelegramHeader(size_t photoSize) {
  String head = "--" + boundary + "\r\nContent-Disposition: form-data; name=\"chat_id\"; \r\n\r\n" + String(MY_CHAT_ID) + "\r\n--" + boundary + "\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
  String tail = "\r\n--" + boundary + "--\r\n";

  unsigned long imageLen = photoSize;
  uint16_t extraLen = head.length() + tail.length();
  unsigned long totalLen = imageLen + extraLen;

  PrintMessageLn("FileSize: " + String(totalLen));

  secured_client.println("POST /bot" + String(BOT_TOKEN) + "/sendPhoto HTTP/1.1");
  secured_client.println("Host: " + String(TelegramDomain));
  secured_client.println("Connection: keep-alive");
  secured_client.println("Content-Type: multipart/form-data; boundary=" + boundary);
  secured_client.println("Content-Length: " + String(totalLen));
  secured_client.println();
  secured_client.print(head);
}

void SendTelegramPhotoBuffer() {
  uint8_t *fbBuf = fb->buf;
  size_t fbLen = fb->len;
  for (size_t n = 0; n < fbLen; n = n + 1024) {
    if (n + 1024 < fbLen) {
      secured_client.write(fbBuf, 1024);
      fbBuf += 1024;
    }
    else if (fbLen % 1024 > 0) {
      size_t remainder = fbLen % 1024;
      secured_client.write(fbBuf, remainder);
    }
  }
}

void SendTelegramPhotoFromFile() {
  if (myFile) {
    const size_t bufferSize = 1024;
    uint8_t buffer[bufferSize];
    while (myFile.available()) {
      size_t n = myFile.readBytes((char*) buffer, bufferSize);
      if (n != 0) {
        secured_client.write(buffer, n);
      } else {
        PrintMessageLn(F("Buffer was empty"));
        break;
      }
    }
  }
  else
    PrintMessageLn("No file opened!");
}

void SendTelegramFooter() {
  String tail = "\r\n--" + boundary + "--\r\n";
  secured_client.print(tail);
}

String ReadTelegramResponse() {
  String getAll = "";
  String getBody = "";
  int waitTime = 10000;   // timeout 10 seconds
  long startTimer = millis();
  boolean state = false;

  while ((startTimer + waitTime) > millis()) {
    Serial.print(".");
    delay(100);
    while (secured_client.available()) {
      char c = secured_client.read();
      Serial.print(String(c));
      if (state == true) getBody += String(c);
      if (c == '\n') {
        if (getAll.length() == 0) state = true;
        getAll = "";
      }
      else if (c != '\r')
        getAll += String(c);
      startTimer = millis();
    }
    if (getBody.length() > 0)
      break;
  }
  secured_client.stop();
  Serial.print(getAll);
  Serial.println(getBody);
  if ((startTimer + waitTime) < millis()) {
    PrintMessageLn("Image transfer to api.telegram.org failed (Timeout).");
    if (currentChat_Id.length() > 0)
      SendStatus(currentChat_Id, "Image transfer failed: " + getBody);
    else
      SendStatus(MY_CHAT_ID, "Image transfer failed: " + getBody);
  }
  currentChat_Id = "";
  return getBody;
}
