/**
    OfficeDisplay

    320x240 LCD display controlled by messages via Telegram's Bot API.


  based on 
    Part of TelegramBotClient (https://github.com/schlingensiepen/TelegramBotClient)    
    Jörn Schlingensiepen <joern@schlingensiepen.com>
    Client's API:   https://schlingensiepen.github.io/TelegramBotClient/
    Telegram's API: https://core.telegram.org/bots/api
    OTA update: https://www.bakke.online/index.php/2017/06/02/self-updating-ota-firmware-for-esp8266/

    
*/
const int FW_VERSION = 102;


#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Adafruit_ILI9341.h>

#define FEATURE_SPIFFS                  false
#define TFT_CS D0  //for D1 mini or TFT I2C Connector Shield (V1.1.0 or later)
#define TFT_DC D8  //for D1 mini or TFT I2C Connector Shield (V1.1.0 or later)
#define TFT_RST -1 //for D1 mini or TFT I2C Connector Shield (V1.1.0 or later)
#define TS_CS D3   //for D1 mini or TFT I2C Connector Shield (V1.1.0 or later)


Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
boolean updated = false;

#include <time.h>
       time_t now = time(nullptr);
/*
 * stuff for storing credentials etc in eeprom
 */
 // Instantiate Wifi connection credentials
//const char* ssid     = "Tilt";
//const char* password = "Moesmate12345";
const char* ssid     = "China";
const char* password= "Moesmate";


#define FLASH_EEPROM_SIZE 4096
extern "C" {
#include "spi_flash.h"
}
extern "C" uint32_t _SPIFFS_start;
extern "C" uint32_t _SPIFFS_end;
extern "C" uint32_t _SPIFFS_page;
extern "C" uint32_t _SPIFFS_block;

struct SecurityStruct
{
  char          WifiSSID[32];
  char          WifiKey[64];
  char          WifiAPKey[64];
  char          ControllerUser[26];
  char          ControllerPassword[64];
  char          Password[26];
} SecuritySettings;

struct UserStruct
{
  char          botToken[50] = "835419670:AAEeYWW1SyQtnIrI3wF060D_mrmFTJQHK-0";
  char          occupant1[25]= "Dr. Martin Husovec";
  char          occupant2[25];
  char          owner1[20] = "Martin";
  char          owner2[20];
  char          boss[10]; 
} UserSettings;


// Instantiate Telegram Bot secure token
// This is provided by BotFather
//const String botToken = "835419670:AAEeYWW1SyQtnIrI3wF060D_mrmFTJQHK-0";
String botToken;

const char* fwUrlBase = "https://github.com/romix123/OfficeDisplay/";  // location of the firmware for OTA update

String occupantS1; // = "Dr. Martin Husovec";
String occupantS2;// = "Dr. Francisco Costa Cabral";
String ownerS1;// = "Martin";
String ownerS2; //= "Francisco";
String bossS = "Romix";
/*
 * 
 * stuff for OTA update
 * activated by message: update by user romix
 */
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
ESP8266WiFiMulti WiFiMulti;

/*
 * end update
 */

#include "icons/Info.h"

 #include "icons/home.h"
 #include "icons/sport.h"
 #include "icons/meeting.h"
 #include "icons/busy.h"
 #include "icons/confcallF.h"
 #include "icons/confcallM.h"
// #include "teach.h"
 #include "icons/faq.h"
 #include "icons/globe.h"
 #include "icons/sick.h"
 

 #include "icons/cursors.h"
 #include "icons/splash.h"
 #include "icons/splash285x43.h"

 #include "utils.h"
 #include "display.h"



#include <TelegramBotClient.h>

/** init stuff
 *  
 * change all of this for new module
 */





// Instantiate the ssl client used to communicate with Telegram's web API
WiFiClientSecure sslPollClient;

// Instantiate the client with secure token and client
TelegramBotClient client(
      UserSettings.botToken, 
      sslPollClient);

/*
 * color table
 * 
*/ 
#define romix_background 0xF79E //0xEFEBE9//0xecf0f1
#define romix_heading    0x0086 //0x000e25
#define romix_content    0x7CB4 //0x68828e
#define romix_separation 0xE73D  //0xd8dfe2
#define romix_accent     0x04B9 //0x0083be
#define romix_red        0x4180 // f44336
#define romix_green      0xB9A1 // 0x8bc34a



