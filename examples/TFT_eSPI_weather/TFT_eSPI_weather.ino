
//  Adapted to use the DarkSkyWeather library: https://github.com/Bodmer/DarkSkyWeather

//  This is a Beta copy, it works but needs to be tidied up and final features added

//  Time zone set at line 349

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
  AUTHORS OR COPYBR_DATUM HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
  See more at http://blog.squix.ch
*/

//  Adapted by Bodmer to use the TFT_eSPI library:  https://github.com/Bodmer/TFT_eSPI

//  Plus:
//  Minor changes to text placement and auto-blanking out old text with background colour padding
//  Moon phase text added
//  Forecast text lines are automatically split onto two lines at a central space (some are long!)
//  Time is printed with colons aligned to tidy display
//  Min and max forecast temperatures spaced out
//  The ` character has been changed to a degree symbol in the 36 point font
//  New smart WU splash startup screen and updated progress messages
//  Display does not need to be blanked between updates
//  Icons nudged about slightly to add wind direction + speed
//  Barometric pressure added

//  Adapted to use the DarkSkyWeather library: https://github.com/Bodmer/DarkSkyWeather


//  IMPORTANT: Modify setup in settings.h tab to configure your location etc
//  ESP8266 pin connections to the TFT are defined in the TFT_eSPI library

#define SERIAL_MESSAGES
//#define RANDOM_LOCATION   // Test only
  
#include <Arduino.h>

#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library

// Additional UI functions
#include "GfxUi.h"

// Fonts created by http://oleddisplay.squix.ch/
#include "ArialRoundedMTBold_14.h"
#include "ArialRoundedMTBold_36.h"

// Download helper
#include "WebResource.h"

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

// Helps with connecting to internet
#include <WiFiManager.h>

// check settings.h for adapting to your needs
#include "settings.h"

#include <JsonStreamingParser.h>

#include <DarkSkyWeather.h>

#include "NTP_Time.h"

// HOSTNAME for OTA update
#define HOSTNAME "ESP8266-OTA-"

TFT_eSPI tft = TFT_eSPI();  // Invoke custom library

DSWforecast dsw;            // Weather forcast library instance

DSW_current *current;
DSW_hourly  *hourly;
DSW_daily   *daily;

boolean booted = true;

GfxUi ui = GfxUi(&tft);

WebResource webResource;

//declaring prototypes
void configModeCallback (WiFiManager *myWiFiManager);
void downloadCallback(String filename, int16_t bytesDownloaded, int16_t bytesTotal);
ProgressCallback _downloadCallback = downloadCallback;
void downloadResources();
void updateData();
void drawProgress(uint8_t percentage, String text);
void drawTime();
void drawCurrentWeather();
void drawForecast();
void drawForecastDetail(uint16_t x, uint16_t y, uint8_t dayIndex);
String getMeteoconIcon(String iconText);
void drawAstronomy();
void drawSeparator(uint16_t y);

long lastDownloadUpdate = millis();

