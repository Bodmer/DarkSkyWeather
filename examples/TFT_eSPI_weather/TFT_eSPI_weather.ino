//  Example from Dark Sky Weather library: https://github.com/Bodmer/DarkSkyWeather
//  Adapted by Bodmer to use the TFT_eSPI library:  https://github.com/Bodmer/TFT_eSPI

//  ################################## IMPORTANT ###################################
//  Upload the fonts and icons to SPIFFS (must set at least 2M for SPIFFS) using the
//  "Tools" "ESP8266 Sketch Data Upload" menu option in the IDE.  To add this option
//  follow instructions here: https://github.com/esp8266/arduino-esp8266fs-plugin
//  Close the IDE and open again to see the new menu option.

//  This sketch uses ESP8266 libraries and is not currently compatible with the ESP32.

//  Modify setup in All_Settings.h tab to configure your location etc
//  ESP8266 pin connections to the TFT are defined in the TFT_eSPI library.
//  ################################################################################

//  Original by Daniel Eichhorn, see license at end of file.

//  Changes:
//  Minor changes to text placement and auto-blanking out old text with background colour padding
//  Moon phase text added
//  Forecast text lines are automatically split onto two lines at a central space (some are long!)
//  Time is printed with colons aligned to tidy display
//  Min and max forecast temperatures spaced out
//  New smart splash startup screen and updated progress messages
//  Display does not need to be blanked between updates
//  Icons nudged about slightly to add wind direction + speed
//  Barometric pressure added

//  Adapted to use the DarkSkyWeather library: https://github.com/Bodmer/DarkSkyWeather
//  Moon rise/set (not provided by Dark Sky) replace with  and cloud cover humidity
//  Created and added new 100x100 and 50x50 pixel weather icons, these are in the
//  sketch data folder, press Ctrl+K to view
//  Add moon icons, eliminate all downloads of icons (may lose server!)
//  Addapted to use anti-aliased fonts, tweaked coords
//  Added forecast for 4th day
//  Added cloud cover and humidity in lieu of Moon rise/set

//#define SERIAL_MESSAGES // For serial output weather reports
//#define SCREEN_SERVER   // For dumping screentshots from TFT
//#define RANDOM_LOCATION // Test only, selects random weather location every refresh

/***************************************************************************************
**                          Load the libraries and settings
***************************************************************************************/
#include <Arduino.h>

#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library

// Additional UI functions
#include "GfxUi.h" // Attached to this sketch

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

// Helps with connecting to internet
#include <WiFiManager.h>

// check settings.h for adapting to your needs
#include "All_Settings.h"

#include <JsonStreamingParser.h> // Load via IDE manager, tested with v 1.0.5

#include <DarkSkyWeather.h>

#include "NTP_Time.h" // Attached to this sketch

/***************************************************************************************
**                          Define the globals and class instances
***************************************************************************************/
// HOSTNAME for OTA update
#define HOSTNAME "ESP8266-OTA-"

TFT_eSPI tft = TFT_eSPI();             // Invoke custom library
//TFT_eSprite spr = TFT_eSprite(&tft); // Sprites not used

DSWforecast dsw;      // Weather forcast library instance

DSW_current *current; // Pointers to structs that holds weather data
DSW_hourly  *hourly;
DSW_daily   *daily;

boolean booted = true;

GfxUi ui = GfxUi(&tft);

long lastDownloadUpdate = millis();

/***************************************************************************************
**                          Declare prototypes
***************************************************************************************/
void configModeCallback (WiFiManager *myWiFiManager);
void updateData();
void drawProgress(uint8_t percentage, String text);
void drawTime();
void drawCurrentWeather();
void drawForecast();
void drawForecastDetail(uint16_t x, uint16_t y, uint8_t dayIndex);
String getMeteoconIcon(String iconText);
void drawAstronomy();
void drawSeparator(uint16_t y);


