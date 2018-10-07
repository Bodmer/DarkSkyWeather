// Sketch to fetch the Weather Forecast from Dark Sky
// an example from the library here:
// https://github.com/Bodmer/DarkSkyWeather

// Sign up for a key and read API configuration info here:
// https://darksky.net/dev

// An example that will eventually be targetted at the TFT_eSPI library
// for displaying weather on TFT or ePaper displays:
// https://github.com/Bodmer/TFT_eSPI

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#include <JsonStreamingParser.h>
#include <DarkSkyWeather.h>

// Just using this library for unix time conversion
#include <Time.h>

// =====================================================
// ========= User configured stuff starts here =========

#define TIME_OFFSET 0UL * 3600UL // UTC + 0 hour

// Change to suit your WiFi router
#define SSID "SSID"
#define SSID_PASSWORD "password"

// Dark Sky API Details, replace x's with y0ur API key
String api_key = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"; // Obtain this from your Dark Sky account

// Set both longitude and latitude to at least 4 decimal places
String latitude =  "27.9881"; // 90.0000 to -90.0000 negative for Southern hemisphere
String longitude = "86.9250"; // 180.000 to -180.000 negative for West

String units = "si";  // See notes tab
String language = ""; // See notes tab

// =========  User configured stuff ends here  =========
// =====================================================

DSWforecast dsw; // Weather forcast library instance

void setup() { 
  Serial.begin(250000); // Fast to stop it holding up the stream
  
  Serial.printf("Connecting to %s\n", SSID);

  WiFi.hostname("DarkSky");

  WiFi.begin(SSID, SSID_PASSWORD);
   
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected\n");
}

void loop() {

  displayCurrentWeather();

  // We can make 1000 requests a day
  delay(2 * 60 * 1000); // Every 2 minutes = 720 requests per day
}


void displayCurrentWeather()
{
  // Create the structures that hold the retrieved weather
  DSW_current *current = new DSW_current;
  DSW_hourly *hourly = new DSW_hourly;
  DSW_daily  *daily = new DSW_daily;

  time_t time;

  Serial.print("\nRequesting weather information from DarkSky.net... ");

  dsw.getForecast(current, hourly, daily, api_key, latitude, longitude, units, language);

  Serial.println("Weather from Dark Sky\n");

  // We can use the timezone to set the offset eventually...
  // Serial.print("Timezone            : "); Serial.println(current->timezone);
  
  Serial.println("############### Current weather ###############\n");
  Serial.print("Current summary     : "); Serial.println(current->summary);
  Serial.print("Current precipInten : "); Serial.println(current->precipIntensity);
  Serial.print("Current precipType  : "); Serial.println(current->precipType);
  Serial.print("Current temperature : "); Serial.println(current->temperature);
  Serial.print("Current humidity    : "); Serial.println(current->humidity);
  Serial.print("Current pressure    : "); Serial.println(current->pressure);
  Serial.print("Current wind speed  : "); Serial.println(current->windSpeed);
  Serial.print("Current wind gust   : "); Serial.println(current->windGust);
  Serial.print("Current wind dirn   : "); Serial.println(current->windBearing);

  Serial.println();

  Serial.println("############### Hourly weather  ###############\n");
  Serial.print("Overall hourly summary : "); Serial.println(hourly->overallSummary);
  for (int i = 0; i<MAX_HOURS; i++)
  {
    Serial.print("Hourly summary ");Serial.print(i);Serial.print(" : "); Serial.println(hourly->summary[i]);
    Serial.print("Time              : "); Serial.print(strTime(hourly->time[i]));
    Serial.print("precipIntensity   : "); Serial.println(hourly->precipIntensity[i]);
    Serial.print("precipType        : "); Serial.println(hourly->precipType[i]);
    Serial.print("temperature       : "); Serial.println(hourly->temperature[i]);
    Serial.print("pressure          : "); Serial.println(hourly->pressure[i]);
    Serial.println();
  }

  Serial.println("###############  Daily weather  ###############\n");
  Serial.print("Daily summary     : "); Serial.println(daily->overallSummary);
  Serial.println();

  for (int i = 0; i<8; i++)
  {
    Serial.print("Daily summary ");Serial.print(i);Serial.print("   : "); Serial.println(daily->summary[i]);
    Serial.print("time              : "); Serial.print(strTime(daily->time[i]));
    Serial.print("Icon              : "); Serial.println(daily->icon[i]);
    Serial.print("sunriseTime       : "); Serial.print(strTime(daily->sunriseTime[i]));
    Serial.print("sunsetTime        : "); Serial.print(strTime(daily->sunsetTime[i]));
    Serial.print("Moon phase        : "); Serial.println(daily->moonPhase[i]);
    Serial.print("precipIntensity   : "); Serial.println(daily->precipIntensity[i]);
    Serial.print("precipProbability : "); Serial.println(daily->precipProbability[i]);
    Serial.print("precipType        : "); Serial.println(daily->precipType[i]);
    Serial.print("temperatureHigh   : "); Serial.println(daily->temperatureHigh[i]);
    Serial.print("temperatureLow    : "); Serial.println(daily->temperatureLow[i]);
    Serial.print("humidity          : "); Serial.println(daily->humidity[i]);
    Serial.print("pressure          : "); Serial.println(daily->pressure[i]);
    Serial.print("windSpeed         : "); Serial.println(daily->windSpeed[i]);
    Serial.print("windGust          : "); Serial.println(daily->windGust[i]);
    Serial.print("windBearing       : "); Serial.println(daily->windBearing[i]);
    Serial.print("cloudCover        : "); Serial.println(daily->cloudCover[i]);
    Serial.println();
  }

  // Delete to free up space and prevent fragmentation as strings change in length
  delete current;
  delete hourly;
  delete daily;
}

// Convert unix time to a time string
String strTime(time_t unixTime)
{
  unixTime += TIME_OFFSET;
  return ctime(&unixTime);
}