void setup() {
#ifdef SERIAL_MESSAGES
  Serial.begin(250000);
#endif
  tft.begin();
  tft.fillScreen(TFT_BLACK);

  tft.setFreeFont(&ArialRoundedMTBold_14);
  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString("Original by: blog.squix.org", 120, 240);
  tft.drawString("Adapted by: Bodmer", 120, 260);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  
  SPIFFS.begin();
  //listFiles();
  //Uncomment if you want to erase SPIFFS and update all internet resources, this takes some time!
  //tft.drawString("Formatting SPIFFS, so wait!", 120, 200); SPIFFS.format();

  if (SPIFFS.exists("/DSW.jpg")   == true) ui.drawJpeg("/DSW.jpg",   0, 10);
  if (SPIFFS.exists("/Earth.jpg") == true) ui.drawJpeg("/Earth.jpg", 0, 320-56); // Image is 56 pixels high
  delay(1000);
  tft.drawString("Connecting to WiFi", 120, 200);
  tft.setTextPadding(240); // Pad next drawString() text to full width to over-write old text

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  // Uncomment for testing wifi manager
  //wifiManager.resetSettings();
  wifiManager.setAPCallback(configModeCallback);

  //or use this for auto generated name ESP + ChipID
  wifiManager.autoConnect();

  //Manual Wifi
  //WiFi.begin(WIFI_SSID, WIFI_PWD);

  // OTA Setup
  String hostname(HOSTNAME);
  hostname += String(ESP.getChipId(), HEX);
  WiFi.hostname(hostname);
  ArduinoOTA.setHostname((const char *)hostname.c_str());
  ArduinoOTA.begin();

  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());

  // download images from the net. If images already exist don't download
  tft.drawString("Downloading to SPIFFS...", 120, 200);
  tft.drawString(" ", 120, 240);  // Clear line
  tft.drawString(" ", 120, 260);  // Clear line
  downloadResources();
  listFiles();
  tft.setTextDatum(BC_DATUM);
  tft.setTextPadding(240); // Pad next drawString() text to full width to over-write old text
  tft.drawString(" ", 120, 200);  // Clear line above using set padding width
  tft.drawString("Fetching weather data...", 120, 200);
  //delay(500);

  // Fetch the time
  requestTime();

  // load the weather information
  updateData();
}

void loop() {
  // Handle OTA update requests
  ArduinoOTA.handle();

  // If minute has changed then request new time from NTP server
  if (minute() != lastMinute) {
    drawTime();
    lastMinute = minute();
    requestTime();
  }

  // Check if we should update weather information
  if (millis() - lastDownloadUpdate > 1000 * UPDATE_INTERVAL_SECS) {
    updateData();
    lastDownloadUpdate = millis();
  }
}

// Called if WiFi has not been configured yet
void configModeCallback (WiFiManager *myWiFiManager) {
  tft.setTextDatum(BC_DATUM);
  tft.setFreeFont(&ArialRoundedMTBold_14);
  tft.setTextColor(TFT_ORANGE);
  tft.drawString("Wifi Manager", 120, 28);
  tft.drawString("Please connect to AP", 120, 42);
  tft.setTextColor(TFT_WHITE);
  tft.drawString(myWiFiManager->getConfigPortalSSID(), 120, 56);
  tft.setTextColor(TFT_ORANGE);
  tft.drawString("To setup Wifi Configuration", 120, 70);
}

// callback called during download of files. Updates progress bar
void downloadCallback(String filename, int16_t bytesDownloaded, int16_t bytesTotal) {
  Serial.println(String(bytesDownloaded) + " / " + String(bytesTotal));

  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.setTextPadding(240);

  int percentage = 100 * bytesDownloaded / bytesTotal;
  if (percentage == 0) {
    tft.drawString(filename, 120, 220);
  }
  if (percentage % 5 == 0) {
    tft.setTextDatum(TC_DATUM);
    tft.setTextPadding(tft.textWidth(" 888% "));
    tft.drawString(String(percentage) + "%", 120, 245);
    ui.drawProgressBar(10, 225, 240 - 20, 15, percentage, TFT_WHITE, TFT_BLUE);
  }
}

