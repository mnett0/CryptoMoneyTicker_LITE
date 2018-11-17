/*
 *  /$$$$$$                                  /$$               /$$      /$$                                      /$$$$$$$$ /$$           /$$                                 /$$       /$$   /$$              
 * /$$__  $$                                | $$              | $$$    /$$$                                     |__  $$__/|__/          | $$                                | $$      |__/  | $$              
 * | $$  \__/  /$$$$$$  /$$   /$$  /$$$$$$  /$$$$$$    /$$$$$$ | $$$$  /$$$$  /$$$$$$  /$$$$$$$   /$$$$$$  /$$   /$$| $$    /$$  /$$$$$$$| $$   /$$  /$$$$$$   /$$$$$$       | $$       /$$ /$$$$$$    /$$$$$$ 
 * | $$       /$$__  $$| $$  | $$ /$$__  $$|_  $$_/   /$$__  $$| $$ $$/$$ $$ /$$__  $$| $$__  $$ /$$__  $$| $$  | $$| $$   | $$ /$$_____/| $$  /$$/ /$$__  $$ /$$__  $$      | $$      | $$|_  $$_/   /$$__  $$
 * | $$      | $$  \__/| $$  | $$| $$  \ $$  | $$    | $$  \ $$| $$  $$$| $$| $$  \ $$| $$  \ $$| $$$$$$$$| $$  | $$| $$   | $$| $$      | $$$$$$/ | $$$$$$$$| $$  \__/      | $$      | $$  | $$    | $$$$$$$$
 * | $$    $$| $$      | $$  | $$| $$  | $$  | $$ /$$| $$  | $$| $$\  $ | $$| $$  | $$| $$  | $$| $$_____/| $$  | $$| $$   | $$| $$      | $$_  $$ | $$_____/| $$            | $$      | $$  | $$ /$$| $$_____/
 * |  $$$$$$/| $$      |  $$$$$$$| $$$$$$$/  |  $$$$/|  $$$$$$/| $$ \/  | $$|  $$$$$$/| $$  | $$|  $$$$$$$|  $$$$$$$| $$   | $$|  $$$$$$$| $$ \  $$|  $$$$$$$| $$            | $$$$$$$$| $$  |  $$$$/|  $$$$$$$
 *  \______/ |__/       \____  $$| $$____/    \___/   \______/ |__/     |__/ \______/ |__/  |__/ \_______/ \____  $$|__/   |__/ \_______/|__/  \__/ \_______/|__/            |________/|__/   \___/   \_______/
 *                      /$$  | $$| $$                                                                      /$$  | $$                                                                                           
 *                     |  $$$$$$/| $$                                                                     |  $$$$$$/                                                                                           
 *                      \______/ |__/                                                                      \______/                                                                   
 *  
 * GitHub --> https://git.io/fxRYm
 *
 *  The MIT License (MIT)
 *
 *  Copyright (c) 2018 Médéric NETTO
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <Arduino.h>
#include <ArduinoJson.h>

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WifiManager.h>
#include <WiFiClientSecure.h>

#include <Wire.h>
#include <U8x8lib.h>

#include <Bounce2.h>

#include "myBitmaps.h"

U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE, 4, 5);

#define BUTTON_A 14 
#define BUTTON_B 12 
#define BUTTON_C 13  

const char host[] = "api.coinmarketcap.com";
const int httpsPort = 443;

unsigned long previousMillis = 0;
long interval = 0;

#define NUM_BUTTONS 3
const uint8_t BUTTON_PINS[NUM_BUTTONS] = {BUTTON_A, BUTTON_B, BUTTON_C};
Bounce * buttons = new Bounce[NUM_BUTTONS];

bool btnState1      = LOW;
bool btnState2      = LOW;
bool btnState3      = LOW;
bool antiFlickering = LOW;

// https://api.coinmarketcap.com/v2/listings/ for find the {id} of the currency
// Change the name of the currency and put the {id} in " "
#define BITCOIN     "1"
#define ETHEREUM "1027"
#define RIPPLE     "52"
#define LITECOIN    "2" 
#define DASH      "131" 

// and change again the name here
String crypto[] = {BITCOIN, ETHEREUM, RIPPLE, LITECOIN, DASH};
int coin = -1;
String oldPrice[5];


void setup() {

  Serial.begin(115200);

  WiFiManager wifiManager;
  wifiManager.autoConnect("TTGO-CONFIG", "12345678");

  u8x8.begin();
  //u8x8.setContrast(10);
  u8x8.setFont(u8x8_font_chroma48medium8_r);

  u8x8.draw1x2String(3, 2, "CryptoMoney");
  u8x8.drawString(5, 4, "Ticker");
  //u8x8.drawString(5, 4, "v1.0");
  u8x8.drawString(2, 7, "git.io/fxRYm");

  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].attach(BUTTON_PINS[i] , INPUT_PULLUP);  //setup the bounce instance for the current button
    buttons[i].interval(25);  // interval in ms
  }


}

void loop() {

  buttonCheck();

  //unsigned long currentMillis = millis();

  if (btnState2 == HIGH || btnState3 == HIGH || (millis() - previousMillis) > interval) {
    previousMillis = millis();
    interval = 60000;

    if(btnState2 == LOW && btnState3 == LOW && btnState1 == LOW){
     coin++;
    }else{
     antiFlickering = HIGH;
    }

    if(coin > 4) {
     coin = 0;
    }else if(coin < 0){
      coin = 4;
    }

    //Serial.print(">>> Connecting to ");
    //Serial.println(host);

    WiFiClientSecure client;

    if (!client.connect(host, httpsPort)) {
    u8x8.clear();
    u8x8.drawString(0, 1, "Connection failed");
    return;
    }

    //Serial.print("Requesting URL: ");
    //Serial.println("Connected to server!");
    client.println("GET /v2/ticker/" + crypto[coin] + "/ HTTP/1.1");
    client.println("Host: api.coinmarketcap.com");
    client.println("Connection: close");
    client.println();

   unsigned long timeout = millis();
   while (client.available() == 0) {
     if (millis() - timeout > 5000) {
       u8x8.clear();
       u8x8.drawString(0, 1, "Client Timeout!");
       client.stop();
       return;
     }
    }

    String data;
    while(client.available()) {
     data = client.readStringUntil('\r');
     Serial.println(data);
    }

    data.replace('[', ' ');
    data.replace(']', ' ');

    char buffer[data.length() + 1];
    data.toCharArray(buffer, sizeof(buffer));
    buffer[data.length() + 1] = '\0';

    const size_t bufferSize = JSON_OBJECT_SIZE(21) + 400;
    DynamicJsonBuffer jsonBuffer(bufferSize);

    JsonObject& root = jsonBuffer.parseObject(buffer);

    if (!root.success()) {
     u8x8.clear();
     u8x8.drawString(0, 1, "parseObject() failed");
     return;
    }

    JsonObject& data0 = root["data"];
    String name = data0["name"];     // "Bitcoin"
    String symbol = data0["symbol"]; // "BTC"

    JsonObject& data1 = data0["quotes"]["USD"];
    String price = data1["price"];                          // "573.137"
    String percent_change_1h = data1["percent_change_1h"];  // "0.04"
    String last_updated = data0["last_updated"];            // "1472762067" <-- Unix Time Stamp
    String error = root["error"];   // id not found 

    client.stop();

    printTransition();
    printName(symbol);
    printPrice(price);
    printPagination();
    //printError(error);
    oldPrice[coin] = price;

    btnState2 = LOW;
    btnState3 = LOW;

  }
  printPagination();
}


void printName(String symbol) {
  int xSymbol;
  if(symbol.length() == 2){
      xSymbol = 6;
  }else if(symbol.length() == 3){
      xSymbol = 5;
  }else if(symbol.length() == 4){
      xSymbol = 4;
  }else if(symbol.length() == 5){
      xSymbol = 3;
  }
  u8x8.draw2x2String(xSymbol, 1, symbol.c_str());
}

void printPrice(String price) {
 
  int reduceDecimalPrice = price.toInt();
  String printPriceConvert;
  int xPrice;

  if(reduceDecimalPrice >= 1000 && reduceDecimalPrice < 100000){
   printPriceConvert = price.substring(0, 7);
   xPrice = 5;
  }else if(reduceDecimalPrice >= 100){
   printPriceConvert = price.substring(0, 6);
   xPrice = 5;
  }else if(reduceDecimalPrice >= 10){
   printPriceConvert = price.substring(0, 5);
   xPrice = 5;
  }else if(reduceDecimalPrice >= 1){
   printPriceConvert = price.substring(0, 4);
   xPrice = 7;
  }else{
   printPriceConvert = price.substring(0, 8);
   xPrice = 4;
  }
  u8x8.draw1x2String(xPrice - 1, 4, "$");
  u8x8.draw1x2String(xPrice, 4, printPriceConvert.c_str());

  int xBitmapPrice = printPriceConvert.length();
  
  if(price != oldPrice[coin]){
    if(price > oldPrice[coin]){
      u8x8.drawTile(xPrice + xBitmapPrice + 1, 4, 1, flecheHaut);
      u8x8.drawTile(xPrice + xBitmapPrice + 1, 5, 1, flecheHaut1);
    }else{
      u8x8.drawTile(xPrice + xBitmapPrice + 1, 4, 1, flecheBas1);
      u8x8.drawTile(xPrice + xBitmapPrice + 1, 5, 1, flecheBas);
    }
  }
}

void printPagination() { 
   if(coin == 0){
    u8x8.drawTile(6, 7, 4, p1);
   }else if(coin == 1){
    u8x8.drawTile(6, 7, 4, p2);
   }else if(coin == 2){
    u8x8.drawTile(6, 7, 4, p3);
   }else if(coin == 3){
    u8x8.drawTile(6, 7, 4, p4);
   }else if(coin == 4){
    u8x8.drawTile(6, 7, 4, p5);
   }
}

/*
void printError(String error) {

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(65, 22);
  tft.println(error);
}
*/

void printTransition(){
    delay(100);
    u8x8.clear();
}

void buttonCheck() {

  for (int i = 0; i < NUM_BUTTONS; i++)  {
    // Update the Bounce instance :
    buttons[i].update();
    if ( buttons[i].fell() && i == 0) {
      btnState1 = !btnState1;
      //antiFlickering = HIGH;
      //tft.fillRect(95, 297, 47, 7, CUSTOM_DARK);
      //tft.fillRect(111, 287, 15, 20, CUSTOM_DARK);
      Serial.print("Bouton 1 est préssé "); Serial.println(btnState1);
    }else if(buttons[i].fell() && i == 1) {
      btnState2 = HIGH;
      Serial.print("Bouton 2 est préssé "); Serial.println(btnState2);
       if(btnState2 == HIGH){
        coin--;  
      }
    }else if(buttons[i].fell() && i == 2) {
      btnState3 = HIGH;
      Serial.print("Button 3 est préssé "); Serial.println(btnState3);
       if(btnState3 == HIGH){
        coin++;
      }
    }
  }
}
