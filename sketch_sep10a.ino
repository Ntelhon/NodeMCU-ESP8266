#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "FirebaseESP8266.h"
#include "ESP8266WiFi.h"
#include "ESP8266HTTPClient.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define FIREBASE_HOST "SECRET"
#define FIREBASE_AUTH "SECRET"
#define HOST "api.thingspeak.com"
#define GOLD_API "/apps/thinghttp/send_request?api_key=AV3DJQZBR45DDR39"
#define DOLLAR_API "/apps/thinghttp/send_request?api_key=LVD54TEKHC318MB4"
#define WIFI_SSID "SECRET"
#define WIFI_PASSWORD "SECRET"

unsigned long lastTime = 0;
int resetTime;

FirebaseData lcdData1;
FirebaseData lcdData2;
FirebaseData lcdDataChange;
FirebaseData updateData;

HTTPClient http;
WiFiClient client;

String clearData(String data){
  String out = data;
  out.replace("\\\"","");
  return out;
}

String isSpecialWord(String data){
  if(data == "altin" || data == "Altin" || data == "gold" || data == "Gold" || data == "altın" || data == "Altın"){

    if(http.begin(client, HOST, 80, GOLD_API)){
      int httpCode = http.GET();

      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY){
        String goldData = "Altin: " + http.getString();
        return goldData;
      }
      else{
        return "Api hatasi";
      }
    }
    else{
      return "Apiye ulasilamadi";
    }
  }

  if(data == "dolar" || data == "Dolar" || data == "dollar" || data == "Dollar"){

    if(http.begin(client, HOST, 80, DOLLAR_API)){
      int httpCode = http.GET();

      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY){
        String dollarData = "Dolar: " + http.getString();
        return dollarData;
      }
      else{
        return "Api hatasi";
      }
    }
    else{
      return "Apiye ulasilamadi";
    }
  }

  else{
    return data;
  }
}

void setup(){
  Serial.begin(9600);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Aga baglaniliyor ");

  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(200);
  }
  Serial.println("/n/r Baglandi!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  lcd.init();

  lcd.backlight();
  lcd.home();
  lcd.print(WiFi.localIP());

  Firebase.setString(lcdDataChange, "/lcd-string-change", "1");

  if(Firebase.getString(updateData, "/lcd-update-time")){
    resetTime = updateData.stringData().toInt();
    Serial.println("Check interval: " + (char)resetTime);
  }
}

void loop(){

  unsigned long currentMillis = millis();
  if(currentMillis >= resetTime * 1000 + lastTime){
    lastTime = currentMillis;
    Serial.println("xd");
    Firebase.setString(lcdDataChange, "/lcd-string-change", "1");
  }

  while(!client.connect(HOST, 80)){
    client.connect(HOST, 80);
    delay(150);
  }

  if(Firebase.getString(lcdDataChange, "/lcd-string-change")){
    while(lcdDataChange.stringData() == "1"){
      if(Firebase.getString(lcdData1, "/lcd-first-row")){
        lcd.setCursor(0, 0);
        lcd.print("                ");
        lcd.setCursor(0, 0);


        // Data will come with \"example\" style, so data have to be cleared.
        String fixData = clearData(lcdData1.stringData());

        String controlledData = isSpecialWord(fixData);

        Serial.println(controlledData);
        lcd.print(controlledData);
        
        Firebase.setString(lcdDataChange, "/lcd-string-change", "0");
      }

      if(Firebase.getString(lcdData2, "/lcd-second-row")){
        lcd.setCursor(0, 1);
        lcd.print("                ");
        lcd.setCursor(0, 1);

        String fixData = clearData(lcdData2.stringData());

        String controlledData = isSpecialWord(fixData);
        
        Serial.println(controlledData);
        lcd.print(controlledData);

        Firebase.setString(lcdDataChange, "/lcd-string-change", "0");
      }

      if(Firebase.getString(updateData, "/lcd-update-time")){
        resetTime = updateData.stringData().toInt();
        Serial.println("NEW check interval: " + (char)resetTime);
      }

      else{
        Serial.println(lcdData1.errorReason());
        Serial.println(lcdData2.errorReason());
        Firebase.getString(lcdDataChange, "/lcd-string-change");
      }
    }
  }
}