// Download the bitmaps
void downloadResources() {
  // tft.fillScreen(TFT_BLACK);
  tft.setFreeFont(&ArialRoundedMTBold_14);
  char id[5];

  // Download DSW graphic jpeg first and display it, then the Earth view
  webResource.downloadFile((String)"http://i.imgur.com/ZyiTWuL.jpg", (String)"/DSW.jpg", _downloadCallback);
  if (SPIFFS.exists("/DSW.jpg") == true) ui.drawJpeg("/DSW.jpg", 0, 10);

  webResource.downloadFile((String)"http://i.imgur.com/v4eTLCC.jpg", (String)"/Earth.jpg", _downloadCallback);
  if (SPIFFS.exists("/Earth.jpg") == true) ui.drawJpeg("/Earth.jpg", 0, 320-56);
  
  //webResource.downloadFile((String)"http://i.imgur.com/IY57GSv.jpg", (String)"/Horizon.jpg", _downloadCallback);
  //if (SPIFFS.exists("/Horizon.jpg") == true) ui.drawJpeg("/Horizon.jpg", 0, 320-160);

  //webResource.downloadFile((String)"http://i.imgur.com/jZptbtY.jpg", (String)"/Rainbow.jpg", _downloadCallback);
  //if (SPIFFS.exists("/Rainbow.jpg") == true) ui.drawJpeg("/Rainbow.jpg", 0, 0);

  for (int i = 0; i < 19; i++) {
    sprintf(id, "%02d", i);
    webResource.downloadFile("http://www.squix.org/blog/wunderground/" + weatherIcons[i] + ".bmp", weatherIcons[i] + ".bmp", _downloadCallback);
  }
  for (int i = 0; i < 19; i++) {
    sprintf(id, "%02d", i);
    webResource.downloadFile("http://www.squix.org/blog/wunderground/mini/" + weatherIcons[i] + ".bmp", "/mini/" + weatherIcons[i] + ".bmp", _downloadCallback);
  }
  for (int i = 0; i < 24; i++) {
    webResource.downloadFile("http://www.squix.org/blog/moonphase_L" + String(i) + ".bmp", "/moon" + String(i) + ".bmp", _downloadCallback);
  }
}

// Update the internet based information and update screen
void updateData() {
  // booted = true;  // Test only
  // booted = false; // Test only

  if (booted) ui.drawJpeg("/DSW.jpg", 0, 10); // May have already drawn this but it does not take long
  else tft.drawCircle(22, 22, 18, TFT_DARKGREY); // Outer ring - optional

  if (booted) drawProgress(20, "Updating time...");
  else fillSegment(22, 22, 0, (int) (20 * 3.6), 16, TFT_NAVY);

//####  timeClient.updateTime();
  if (booted) drawProgress(50, "Updating conditions...");
  else fillSegment(22, 22, 0, (int) (50 * 3.6), 16, TFT_NAVY);

  // Create the structures that hold the retrieved weather
  current = new DSW_current;
  hourly = new DSW_hourly;
  daily = new DSW_daily;

  #ifdef RANDOM_LOCATION
    String latitude = "";
    latitude = (random(180) - 90);
    String longitude = "";
    longitude = (random(360) - 180);
  #endif
  
  bool parsed = dsw.getForecast(current, hourly, daily, api_key, latitude, longitude, units, language);
  printWeather(); // For debug

  if (booted) drawProgress(100, "Done...");
  else fillSegment(22, 22, 0, 360, 16, TFT_NAVY);

  if (booted) delay(2000);

  if (booted) tft.fillScreen(TFT_BLACK);
  else   fillSegment(22, 22, 0, 360, 22, TFT_BLACK);

  //tft.fillScreen(TFT_CYAN); // For text padding and update graphics over-write checking only

  if (parsed)
  {
    //drawTime();
    drawCurrentWeather();
    drawForecast();
    drawAstronomy();
  }
  else
  {
    Serial.println("Failed to get weather");
  }

  booted = false;

  // Delete to free up space
  delete current;
  delete hourly;
  delete daily;
}

// Progress bar helper
void drawProgress(uint8_t percentage, String text) {
  tft.setFreeFont(&ArialRoundedMTBold_14);

  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.setTextPadding(240);
  tft.drawString(text, 120, 220);

  ui.drawProgressBar(10, 225, 240 - 20, 15, percentage, TFT_WHITE, TFT_BLUE);

  tft.setTextPadding(0);
}

// draws the clock
void drawTime() {

  tft.setFreeFont(&ArialRoundedMTBold_36);

  //time_t local_time = UK.toLocal(utc, &tz1_Code);
  time_t local_time = euCET.toLocal(now(), &tz1_Code);  // Convert UTC to local time, get zone code e.g "GMT"
  
  String timeNow = "";
  if(hour(local_time) < 10) timeNow += "0";
  timeNow += hour(local_time);
  timeNow += ":";
  if(minute(local_time) < 10) timeNow += "0";
  timeNow += minute(local_time);

  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextPadding(tft.textWidth(" 44:44 "));  // String width + margin
  tft.drawString(timeNow, 120, 53);

  drawSeparator(54);

  tft.setTextPadding(0);
}

