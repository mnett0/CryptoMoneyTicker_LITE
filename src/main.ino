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
#include <WiFiClientSecureBearSSL.h>
#include <ESP8266HTTPClient.h>

#include <AutoConnect.h>

#include <Wire.h>
#include <U8x8lib.h>

#include <Bounce2.h>

#include "myBitmaps.h"

U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE, 4, 5);

#define BUTTON_A 14
#define BUTTON_B 12
#define BUTTON_C 13

// const char host[] = "api.coinpaprika.com";
// const int httpPort = 80;
// Expiration le jeudi 23 septembre 2021 à 12:54:21 heure d’été d’Europe centrale
const uint8_t fingerprint[20] = {0x76, 0xc0, 0x35, 0xe4, 0xe7, 0x7e, 0x0e, 0xab, 0x61, 0xcf, 0xe9, 0xcd, 0x5a, 0x3e, 0x3f, 0x68, 0x9b, 0xc8, 0x9f, 0xcb};

unsigned long previousMillis = 0;
long interval = 0;

#define NUM_BUTTONS 3
const uint8_t BUTTON_PINS[NUM_BUTTONS] = {BUTTON_A, BUTTON_B, BUTTON_C};
Bounce *buttons = new Bounce[NUM_BUTTONS];

bool btnState1 = LOW;
bool btnState2 = LOW;
bool btnState3 = LOW;
bool antiFlickering = LOW;

// For choose ur cryptocurrency -> https://api.coinpaprika.com/v1/coins <-
#define ETH "eth-ethereum"
#define VET "vet-vechain"
#define OCEAN "ocean-ocean-protocol"
#define EWT "ewt-energy-web-token"
#define ENJ "enj-enjin-coin"
// #define LINK "link-chainlink"

// and change again the name here
String crypto[] = {ETH, VET, OCEAN, EWT, ENJ};
int coin = -1;
String oldPrice[5];

ESP8266WebServer Server;
AutoConnect Portal(Server);

void setup()
{

  Serial.begin(115200);

  u8x8.begin();
  u8x8.setContrast(10);
  u8x8.setFont(u8x8_font_chroma48medium8_r);

  u8x8.draw1x2String(3, 2, "CryptoMoney");
  u8x8.drawString(5, 4, "Ticker");
  //u8x8.drawString(5, 4, "v1.0");
  u8x8.drawString(2, 7, "git.io/fxRYm");

  for (int i = 0; i < NUM_BUTTONS; i++)
  {
    buttons[i].attach(BUTTON_PINS[i], INPUT_PULLUP); //setup the bounce instance for the current button
    buttons[i].interval(25);                         // interval in ms
  }

  Portal.begin();
}