/***************************************************************************************
**                          Setup
***************************************************************************************/
void setup() {
  Serial.begin(250000);

  tft.begin();
  tft.fillScreen(TFT_BLACK);

  SPIFFS.begin();
  listFiles();

  //Uncomment if you want to erase SPIFFS and update all internet resources, this takes some time!
  //tft.drawString("Formatting SPIFFS, so wait!", 120, 200); SPIFFS.format();

  if (SPIFFS.exists("/splash/DarkSky.jpg")   == true) ui.drawJpeg("/splash/DarkSky.jpg",   0, 0);

  delay(2000);
  //screenServer();
  tft.fillRect(0, 206, 240, 320 - 206, TFT_BLACK);

  tft.loadFont(AA_FONT_SMALL);
  //tft.setFreeFont(&ArialRoundedMTBold_14);
  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);

  tft.drawString("Original by: blog.squix.org", 120, 260);
  tft.drawString("Adapted by: Bodmer", 120, 280);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);

  delay(2000);

  tft.fillRect(0, 206, 240, 320 - 206, TFT_BLACK);
  tft.drawString("Connecting to WiFi", 120, 240);
  tft.setTextPadding(240); // Pad next drawString() text to full width to over-write old text

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  // Uncomment for testing wifi manager
  //wifiManager.resetSettings();
  wifiManager.setAPCallback(configModeCallback);

  //or use this for auto generated name ESP + ChipID
  wifiManager.autoConnect();

  //Manual Wifi connection
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

  tft.setTextDatum(BC_DATUM);
  tft.setTextPadding(240); // Pad next drawString() text to full width to over-write old text
  tft.drawString(" ", 120, 220);  // Clear line above using set padding width
  tft.drawString("Fetching weather data...", 120, 240);
  //delay(500);

  // Fetch the time
  syncTime();

  tft.unloadFont();
}

/***************************************************************************************
**                          Loop
***************************************************************************************/
void loop() {
  // Handle OTA update requests
  ArduinoOTA.handle();

  // Check if we should update weather information
  if (booted || (millis() - lastDownloadUpdate > 1000UL * UPDATE_INTERVAL_SECS)) {
    updateData();
    lastDownloadUpdate = millis();
  }

  // If minute has changed then request new time from NTP server
  if (booted || minute() != lastMinute) {
    drawTime();
    lastMinute = minute();
    syncTime();
#ifdef SCREEN_SERVER
    screenServer();
#endif
  }

  booted = false;
}

/***************************************************************************************
**                          WiFi Manager configModeCallback screen message
***************************************************************************************/
// Called if WiFi has not been configured yet
void configModeCallback (WiFiManager *myWiFiManager) {
  tft.fillScreen(TFT_BLACK);

  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.setTextPadding(240);
  tft.drawString("Wifi Manager", 120, 28);
  tft.drawString("Please connect to AP", 120, 42);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(myWiFiManager->getConfigPortalSSID(), 120, 56);
  tft.drawString("http://192.168.4.1/", 120, 70);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.drawString("To setup Wifi Configuration", 120, 86);

  tft.setTextPadding(0);
}

/***************************************************************************************
**                          Update progress bar
***************************************************************************************/
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
    tft.drawString(String(percentage) + "%", 120, 285);
    ui.drawProgressBar(10, 260, 240 - 20, 15, percentage, TFT_WHITE, TFT_BLUE);
  }
}

/***************************************************************************************
**                          Fetch the weather data  and update screen
***************************************************************************************/
// Update the internet based information and update screen
void updateData() {
  // booted = true;  // Test only
  // booted = false; // Test only

  tft.loadFont(AA_FONT_SMALL);

  if (booted) drawProgress(20, "Updating time...");
  else fillSegment(22, 22, 0, (int) (20 * 3.6), 16, TFT_NAVY);

  if (booted) drawProgress(50, "Updating conditions...");
  else fillSegment(22, 22, 0, (int) (50 * 3.6), 16, TFT_NAVY);

  // Create the structures that hold the retrieved weather
  current = new DSW_current;
  hourly =  new DSW_hourly;
  daily =   new DSW_daily;

#ifdef RANDOM_LOCATION // Randomly choose a place on Earth
  String latitude = "";
  latitude = (random(180) - 90);
  String longitude = "";
  longitude = (random(360) - 180);
#endif

  bool parsed = dsw.getForecast(current, hourly, daily, api_key, latitude, longitude, units, language);
  printWeather(); // For debug

  if (booted)
  {
    drawProgress(100, "Done...");
    delay(2000);
    tft.fillScreen(TFT_BLACK);
  }
  else
  {
    fillSegment(22, 22, 0, 360, 16, TFT_NAVY);
    fillSegment(22, 22, 0, 360, 22, TFT_BLACK);
  }

  //tft.fillScreen(TFT_CYAN); // For text padding and update graphics over-write checking only

  if (parsed)
  {
    drawCurrentWeather();
    drawForecast();
    drawAstronomy();

    tft.unloadFont();

    // Update the temperature here so we dont need keep
    // loading and unloading font which takes time
    tft.loadFont(AA_FONT_LARGE);
    tft.setTextDatum(TR_DATUM);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);

    // Font ASCII code 0xB0 is a degree symbol
    tft.setTextPadding(tft.textWidth(" -88")); // Max width of values

    String weatherText = "";
    weatherText = (int16_t) current->temperature;  // Make it integer temperature
    tft.drawString(weatherText, 215, 95); //  + "°" symbol is big... use o in small font
  }
  else
  {
    Serial.println("Failed to get weather");
  }

  // Delete to free up space
  delete current;
  delete hourly;
  delete daily;

  tft.unloadFont();
}