// draws current weather information
void drawCurrentWeather() {

  tft.setFreeFont(&ArialRoundedMTBold_14);

  String date = strTime(current->time);
  date = "Updated: " + date.substring(0, 16);

  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextPadding(tft.textWidth(" Updated: Ddd Mmm 44 44:44 "));  // String width + margin
  tft.drawString(date, 120, 16);

  // Weather Icon patch for "wind"
  String weatherIcon = "";
  if ( current->icon == "wind" && current->summary.indexOf("Cloudy") >= 0)
  {
    weatherIcon = getMeteoconIcon("cloudy");
  }
  else weatherIcon = getMeteoconIcon(current->icon);

  //uint32_t dt = millis();
  ui.drawBmp(weatherIcon + ".bmp", 0, 59);
  //Serial.print("Icon draw time = "); Serial.println(millis()-dt);

  // Weather Text
  String weatherText = current->summary;
  //weatherText = "Heavy Thunderstorms with Small Hail"; // Test line splitting with longest(?) string

  tft.setTextDatum(BR_DATUM);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);

  int splitPoint = 0;
  int xpos = 230;
  splitPoint =  splitIndex(weatherText);
  if (splitPoint > 16) xpos = 235;

  tft.setTextPadding(tft.textWidth("Heavy Thunderstorms"));  // Max anticipated string width
  if (splitPoint) tft.drawString(weatherText.substring(0, splitPoint), xpos, 72);
  else tft.drawString(" ", xpos, 72);
  tft.setTextPadding(tft.textWidth(" with Small Hail"));  // Max anticipated string width + margin
  tft.drawString(weatherText.substring(splitPoint), xpos, 87);

  tft.setFreeFont(&ArialRoundedMTBold_36);

  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);

  // Font ASCII code 96 (0x60) modified to make "`" a degree symbol
  tft.setTextPadding(tft.textWidth("-88`")); // Max width of vales

  weatherText = current->temperature;
  if (weatherText.indexOf(".")) weatherText = weatherText.substring(0, weatherText.indexOf(".")); // Make it integer temperature
  if (weatherText == "") weatherText = "?";  // Handle null return
  tft.drawString(weatherText + "`", 221, 100);

  tft.setFreeFont(&ArialRoundedMTBold_14);

  tft.setTextDatum(TL_DATUM);
  tft.setTextPadding(0);
  if (units == "si") tft.drawString("C ", 221, 100);
  else  tft.drawString("F ", 221, 100);

  weatherText = (uint16_t)current->windSpeed;
  if (units == "si") weatherText += " m/s";
  else if (units == "us" || units == "uk2") weatherText += " mph";
  else if (units == "ca") weatherText += " kph";

  tft.setTextDatum(TC_DATUM);
  tft.setTextPadding(tft.textWidth(" 888 m/s")); // Max string length?
  tft.drawString(weatherText, 128, 136);

  if (units == "us")
  {
    weatherText = current->pressure;
    weatherText += " in";
  }
  else
  {
    weatherText = (uint16_t)current->pressure;
    weatherText += " hPa";
  }

  tft.setTextDatum(TR_DATUM);
  tft.setTextPadding(tft.textWidth(" 8888hPa")); // Max string length?
  tft.drawString(weatherText, 230, 136);
  
  int windAngle = current->windBearing;

  tft.fillCircle(128, 110, 23, TFT_BLACK); // Erase old plot, radius + 1 to delete stray pixels
  tft.drawCircle(128, 110, 22, TFT_DARKGREY);    // Outer ring - optional
  if ( windAngle >= 0 ) fillSegment(128, 110, windAngle - 15, 30, 22, TFT_GREEN);
  tft.drawCircle(128, 110, 6, TFT_RED);

  drawSeparator(153);

  tft.setTextDatum(TL_DATUM); // Reset datum to normal
  tft.setTextPadding(0); // Reset padding width to none
}

