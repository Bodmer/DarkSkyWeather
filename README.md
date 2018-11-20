# ESP8266 and ESP32 Dark Sky weather client

Arduino client library for https://darksky.net/dev

Collects current weather plus daily forecasts.

Requires the JSON parse library here:
https://github.com/Bodmer/JSON_Decoder

The DarkSkyWeather_Test example sketch sends collected data to the Serial port for API test. It does not not require a TFT screen.

The TFT_eSPI_Weather example works with the ESP8266 and ESP32, it displays the weather data on a TFT screen.  These examples use anti-aliased fonts and newly created icons:

![Weather isons](https://i.imgur.com/luK7Vcj.jpg)

Latest screen grabs:

![TFT splash screen](https://i.imgur.com/gh75gd6.png)

![TFT screenshot 1](https://i.imgur.com/ORovwNY.png)

