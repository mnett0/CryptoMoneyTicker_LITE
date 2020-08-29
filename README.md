# CryptoMoneyTicker_LITE v1.0

![GIF_demo](https://github.com/mnett0/CryptoMoneyTicker_LITE/blob/master/media/demo2.gif)

## Description 

CryptoMoneyTicker Lite is as its name suggests a lite version of the [CryptoMoneyTicker](https://github.com/mnett0/CryptoMoneyTicker) project.

It allows you to follow, on a small screen, 5 of your favorite cryptomoney that scroll every minute, by showing the "symbol" of the cryptomoney, its current exchange rate in dollars.

Data are collected via the API of the site [coinpaprika](https://coinpaprika.com/).

The arrow to the right indicates whether the previous price of the cryptomoney is falling or rising. 

[Experimental] Left button allows to switch to the previous cryptomoney, the one in the middle to block it, the one on the right to switch to the next.

### Materials

 * ESP8266
 * OLED SSH1106 1.3"
 * 3 Push buttons (optional) 

![schema](https://github.com/mnett0/CryptoMoneyTicker_LITE/blob/master/media/diagram.jpg)

### Library required

* [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
* [ESP8266WiFi](https://github.com/esp8266/Arduino/tree/master/libraries)
* [AutoConnect](https://github.com/Hieromon/AutoConnect)
* [WiFiClientSecure](https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFiClientSecure)
* [U8g2](https://github.com/olikraus/u8g2)
* [Bounce2](https://github.com/thomasfredericks/Bounce2)

### To begin

* To connect the device to the wifi watch [HERE](https://hieromon.github.io/AutoConnect/gettingstarted/index.html#run-at-first)

* Enter the 5 crypto-currency of your choice:

Go to [listings](https://api.coinpaprika.com/v1/coins) then CTRL+F to launch the search tool, then type the name of the desired currency, take its {id}.
And finally follow the instructions from line 67

![listings_picture](https://github.com/mnett0/CryptoMoneyTicker_LITE/blob/master/media/listings2.png)

* Choose the screen change interval *(do not exceed 30 changes/minute)*

      114  interval = yourInterval; // in milliseconds

### License

[MIT](https://github.com/mnett0/CryptoMoneyTicker_LITE/blob/master/LICENSE.md)