// draws the three forecast columns
void drawForecast() {
  int8_t dayIndex = 0;
  while ((daily->time[dayIndex] < current->time) && (dayIndex < (MAX_DAYS - 3))) dayIndex++;
  drawForecastDetail(10, 171, dayIndex);
  drawForecastDetail(95, 171, dayIndex + 1);
  drawForecastDetail(180, 171, dayIndex + 2);
  drawSeparator(171 + 69);
}

// helper for the forecast columns
void drawForecastDetail(uint16_t x, uint16_t y, uint8_t dayIndex) {
  tft.setFreeFont(&ArialRoundedMTBold_14);
  String date = strTime(daily->time[dayIndex]);
  String day  = date.substring(0, 3);
  day.toUpperCase();

  tft.setTextDatum(BC_DATUM);

  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.setTextPadding(tft.textWidth("WWW"));
  tft.drawString(day, x + 25, y);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextPadding(tft.textWidth("-88   -88"));
  String highTemp = String(daily->temperatureHigh[dayIndex], 0);
  String lowTemp  = String(daily->temperatureLow[dayIndex], 0);
  tft.drawString(highTemp + "   " + lowTemp, x + 25, y + 14);

  // Weather Icon patch for "wind"
  String weatherIcon = "";
  if ( daily->icon[dayIndex] == "wind" && daily->summary[dayIndex].indexOf("loudy") >= 0)
  {
    weatherIcon = getMeteoconIcon("cloudy");
  }
  else weatherIcon = getMeteoconIcon(daily->icon[dayIndex]);

  ui.drawBmp("/mini/" + weatherIcon + ".bmp", x, y + 15);

  tft.setTextPadding(0); // Reset padding width to none
}