// Function called on receiving a message
void onReceive (TelegramProcessError tbcErr, JwcProcessError jwcErr, Message* msg)
{   String command, param;
    int len; 
    tft.setFont(&FreeSans9pt7b);
    tft.setTextColor(romix_content); 

 if ((msg->FromFirstName == bossS)&&(msg->Text.startsWith("~config"))) {
  command = getValue(msg->Text, ' ',1);
  param = getValue(msg->Text, ' ' ,2);
 
  len = param.length()+1;
  
  
  if (command == "bot") {
    param.toCharArray(UserSettings.botToken, len);
  } else if (command == "occupant1") {
        param.replace("_", " ");
        param.toCharArray(UserSettings.occupant1, len);
  } else if (command == "occupant2")  {
    param.replace("_", " ");
    param.toCharArray(UserSettings.occupant2, len);
  } else if (command == "owner1") {
    param.toCharArray(UserSettings.owner1, len);
  } else if (command == "owner2") {
    param.toCharArray(UserSettings.owner2, len);
  } else if (command == "done") {
    SaveSettings();
    LoadSettings();
    drawOccupant1();
    drawOccupant2();
    Serial.println("Settings saved");
  }else if (getValue(msg->Text, ' ',1) == "wipe") {
      ZeroFillFlash();
  } else if (getValue(msg->Text, ' ',1) == "update") {
      checkForUpdates();  
      }
      client.postMessage(msg->ChatId, msg->Text); 
  return;
 }
 
 if (msg->Text.startsWith("~icons")) {
  client.postMessage(msg->ChatId, "home, busy, travel, teach, sick, telco, meeting, welcome, sport"); 
  return;
 }
  
 if (msg->Text.startsWith("~?")) {
  client.postMessage(msg->ChatId, "bot, occupant1, occupant2, owner1, owner2, done, wipe, update"); 
  return;
 }
     
  if ((msg->FromFirstName == ownerS1)||(msg->FromFirstName == bossS)){
    if (msg->Text.startsWith("-")) {
      doIcon(msg->Text, 1); 
    } else {
      if (occupantS2 != "" ){
           tft.fillRect(0,25, 247, 99, romix_background);
      } else {
         tft.fillRect(0,25, 247, 240, romix_background);
        }
    tft.setCursor(0,42); 
    }
  } else 
   if ((msg->FromFirstName == ownerS2)||(msg->FromFirstName == bossS)){
    if (msg->Text.startsWith("-")) {
      doIcon(msg->Text, 2); 
    } else {
    tft.fillRect(0,146, 247, 240, romix_background);
    tft.setCursor(0,162); 
    }
   }
     drawMessage(msg->Text);
      if (occupantS2 != "" ){
            drawOccupant2();
      }
    // Sending the text of received message back to the same chat
    // chat is identified by an id stored in the ChatId attribute of msg
   // client.postMessage(msg->ChatId, msg->Text); 
}

void spinCursor(int i){
  switch (i){
    case 1: 
       tft.drawRGBBitmap(136, 160, wifi1, 48, 48);
        break;
    case 2: 
       tft.drawRGBBitmap(136, 160, wifi2, 48, 48);
        break;
    case 3: 
       tft.drawRGBBitmap(136, 160, wifi3, 48, 48);
        break;
    case 4: 
       tft.drawRGBBitmap(136, 160, wifi4, 48, 48);
        break;
 
    default:
    break;
  }
}

// Function called if an error occures
void onError (TelegramProcessError tbcErr, JwcProcessError jwcErr)
{
  Serial.println("onError");
  Serial.print("tbcErr"); Serial.print((int)tbcErr); Serial.print(":"); Serial.println(toString(tbcErr));
  Serial.print("jwcErr"); Serial.print((int)jwcErr); Serial.print(":"); Serial.println(toString(jwcErr));
}

