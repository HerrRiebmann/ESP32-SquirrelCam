// Get the picture filename based on the current ime
String getPictureFilename() {
  struct tm timeinfo;
  if (!InitTime(myTimezone))
    return "";
  getLocalTime(&timeinfo);
  char timeString[20];
  strftime(timeString, sizeof(timeString), "%Y-%m-%d_%H-%M-%S", &timeinfo);
  String filename = "/Photos/picture_" + String(timeString) + ".jpg";
  return filename;
}

// Take photo and save to microSD card
void takeSavePhoto() {
  takePhoto();
  savePhoto();
}

// Take photo and save to microSD card
void takePhoto() {
  // Take Picture with Camera
  //camera_fb_t * fb = esp_camera_fb_get();
  fb = NULL;
  // Take Picture with Camera
  fb = esp_camera_fb_get();

  //Uncomment the following lines if you're getting old pictures
  //esp_camera_fb_return(fb); // dispose the buffered image
  //fb = NULL; // reset to capture errors
  //fb = esp_camera_fb_get();

  if (!fb) {
    PrintMessageLn("Camera capture failed");
    delay(1000);
    ESP.restart();
  }
}
void savePhoto() {
  // Path where new picture will be saved in SD Card
  String path = getPictureFilename();
  Serial.printf("Picture file name: %s\n", path.c_str());
  if (!TimeInitialized) {
    PrintMessageLn("No time, cannot create filename!");
    SendStatus();
    return;
  }
  if (!WiFiConnected && photoWakeup)
    lastPhotoFilename = path;

  // Save picture to microSD card
  File file = GetFS().open(path.c_str(), FILE_WRITE);
  if (!file) {
    Serial.printf("Failed to open file in writing mode");
  }
  else {
    file.write(fb->buf, fb->len); // payload (image), payload length
    Serial.printf("Saved: %s\n", path.c_str());
  }
  file.close();
  if (WiFiConnected && !photoWakeup) {
    sendPhotoTelegram();
  }
  esp_camera_fb_return(fb);
}

void SendLastPhoto() {
  PrintMessageLn("Get Files:");
  File file = GetFS().open("/Photos");
  if (!file) {
    PrintMessageLn("Path not found!");
    return;
  }
  String newestFilename;
  time_t newestTime = 0;
  File entry =  file.openNextFile();

  while (entry) {
    if (!entry.isDirectory()) {
      String filename = entry.name();
      PrintMessageLn(filename);
      // /Photos/picture_2023-07-05_14-57-02.jpg
      filename.remove(0, 16);
      // 2023-07-05_14-57-02.jpg
      filename.remove(19, 4);
      PrintMessageLn(filename);
      // Convert the string to time
      struct tm tm;
      if (strptime(filename.c_str(), "%Y-%m-%d_%H-%M-%S", &tm))
      {
        time_t current = mktime(&tm);
        if (current > newestTime) {
          newestTime = current;
          newestFilename = entry.name();
        }
      }
    }
    entry =  file.openNextFile();
  }
  entry.close();
  file.close();
  PrintMessageLn();
  SendPhotoFromSD(newestFilename);
}