/***************************************************************************************
**                          Update progress bar
***************************************************************************************/
void drawProgress(uint8_t percentage, String text) {
  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.setTextPadding(240);
  tft.drawString(text, 120, 260);

  ui.drawProgressBar(10, 269, 240 - 20, 15, percentage, TFT_WHITE, TFT_BLUE);

  tft.setTextPadding(0);
}

/***************************************************************************************
**                          Draw the clock digits
***************************************************************************************/
void drawTime() {
  tft.loadFont(AA_FONT_LARGE);

  time_t local_time = TIMEZONE.toLocal(now(), &tz1_Code);  // Convert UTC to local time, returns zone code e.g "GMT"

  String timeNow = "";

  if (hour(local_time) < 10) timeNow += "0";
  timeNow += hour(local_time);
  timeNow += ":";
  if (minute(local_time) < 10) timeNow += "0";
  timeNow += minute(local_time);

  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextPadding(tft.textWidth(" 44:44 "));  // String width + margin
  tft.drawString(timeNow, 120, 53);

  drawSeparator(51);

  tft.setTextPadding(0);

  tft.unloadFont();
}

/***************************************************************************************
**                          Draw the current weather
***************************************************************************************/
void drawCurrentWeather() {
  String date = "Updated: " + strDate(current->time);

  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.setTextPadding(tft.textWidth(" Updated: Mmm 44 44:44 "));  // String width + margin
  tft.drawString(date, 120, 16);

  String weatherIcon = getMeteoconIcon(current->icon);

  //uint32_t dt = millis();
  ui.drawBmp("/icon100/" + weatherIcon + ".bmp", 0, 53);
  //Serial.print("Icon draw time = "); Serial.println(millis()-dt);

  // Weather Text
  String weatherText = current->summary;

  tft.setTextDatum(BR_DATUM);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);

  int splitPoint = 0;
  int xpos = 235;
  splitPoint =  splitIndex(weatherText);

  tft.setTextPadding(xpos - 100);  // xpos - icon width
  if (splitPoint) tft.drawString(weatherText.substring(0, splitPoint), xpos, 69);
  else tft.drawString(" ", xpos, 69);
  tft.drawString(weatherText.substring(splitPoint), xpos, 86);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextDatum(TR_DATUM);
  tft.setTextPadding(0);
  if (units == "si") tft.drawString("oC", 237, 95);
  else  tft.drawString("oF", 237, 95);

  //Temperature large digits added in updateData() to save swapping font here
 
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  weatherText = (uint16_t)current->windSpeed;
  if (units == "si") weatherText += " m/s";
  else if (units == "us" || units == "uk2") weatherText += " mph";
  else if (units == "ca") weatherText += " kph";

  tft.setTextDatum(TC_DATUM);
  tft.setTextPadding(tft.textWidth("888 m/s")); // Max string length?
  tft.drawString(weatherText, 124, 136);

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

  int windAngle = (current->windBearing + 22.5) / 45;
  if (windAngle > 7) windAngle = 0;
  String wind[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW" };
  ui.drawBmp("/wind/" + wind[windAngle] + ".bmp", 101, 86);

/*
  tft.fillCircle(126, 110, 24, TFT_BLACK); // Erase old plot, radius + 2 to delete stray pixels
  tft.drawCircle(126, 110, 22, TFT_DARKGREY);    // Outer ring - optional
  if ( windAngle >= 0 ) fillSegment(126, 110, windAngle - 15, 30, 22, TFT_GREEN);
  tft.drawCircle(126, 110, 6, TFT_RED);
*/
  drawSeparator(153);

  tft.setTextDatum(TL_DATUM); // Reset datum to normal
  tft.setTextPadding(0); // Reset padding width to none
}