void loop()
{

  Portal.handleClient();

  buttonCheck();

  //unsigned long currentMillis = millis();

  if (btnState2 == HIGH || btnState3 == HIGH || (millis() - previousMillis) > interval)
  {
    previousMillis = millis();
    interval = 60000;

    if (btnState2 == LOW && btnState3 == LOW && btnState1 == LOW)
    {
      coin++;
    }
    else
    {
      antiFlickering = HIGH;
    }

    if (coin > 4)
    {
      coin = 0;
    }
    else if (coin < 0)
    {
      coin = 4;
    }

    String payload;

    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);

    client->setFingerprint(fingerprint);

    HTTPClient https;

    if (https.begin(*client, "https://api.coinpaprika.com/v1/tickers/" + crypto[coin]))
    { // HTTPS

      // start connection and send HTTP header
      int httpCode = https.GET();

      // httpCode will be negative on error
      if (httpCode > 0)
      {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
        {
          payload = https.getString();
        }
      }
      else
      {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    }
    else
    {
      Serial.printf("[HTTPS] Unable to connect\n");
    }

    // String jsonData;
    Serial.println(payload);
    const size_t capacity = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(10) + JSON_OBJECT_SIZE(17) + 900;
    DynamicJsonDocument doc(capacity);
    DeserializationError error = deserializeJson(doc, payload);

    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }

    // String name = doc["name"];     // "Dai"
    String symbol = doc["symbol"]; // "BTC"
    // String error = doc["error"];   // id not found

    JsonObject quotes_USD = doc["quotes"]["USD"];
    String price = quotes_USD["price"];                         // "573.137"
    String percent_change_1h = quotes_USD["percent_change_1h"]; // "0.04"
    // String last_updated = quotes_USD["last_updated"];           // "1472762067" <-- Unix Time Stamp

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

void printName(String symbol)
{
  int xSymbol;
  if (symbol.length() == 2)
  {
    xSymbol = 6;
  }
  else if (symbol.length() == 3)
  {
    xSymbol = 5;
  }
  else if (symbol.length() == 4)
  {
    xSymbol = 4;
  }
  else if (symbol.length() == 5)
  {
    xSymbol = 3;
  }
  u8x8.draw2x2String(xSymbol, 1, symbol.c_str());
}

void printPrice(String price)
{

  int reduceDecimalPrice = price.toInt();
  String printPriceConvert;
  int xPrice;

  if (reduceDecimalPrice >= 1000 && reduceDecimalPrice < 100000)
  {
    printPriceConvert = price.substring(0, 7);
    xPrice = 5;
  }
  else if (reduceDecimalPrice >= 100)
  {
    printPriceConvert = price.substring(0, 6);
    xPrice = 5;
  }
  else if (reduceDecimalPrice >= 10)
  {
    printPriceConvert = price.substring(0, 5);
    xPrice = 5;
  }
  else if (reduceDecimalPrice >= 1)
  {
    printPriceConvert = price.substring(0, 4);
    xPrice = 7;
  }
  else
  {
    printPriceConvert = price.substring(0, 8);
    xPrice = 4;
  }
  u8x8.draw1x2String(xPrice - 1, 4, "$");
  u8x8.draw1x2String(xPrice, 4, printPriceConvert.c_str());

  int xBitmapPrice = printPriceConvert.length();

  if (price != oldPrice[coin])
  {
    if (price > oldPrice[coin])
    {
      u8x8.drawTile(xPrice + xBitmapPrice + 1, 4, 1, flecheHaut);
      u8x8.drawTile(xPrice + xBitmapPrice + 1, 5, 1, flecheHaut1);
    }
    else
    {
      u8x8.drawTile(xPrice + xBitmapPrice + 1, 4, 1, flecheBas1);
      u8x8.drawTile(xPrice + xBitmapPrice + 1, 5, 1, flecheBas);
    }
  }
}

void printPagination()
{
  if (coin == 0)
  {
    u8x8.drawTile(6, 7, 4, p1);
  }
  else if (coin == 1)
  {
    u8x8.drawTile(6, 7, 4, p2);
  }
  else if (coin == 2)
  {
    u8x8.drawTile(6, 7, 4, p3);
  }
  else if (coin == 3)
  {
    u8x8.drawTile(6, 7, 4, p4);
  }
  else if (coin == 4)
  {
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

void printTransition()
{
  delay(100);
  u8x8.clear();
}

void buttonCheck()
{

  for (int i = 0; i < NUM_BUTTONS; i++)
  {
    // Update the Bounce instance :
    buttons[i].update();
    if (buttons[i].fell() && i == 0)
    {
      btnState1 = !btnState1;
      //antiFlickering = HIGH;
      //tft.fillRect(95, 297, 47, 7, CUSTOM_DARK);
      //tft.fillRect(111, 287, 15, 20, CUSTOM_DARK);
      Serial.print("Bouton 1 est préssé ");
      Serial.println(btnState1);
    }
    else if (buttons[i].fell() && i == 1)
    {
      btnState2 = HIGH;
      Serial.print("Bouton 2 est préssé ");
      Serial.println(btnState2);
      if (btnState2 == HIGH)
      {
        coin--;
      }
    }
    else if (buttons[i].fell() && i == 2)
    {
      btnState3 = HIGH;
      Serial.print("Button 3 est préssé ");
      Serial.println(btnState3);
      if (btnState3 == HIGH)
      {
        coin++;
      }
    }
  }
}
