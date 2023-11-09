# ESP32-SquirrelCam
ESP32 Cam Modul as a Squirrel camera trap

A PIR detects movement and takes a picture. This will be stored onto the SD-Card.
Afterwards, it will be transferred to a Telegram Bot. This Bot also can control the ESP32.

> :warning: **Attention:** The Cam does not work properly with the ESP32 Firmware above 2.0.X!!!
Streaming reboots the ESP, image is blue and blurry, connection is not reliable.
Use 1.0.6 instead.


### Description
Some squirrels were walking over my balcony, the other day. So my first intention was to make friends with and feed them.
So I bought a feeding house, which should prevent the nuts from being eaten by birds:

![SquirrelHouse](/Images/SquirrelHouse.png)

Then I wanted to know when a squirrel appeard. Already got a ESP32 Cam and the idea was born!
![TelegramBot](/Images/ESP32CamCase.png)

The next idea was, how can I actively be notified, when something happens? Push-Notifications were not that easy.
You need an app, a subscription or some complicated tooling. Or use a simple messenger!
A Telegram-Bot can do everything I need!


![TelegramBot](/Images/Telegram%20Bot.png)

Commands will be received by the ESP32 on every wakeup. With them, it´s possible to skipt the deepsleep, take a picture, start/stop the webserver and so on.

The last thing was to have a web interface, to control some settings, see or delete images and make updates.

![WebinterfaceGif](/Images/Squirrel%20Cam%20Webinterface.gif)

**Upload** takes any file and put it onto the root of the SD-Card. If the file ends with *.html, *.js or *.css it automatically be routed into the /web/ -folder.

*.bin-Files make an OTA Update and resets the ESP afterwards. The website should refresh automatically.

## Installation
* Prepare the Arduino IDE for ESP32 Boards (_not latest, better below 2.X_).
* Copy the /web/-Folder onto the SD-Card.
* Open ESP32_Squirrel_Cam.ino
* Install missing Libraries (_see [below](#Libraries)_)
* Create an own Library (_see [below](#CustomLibrary)_)
* Select Board "ESP32 Dev Module" => With PSRAM active (_Needed to use OTA, which won´t work with AI Thinker ESP32-CAM Board_)
* Upload it onto the ESP32 Cam (_with a programmer or the Cam-Board you can purchase with the Cam-Module_)

## Hardware
### Components
* AI Thinker ESP32-CAM
* HC-SR501 (_PIR-Sensor_)
* ESP32-CAM Board (_Programmer Board_)
* Powersupply

### Wiring
ESP 32 | Powersupply
------- | --------
5V | VCC
GND | GND

ESP 32 | PIR
------- | --------
5V | VCC
PIN12 | OUT
GND | GND

### Libraries
* [UniversalTelegramBot](https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot) (1.3.0)
* ArduinoJson (6.21.3)

### CustomLibrary
To prevent me from uploading my credentials to this repository, I´ve created a Library with just my personal informations:

**Sketches\libraries\Credentials\Credentials.h**

```
#pragma once

#define WIFI_SSID "MyWiFiName"
#define WIFI_PASSWORD "MyWiFiP4$$w0rd"

//Telegram Squirrel Bot
#define BOT_TOKEN "0123456789:AbCdEfGhIjKlMnOpQrStUvWxYz"
#define MY_CHAT_ID "9876543210"
```

### OTA (Over the Air Update)
You should see the ESP32 in Arduino IDE under Tools -> Port -> Network-Interfaces (ESP32 Squirrel Cam at _IP-Adress_)
For more information see [RandomNerdTutorials](https://randomnerdtutorials.com/esp32-over-the-air-ota-programming/)

### Telegram Bot
To create a Telegram Bot, you need to follow some steps:
1. Search for the so called "BotFather" and write ```/start```
2. Then you will see all possible commands and select or write ```/newbot```
3. Now enter the name, which must contain "Bot", like ```MyVeryPersonalBot```
4. At last, add a username.

Afterwards, you receive a Token, which must be stored. And the Bot-URL. You can add the Bot to a Chat by following this URL.
Ensure to setup the Bot to not be added by groups and set some security settings! Your Bot can be found public, but since you only process and send to/from your own personal chat id, no strangers can interact.
It´s also recommended to also set the command-list with the BotFather. Then you will have a menu with command buttons within your chat.
I can recommend to follow this [RandomNerdTutorial](https://randomnerdtutorials.com/telegram-control-esp32-esp8266-nodemcu-outputs/)

### ToDo
- [x] Order Gallery
- [ ] Do not load all images at once in gallery
- [x] Chat Id management to allow others to see images
- [ ] Prevent sunlight to trigger (by optimizing PIR or on software side)
- [ ] Etag webfiles / Immutable photo