/***************************************************************************************
**                          Draw the 4 forecast columns
***************************************************************************************/
// draws the three forecast columns
void drawForecast() {
  int8_t dayIndex = 0;
  while ((daily->time[dayIndex] < current->time) && (dayIndex < (MAX_DAYS - 4))) dayIndex++;
  drawForecastDetail(  8, 171, dayIndex);
  drawForecastDetail( 66, 171, dayIndex + 1); // was 95
  drawForecastDetail(124, 171, dayIndex + 2); // was 180
  drawForecastDetail(182, 171, dayIndex + 3); // was 180
  drawSeparator(171 + 69);
}

/***************************************************************************************
**                          Draw 1 forecast column at x, y
***************************************************************************************/
// helper for the forecast columns
void drawForecastDetail(uint16_t x, uint16_t y, uint8_t dayIndex) {
  String day  = dayShortStr(weekday(TIMEZONE.toLocal(daily->time[dayIndex], &tz1_Code)));
  day.toUpperCase();

  tft.setTextDatum(BC_DATUM);

  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.setTextPadding(tft.textWidth("WWW"));
  tft.drawString(day, x + 25, y);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextPadding(tft.textWidth("-88   -88"));
  String highTemp = String(daily->temperatureHigh[dayIndex], 0);
  String lowTemp  = String(daily->temperatureLow[dayIndex], 0);
  tft.drawString(highTemp + " " + lowTemp, x + 25, y + 17);

  String weatherIcon = getMeteoconIcon(daily->icon[dayIndex]);

  ui.drawBmp("/icon50/" + weatherIcon + ".bmp", x, y + 18);

  tft.setTextPadding(0); // Reset padding width to none
}