void checkForUpdates() {
 // String mac = getMAC();
  String fwURL = String( fwUrlBase );
  fwURL.concat( "OfficeDisplay" );

  //https://raw.githubusercontent.com/romix123/OfficeDisplay/master/OfficeDisplay.version
//  String fwVersionURL = fwURL;
//  fwVersionURL.concat( ".version" );

String  fwVersionURL = "https://raw.githubusercontent.com/romix123/OfficeDisplay/master/OfficeDisplay.version";

  Serial.println( "Checking for firmware updates." );
 
  Serial.print( "Firmware version URL: " );
  Serial.println( fwVersionURL );

  HTTPClient httpClient;
  httpClient.begin( fwVersionURL );
  int httpCode = httpClient.GET();
  if( httpCode == 200 ) {
    String newFWVersion = httpClient.getString();

    Serial.print( "Current firmware version: " );
    Serial.println( FW_VERSION );
    Serial.print( "Available firmware version: " );
    Serial.println( newFWVersion );

    int newVersion = newFWVersion.toInt();

    if( newVersion > FW_VERSION ) {
      Serial.println( "Preparing to update" );

      String fwImageURL = "https://github.com/romix123/OfficeDisplay/raw/master/OfficeDisplay.bin"; //fwURL;
  //    fwImageURL.concat( ".bin" );
     
      t_httpUpdate_return ret = ESPhttpUpdate.update( fwImageURL );

      switch(ret) {
        case HTTP_UPDATE_FAILED:
          Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
          break;

        case HTTP_UPDATE_NO_UPDATES:
          Serial.println("HTTP_UPDATE_NO_UPDATES");
          break;
      }
    }
    else {
      Serial.println( "Already on latest version" );
    }
  }
  else {
    Serial.print( "Firmware version check failed, got HTTP response code " );
    Serial.println( httpCode );
  }
  httpClient.end();
}
//
//void update(){
//  if((WiFiMulti.run() == WL_CONNECTED)) {
//
//        t_httpUpdate_return ret = ESPhttpUpdate.update(updatefile);
//        //t_httpUpdate_return  ret = ESPhttpUpdate.update("https://server/file.bin");
//
//        switch(ret) {
//            case HTTP_UPDATE_FAILED:
//               Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
//                break;
//
//            case HTTP_UPDATE_NO_UPDATES:
//                Serial.println("HTTP_UPDATE_NO_UPDATES");
//                break;
//
//            case HTTP_UPDATE_OK:
//                Serial.println("HTTP_UPDATE_OK");
//                break;
//        }
//    }
//}

// Setup WiFi connection using credential defined at begin of file
void setupWiFi(){

     tft.drawRGBBitmap(17, 80, splashklein, 285, 43);
  
  Serial.println();
  Serial.printf("Try to connect to network %s ",ssid);
  Serial.println();
    int i = 0;
  WiFi.begin(ssid, password);
  Serial.print(".");
  while ((WiFi.status() != WL_CONNECTED) && ( i< 25)) {
    delay(500);
    Serial.print(".");
    spinCursor(i % 4);
    i++;
  }


//  if (! WL_CONNECTED) {
//        i = 0;
//        WiFi.begin(ssid1, password1);
//        tft.println();
//        tft.printf("Try to connect to network %s ",ssid1);
//        Serial.print(".");
//        while ((WiFi.status() != WL_CONNECTED) && ( i< 25)) {
//          delay(500);
//          Serial.print(".");
//          tft.print(".");
//          i++;
//        }
//  }
  Serial.println();
  tft.println();
  Serial.println("OK");
  tft.println("OK");
  Serial.print("IP address .: ");
  tft.print("IP address .: ");
  Serial.println(WiFi.localIP());
  tft.println(WiFi.localIP());

 
}

      
void setup() {
  
  Serial.begin(115200);
  LoadSettings();
  delay(10);
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(romix_background);
  tft.setCursor(0,0);
  
  setupWiFi();
   configTime(0, 0, "pool.ntp.org", "time.nist.gov");
   setenv("TZ", "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", 1);

    // Sets the functions implemented above as so called callback functions,
  // thus the client will call this function on receiving data or on error.
  client.begin(      
      onReceive,
      onError);    

   drawdisplay();
}

void loop() {
  // put your main code here, to run repeatedly:
    client.loop();
    delay(6000);
    now = time(nullptr);
    String time = String(ctime(&now));
    time.trim();
    Serial.println(time);
    if ((time.substring(11,14) == "18:53") && (!updated))  {
    Serial.println("Update");
  //  update();
    updated = true;
    ESP.reset();
  }


}