// draw moonphase and sunrise/set and moonrise/set
void drawAstronomy() {
  tft.setFreeFont(&ArialRoundedMTBold_14);

  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.setTextPadding(tft.textWidth(" Waxing Crescent "));

  int8_t dayIndex = 0;

  //The first daly summary may be yesterday
  //while ((daily->time[dayIndex] < current->time) && (dayIndex < MAX_DAYS)) dayIndex++;

  uint8_t moonIndex = (uint8_t)(((float)daily->moonPhase[dayIndex] + 6.25) / 12.5);
  if (moonIndex > 7) moonIndex = 0;
  tft.drawString(moonPhase[moonIndex], 120, 260 - 2);

  uint8_t moonAgeImage = (24.0 * daily->moonPhase[dayIndex])/100;
  if (moonAgeImage > 23) moonAgeImage = 0;
  ui.drawBmp("/moon" + String(moonAgeImage) + ".bmp", 120 - 30, 260);

  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.setTextPadding(0); // Reset padding width to none
  tft.drawString("Sun", 40, 280);

  tft.setTextDatum(BR_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextPadding(tft.textWidth(" 88:88 "));

  String date = strTime(daily->sunriseTime[dayIndex]);
  String rising  = date.substring(11, 16);
  int dt = rightOffset(rising, ":"); // Draw relative to colon to them aligned
  tft.drawString(rising, 40 + dt, 300);

  date = strTime(daily->sunsetTime[dayIndex]);
  String setting  = date.substring(11, 16);
  dt = rightOffset(setting, ":");
  tft.drawString(setting, 40 + dt, 315);

  /* Dark Sky does not provide Moon rise and setting times
   * There are other free API providers:
   * http://api.usno.navy.mil/rstt/oneday?date=1/5/2005&loc=Los%20Angeles,%20CA

  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.setTextPadding(0); // Reset padding width to none
  tft.drawString("Moon", 200, 280);

  tft.setTextDatum(BR_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextPadding(tft.textWidth(" 88:88 "));

  date = strTime(daily->moonriseTime[dayIndex]);
  rising  = date.substring(11, 16);
  dt = rightOffset(rising, ":"); // Draw relative to colon to them aligned
  tft.drawString(rising, 200 + dt, 300);

  date = strTime(daily->moonsetTime[dayIndex]);
  setting  = date.substring(11, 16);
  dt = rightOffset(moonsetTime, ":");
  tft.drawString(moonsetTime, 200 + dt, 315);
  */

  tft.setTextPadding(0); // Reset padding width to none
}

// Helper function, should be part of the weather station library and should disappear soon
// These are from Wunderground, currently no "wind" icon for Dark Sky API
String getMeteoconIcon(String iconText) {
  //if (iconText == "F") return "chanceflurries";
  //if (iconText == "Q") return "chancerain";
  //if (iconText == "W") return "chancesleet";
  //if (iconText == "V") return "chancesnow";
  //if (iconText == "S") return "chancetstorms";
  if (iconText == "clear-day") return "clear";
  if (iconText == "clear-night") return "clear"; // Need a moon
  if (iconText == "cloudy") return "cloudy";
  //if (iconText == "F") return "flurries";
  if (iconText == "fog") return "fog";
  //if (iconText == "E") return "hazy";
  //if (iconText == "Y") return "mostlycloudy";
  //if (iconText == "H") return "mostlysunny";
  if (iconText == "partly-cloudy-day") return "partlycloudy";
  if (iconText == "partly-cloudy-night") return "partlycloudy";  // Need a moon + cloud
  //if (iconText == "J") return "partlysunny";
  if (iconText == "sleet") return "sleet";
  if (iconText == "rain") return "rain";
  if (iconText == "snow") return "snow";
  //if (iconText == "wind") return "wind"; // Need a wind icon
  //if (iconText == "B") return "sunny";
  //if (iconText == "0") return "tstorms";


  return "unknown";
}

// if you want separators, uncomment the tft-line
void drawSeparator(uint16_t y) {
  tft.drawFastHLine(10, y, 240 - 2 * 10, 0x4228);
}

// determine the "space" split point in a long string
int splitIndex(String text)
{
  int index = 0;
  while ( (text.indexOf(' ', index) >= 0) && ( index <= text.length() / 2 ) ) {
    index = text.indexOf(' ', index) + 1;
  }
  if (index) index--;
  return index;
}

// Calculate coord delta from start of text String to start of sub String contained within that text
// Can be used to vertically right align text so for example a colon ":" in the time value is always
// plotted at same point on the screen irrespective of different proportional character widths,
// could also be used to align decimal points for neat formatting
int rightOffset(String text, String sub)
{
  int index = text.indexOf(sub);
  return tft.textWidth(text.substring(index));
}

// Calculate coord delta from start of text String to start of sub String contained within that text
// Can be used to vertically left align text so for example a colon ":" in the time value is always
// plotted at same point on the screen irrespective of different proportional character widths,
// could also be used to align decimal points for neat formatting
int leftOffset(String text, String sub)
{
  int index = text.indexOf(sub);
  return tft.textWidth(text.substring(0, index));
}

// Draw a segment of a circle, centred on x,y with defined start_angle and subtended sub_angle
// Angles are defined in a clockwise direction with 0 at top
// Segment has radius r and it is plotted in defined colour
// Can be used for pie charts etc, in this sketch it is used for wind direction
#define DEG2RAD 0.0174532925 // Degrees to Radians conversion factor
#define INC 2 // Minimum segment subtended angle and plotting angle increment (in degrees)
void fillSegment(int x, int y, int start_angle, int sub_angle, int r, unsigned int colour)
{
  // Calculate first pair of coordinates for segment start
  float sx = cos((start_angle - 90) * DEG2RAD);
  float sy = sin((start_angle - 90) * DEG2RAD);
  uint16_t x1 = sx * r + x;
  uint16_t y1 = sy * r + y;

  // Draw colour blocks every INC degrees
  for (int i = start_angle; i < start_angle + sub_angle; i += INC) {

    // Calculate pair of coordinates for segment end
    int x2 = cos((i + 1 - 90) * DEG2RAD) * r + x;
    int y2 = sin((i + 1 - 90) * DEG2RAD) * r + y;

    tft.fillTriangle(x1, y1, x2, y2, x, y, colour);

    // Copy segment end to sgement start for next segment
    x1 = x2;
    y1 = y2;
  }
}

void printWeather(void)
{
  Serial.println("Weather from Dark Sky\n");

  // We can use the timezone to set the offset eventually...
  Serial.print("Timezone           : "); Serial.println(current->timezone);
  
  Serial.println("############### Current weather ###############\n");
  Serial.print("time               : "); Serial.print(strTime(current->time));
  Serial.print("summary            : "); Serial.println(current->summary);
  Serial.print("icon               : "); Serial.println(current->icon);
  Serial.print("precipInten        : "); Serial.println(current->precipIntensity);
  Serial.print("precipType         : "); Serial.println(current->precipType);
  Serial.print("precipProbability  : "); Serial.println(current->precipProbability);
  Serial.print("temperature        : "); Serial.println(current->temperature);
  Serial.print("humidity           : "); Serial.println(current->humidity);
  Serial.print("pressure           : "); Serial.println(current->pressure);
  Serial.print("wind speed         : "); Serial.println(current->windSpeed);
  Serial.print("wind gust          : "); Serial.println(current->windGust);
  Serial.print("wind dirn          : "); Serial.println(current->windBearing);
  Serial.print("cloudCover         : "); Serial.println(current->cloudCover);

  Serial.println();

/*
  Serial.println("############### Hourly weather  ###############\n");
  Serial.print("Overall hourly summary : "); Serial.println(hourly->overallSummary);
  for (int i = 0; i<MAX_HOURS; i++)
  {
    Serial.print("Hourly summary ");Serial.print(i);Serial.print("  : "); Serial.println(hourly->summary[i]);
    Serial.print("time               : "); Serial.print(strTime(hourly->time[i]));
    Serial.print("precipIntensity    : "); Serial.println(hourly->precipIntensity[i]);
    Serial.print("precipProbability  : "); Serial.println(hourly->precipProbability[i]);
    Serial.print("precipType         : "); Serial.println(hourly->precipType[i]);
    Serial.print("precipAccumulation : "); Serial.println(hourly->precipAccumulation[i]);
    Serial.print("temperature        : "); Serial.println(hourly->temperature[i]);
    Serial.print("pressure           : "); Serial.println(hourly->pressure[i]);
    Serial.print("cloudCover         : "); Serial.println(hourly->cloudCover[i]);
    Serial.println();
  }
*/
  Serial.println("###############  Daily weather  ###############\n");
  Serial.print(" Overall summary     : "); Serial.println(daily->overallSummary);
  Serial.println();

  for (int i = 0; i<4; i++)
  {
    Serial.print("Day summary   ");Serial.print(i);Serial.print("    : "); Serial.println(daily->summary[i]);
    Serial.print("time               : "); Serial.print(strTime(daily->time[i]));
    Serial.print("icon               : "); Serial.println(daily->icon[i]);
    Serial.print("sunriseTime        : "); Serial.print(strTime(daily->sunriseTime[i]));
    Serial.print("sunsetTime         : "); Serial.print(strTime(daily->sunsetTime[i]));
    Serial.print("Moon phase         : "); Serial.println(daily->moonPhase[i]);
    Serial.print("precipIntensity    : "); Serial.println(daily->precipIntensity[i]);
    Serial.print("precipProbability  : "); Serial.println(daily->precipProbability[i]);
    Serial.print("precipType         : "); Serial.println(daily->precipType[i]);
    Serial.print("precipAccumulation : "); Serial.println(daily->precipAccumulation[i]);
    Serial.print("temperatureHigh    : "); Serial.println(daily->temperatureHigh[i]);
    Serial.print("temperatureLow     : "); Serial.println(daily->temperatureLow[i]);
    Serial.print("humidity           : "); Serial.println(daily->humidity[i]);
    Serial.print("pressure           : "); Serial.println(daily->pressure[i]);
    Serial.print("windSpeed          : "); Serial.println(daily->windSpeed[i]);
    Serial.print("windGust           : "); Serial.println(daily->windGust[i]);
    Serial.print("windBearing        : "); Serial.println(daily->windBearing[i]);
    Serial.print("cloudCover         : "); Serial.println(daily->cloudCover[i]);
    Serial.println();
  }
}
// Convert unix time to a time string, ends with a line feed...
String strTime(time_t unixTime)
{
  unixTime += (timeZone * 3600UL);
  return ctime(&unixTime);
}