/***************************************************************************************
**                          Draw Sun rise/set, Moon, cloud cover and humidity
***************************************************************************************/
void drawAstronomy() {
  // Moon phase strings...
  String moonPhase [8] = {"New", "Waxing", "1st qtr", "Waxing", "Full", "Waning", "Last qtr", "Waning"};

  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextPadding(tft.textWidth(" Last qtr "));

  int8_t dayIndex = 0;

  //The first daily summary may be yesterday, search for time > now
  //while ((daily->time[dayIndex] < current->time) && (dayIndex < MAX_DAYS)) dayIndex++;

  uint8_t moonIndex = (uint8_t)(((float)daily->moonPhase[dayIndex] + 6.25) / 12.5);
  if (moonIndex > 7) moonIndex = 0;
  tft.drawString(moonPhase[moonIndex], 120, 319);

  uint8_t moonAgeImage = (24.0 * daily->moonPhase[dayIndex]) / 100;
  if (moonAgeImage > 23) moonAgeImage = 0;
  ui.drawBmp("/moon/moonphase_L" + String(moonAgeImage) + ".bmp", 120 - 30, 318 - 16 - 60);

  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.setTextPadding(0); // Reset padding width to none
  tft.drawString("Sun", 40, 270);

  tft.setTextDatum(BR_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextPadding(tft.textWidth(" 88:88 "));

  String rising = strTime(daily->sunriseTime[dayIndex]) + " ";
  int dt = rightOffset(rising, ":"); // Draw relative to colon to them aligned
  tft.drawString(rising, 40 + dt, 290);

  String setting = strTime(daily->sunsetTime[dayIndex]) + " ";
  dt = rightOffset(setting, ":");
  tft.drawString(setting, 40 + dt, 305);

  /* Dark Sky does not provide Moon rise and setting times
     There are other free API providers:
     http://api.usno.navy.mil/rstt/oneday?date=1/5/2005&loc=Los%20Angeles,%20CA
  */

  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.drawString("Cloud", 195, 260);

  String cloudCover = "";
  cloudCover += current->cloudCover;
  cloudCover += "%";

  tft.setTextDatum(BR_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextPadding(tft.textWidth(" 100%"));
  tft.drawString(cloudCover, 210, 277);

  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.drawString("Humidity", 195, 300 - 2);

  String humidity = "";
  humidity += current->humidity;
  humidity += "%";

  tft.setTextDatum(BR_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextPadding(tft.textWidth("100%"));
  tft.drawString(humidity, 210, 315);

  tft.setTextPadding(0); // Reset padding width to none
}

/***************************************************************************************
**                          Translate icon string to bitmap file name
***************************************************************************************/
String getMeteoconIcon(String iconText) {
  if (iconText == "clear-day") return "clear";
  if (iconText == "clear-night") return "clearNight"; // Need a moon
  if (iconText == "cloudy") return "cloudy";
  if (iconText == "fog") return "fog";
  if (iconText == "partly-cloudy-day") return "partlyCloudy";
  if (iconText == "partly-cloudy-night") return "partlyCloudyNight";
  if (iconText == "sleet") return "sleet";
  if (iconText == "rain") return "rain";
  if (iconText == "snow") return "snow";
  if (iconText == "wind") return "wind";

  return "unknown";
}

/***************************************************************************************
**                          Draw screen section separator line
***************************************************************************************/
// if you want separators, uncomment the tft-line
void drawSeparator(uint16_t y) {
  tft.drawFastHLine(10, y, 240 - 2 * 10, 0x4228);
}

/***************************************************************************************
**                          Determine place to split a line line
***************************************************************************************/
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

/***************************************************************************************
**                          Right side offset to a character
***************************************************************************************/
// Calculate coord delta from start of text String to start of sub String contained within that text
// Can be used to vertically right align text so for example a colon ":" in the time value is always
// plotted at same point on the screen irrespective of different proportional character widths,
// could also be used to align decimal points for neat formatting
int rightOffset(String text, String sub)
{
  int index = text.indexOf(sub);
  return tft.textWidth(text.substring(index));
}

/***************************************************************************************
**                          Left side offset to a character
***************************************************************************************/
// Calculate coord delta from start of text String to start of sub String contained within that text
// Can be used to vertically left align text so for example a colon ":" in the time value is always
// plotted at same point on the screen irrespective of different proportional character widths,
// could also be used to align decimal points for neat formatting
int leftOffset(String text, String sub)
{
  int index = text.indexOf(sub);
  return tft.textWidth(text.substring(0, index));
}

/***************************************************************************************
**                          Draw circle segment
***************************************************************************************/
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

/***************************************************************************************
**                          Print the weather info to the Serial Monitor
***************************************************************************************/
void printWeather(void)
{
#ifdef SERIAL_MESSAGES
  Serial.println("Weather from Dark Sky\n");

  // We can use the timezone to set the offset eventually...
  Serial.print("Timezone           : "); Serial.println(current->timezone);

  Serial.println("############### Current weather ###############\n");
  Serial.print("time               : "); Serial.println(strDate(current->time));
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
      Serial.print("time               : "); Serial.println(strDate(hourly->time[i]));
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

  for (int i = 0; i < 5; i++)
  {
    Serial.print("Day summary   "); Serial.print(i); Serial.print("    : "); Serial.println(daily->summary[i]);
    Serial.print("time               : "); Serial.println(strDate(daily->time[i]));
    Serial.print("icon               : "); Serial.println(daily->icon[i]);
    Serial.print("sunriseTime        : "); Serial.println(strDate(daily->sunriseTime[i]));
    Serial.print("sunsetTime         : "); Serial.println(strDate(daily->sunsetTime[i]));
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
#endif
}
// Convert unix time to a time string
String strTime(time_t unixTime)
{
  time_t local_time = TIMEZONE.toLocal(unixTime, &tz1_Code);

  String localTime = "";

  if (hour(local_time) < 10) localTime += "0";
  localTime += hour(local_time);
  localTime += ":";
  if (minute(local_time) < 10) localTime += "0";
  localTime += minute(local_time);

  return localTime;
}

// Convert unix time to a time string, ends with a line feed...
String strDate(time_t unixTime)
{
  time_t local_time = TIMEZONE.toLocal(unixTime, &tz1_Code);

  String localDate = "";

  localDate += monthShortStr(month(local_time));
  localDate += " ";
  localDate += day(local_time);
  localDate += " " + strTime(unixTime);

  return localDate;
}


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
