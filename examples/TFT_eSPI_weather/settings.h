
//  Adapted to use the DarkSkyWeather library: https://github.com/Bodmer/DarkSkyWeather

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

//Adapted by Bodmer to:
//  Use the TFT_eSPI library: https://github.com/Bodmer/TFT_eSPI
//  Use Dark Sky API

// ***************************************************************************************
//                                WARNING - READ THIS
//
// 3M Flash Size MUST be allocated to SPIFFS using the IDE Tools menu option or else the
// ESP8266 may crash or do strange things (due to lack of error checks in SPIFFS library?)
// ***************************************************************************************


// Setup

const int UPDATE_INTERVAL_SECS = 15 * 60; // Update every 15 minutes

// Pins for the TFT interface are defined in the User_Config.h file inside the TFT_eSPI library


// timeZone not used ####
const int timeZone = 1;     // Central European Time
//const int timeZone = -5;  // Eastern Standard Time (USA)
//const int timeZone = -4;  // Eastern Daylight Time (USA)
//const int timeZone = -8;  // Pacific Standard Time (USA)
//const int timeZone = -7;  // Pacific Daylight Time (USA)

// Weather API settings
const String units = "si";

String api_key = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

// For language codes see #############
const String language = ""; // Default language = English

// Set both longitude and latitude to at least 4 decimal places
String latitude =  "27.9881"; // 90.0000 to -90.0000 negative for Southern hemisphere
String longitude = "86.9250"; // 180.000 to -180.000 negative for West


// List icons, so that the downloader knows what to fetch
String weatherIcons [] = {"chanceflurries","chancerain","chancesleet","chancesnow","clear","cloudy","flurries","fog","hazy","mostlycloudy","mostlysunny","partlycloudy","partlysunny","rain","sleet","snow","sunny","tstorms","unknown"};

String moonPhase [8] = {"New Moon","Waxing crescent","First quarter","Waxing gibbous","Full Moon","Waning gibbous","Last quarter","Waning crescent"};

