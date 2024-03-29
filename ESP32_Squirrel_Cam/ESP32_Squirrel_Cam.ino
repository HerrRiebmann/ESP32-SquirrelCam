//Board
//AI Thinker ESP32-CAM

//For OTA capablities:
//ESP32 Dev Module => With PSRAM active

//ToDo:
//Create a "Library":
//"Sketches\libraries\Credentials\Credentials.h" with #define WIFI_SSID, WIFI_PASSWORD, BOT_TOKEN & MY_CHAT_ID

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <Preferences.h>
#include <DNSServer.h>
#include "esp_camera.h"
#include "camera.h"
#include "FS.h"
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"
#include "time.h"
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "Credentials.h"

//#define TELEGRAM_DEBUG
//#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#define FLASH_LED_PIN 4
#define PIR_PIN 12
#define LED_BUILTIN 33
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define MAX_BOT_USER 20

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

WebServer webServer(80);
const byte DNS_PORT = 53;
DNSServer dnsServer;

bool WiFiConnected = false;
long WiFiStart;
bool DnsStarted = false;
bool WebServerStarted = false;
bool TimeInitialized = false;
bool StreamActive = false;
bool CameraActivated = false;
bool CamHighRes = true;
bool PirActive = true;
esp_err_t CameraError;

String ssid = WIFI_SSID;
String password = WIFI_PASSWORD;

//Over the Air Update
bool UploadIsOTA = false;
const char deviceNameShort[] = "ESP32 Squirrel Cam";

enum UserType {
  Empty,
  Admin,
  Subscriber,
  Undefined
};

typedef struct {
  unsigned long chatId;
  UserType userType;
  char userName[20];
} user;

user users[MAX_BOT_USER];

enum Filesystems {
  None,
  Spiffs,
  Sd,
  LittleFs
};
Filesystems Filesystem = Sd;
bool FSinitialized;

const char* TelegramDomain = "api.telegram.org";

// REPLACE WITH YOUR TIMEZONE STRING: https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
String myTimezone = "CET-1CEST,M3.5.0,M10.5.0/3";

//Settings
Preferences settings;
bool Serial_Enabled = true;
bool OTA_Enabled = true;
bool Webserver_Enabled = true;
bool Force_Accesspoint = false;

String SerialData = "";
const int SerialDataMaxSize = 1000;

String LastError = "";

const unsigned long BOT_MTBS = 1000; // mean time between scan messages
unsigned long bot_lasttime; // last time messages' scan has been done

bool flashState = LOW;

camera_fb_t *fb = NULL;

bool lastState = LOW;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
bool TriggerState = HIGH; //LOW; //HIGH for PIR, LOW for Button

bool skipDeepsleep = false;
uint16_t secondsToSleep = 2 * 60;
uint16_t secondsToSleepOnIdle = 5 * 60;
bool sendToDeepsleepAfterMsgConfirm = false;
int hourToKeepAwake = 7;
int hourToSleep = 21;
//*** IDLE check ***
unsigned long lastActionTime;
uint8_t idleTresholdSeconds = 30;

bool photoWakeup = false;
String lastPhotoFilename = "";
String currentChat_Id;
bool startWebserver = false;

bool LedState = false;

const String PictureFolder = "/Photos";

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector
  Serial.begin(115200);
  Serial.println();

  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, flashState);

  pinMode(PIR_PIN, INPUT_PULLUP);
  lastState = !digitalRead(PIR_PIN);
  pinMode(LED_BUILTIN, OUTPUT);

  //Save power to not run into 0x20004 Cam Init Error
  SwitchOnBoardLed(LedState);

  SetupDeepSleep();

  InitSettings();
  delay(100);
  InitFilesystem();
  delay(200);
  InitCam();
  if (Webserver_Enabled)
    WiFiBegin();
  if (OTA_Enabled)
    SetupOTA();

  InitBotUser();
  CheckWakeupMode();
  ResetIdleTime();
}

void loop() {
  if (!StreamActive) {
    WiFiStateLoop();
    WebserverHandle();
    DnsHandle();
    OTA_Handle();
    CheckIdle();
    CheckPIR();
    TelegramLoop();
  }
  else {
    WebserverHandle();
    StreamingLoop();
  }
}

void CheckPIR() {
  bool currentState = digitalRead(PIR_PIN);

  if (lastState == currentState) {
    lastDebounceTime = millis();
    return;
  }

  if (photoWakeup)
    return;

  if ((millis() - lastDebounceTime) > debounceDelay) {
    lastState = currentState;
    lastDebounceTime = millis();
  } else
    return;
  PrintMessage("PIR State changed: ");
  PrintMessageLn(currentState ? "High" : "Low");
  if (!PirActive)
    return;
  if (currentState == TriggerState)
    takeSavePhoto();
}
