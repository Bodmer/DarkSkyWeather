//  Original by Daniel Eichhorn, see license at end of file

//  Adapted to use the DarkSkyWeather library: https://github.com/Bodmer/DarkSkyWeather

//  The weather icons and fonts, these are in the sketch data folder, press Ctrl+K
//  to view.

//  Upload the fonts and icons to SPIFFS (must set at least 2M for SPIFFS) using the
//  "Tools" "ESP8266 Sketch Data Upload" menu option in the IDE.  To add this option
//  follow instructions here: https://github.com/esp8266/arduino-esp8266fs-plugin
//  Close the IDE and open again to see the new menu option.

//  Adapted by Bodmer to:
//  Use the TFT_eSPI library: https://github.com/Bodmer/TFT_eSPI
//  Use Dark Sky API

// ***************************************************************************************
//                                WARNING - READ THIS
//
// 2M Flash Size MUST be allocated to SPIFFS using the IDE Tools menu option or else the
// ESP8266 may crash or do strange things (due to lack of error checks in SPIFFS library?)
// ***************************************************************************************


// Settting defined here

#define TIMEZONE UK // See NTP_Time.h tab for other "Zone references", UK, usMT etc

// Update every 15 minutes, up to 1000 request per day are free (viz average of ~40 per hour)
const int UPDATE_INTERVAL_SECS = 15 * 60UL; //  15 minutes

// Pins for the TFT interface are defined in the User_Config.h file inside the TFT_eSPI library

// For units codes see https://darksky.net/dev/docs
const String units = "si";

// Sign up for an account at Dark Sky, changes x's to your API key
const String api_key = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

// For language codes see https://darksky.net/dev/docs
const String language = "en"; // Default language = English

// Set the geolocation for the forecast
String latitude =  "27.9881"; // 90.0000 to -90.0000 negative for Southern hemisphere
String longitude = "86.9250"; // 180.000 to -180.000 negative for West

// Set the small and large anti-aliased fonts used for the display
// A processing sketch to create new fonts can be found in the Tools folder of TFT_eSPI
// https://github.com/Bodmer/TFT_eSPI/tree/master/Tools/Create_Smooth_Font/Create_font
#define AA_FONT_SMALL "fonts/NotoSansBold15"
#define AA_FONT_LARGE "fonts/NotoSansBold36"


//Original license:

/**The MIT License (MIT)
Copyright (c) 2015 by Daniel Eichhorn
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
See more at http://blog.squix.ch
*/
