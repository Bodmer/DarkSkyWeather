// Sketch for ESP32 to fetch the Weather Forecast from Dark Sky
// an example from the library here:
// https://github.com/Bodmer/DarkSkyWeather

// Sign up for a key and read API configuration info here:
// https://darksky.net/dev

// Choose library to load
#ifdef ESP8266
  #include <ESP8266WiFi.h>
  #include <WiFiClientSecure.h>
#else // ESP32
  #include <WiFi.h>
#endif

#include <JSON_Decoder.h>

#include <DarkSkyWeather.h>

// Just using this library for unix time conversion
#include <Time.h>

// =====================================================
// ========= User configured stuff starts here =========
// Further configuration settings can be found in the
// DarkSkyWeather library "User_Setup.h" file

#define TIME_OFFSET 0UL * 3600UL // UTC + 0 hour

// Change to suit your WiFi router
#define SSID "Your_SSID"
#define SSID_PASSWORD "Your_password"

// Dark Sky API Details, replace x's with your API key
String api_key = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"; // Obtain this from your Dark Sky account

// Set both longitude and latitude to at least 4 decimal places
String latitude =  "27.9881"; // 90.0000 to -90.0000 negative for Southern hemisphere
String longitude = "86.9250"; // 180.000 to -180.000 negative for West

String units = "si";  // See notes tab
String language = ""; // See notes tab

// =========  User configured stuff ends here  =========
// =====================================================

DS_Weather dsw; // Weather forecast library instance

void setup() { 
  Serial.begin(250000); // Fast to stop it holding up the stream

  Serial.printf("Connecting to %s\n", SSID);

  WiFi.begin(SSID, SSID_PASSWORD);
   
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected\n");
}

void loop() {

  printCurrentWeather();

  // We can make 1000 requests a day
  delay(5 * 60 * 1000); // Every 5 minutes = 288 requests per day
}

/***************************************************************************************
**                          Send weather info to serial port
***************************************************************************************/
void printCurrentWeather()
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
  Serial.print("Current time             : "); Serial.print(strTime(current->time));
  Serial.print("Current summary          : "); Serial.println(current->summary);
  Serial.print("Current icon             : "); Serial.println(getMeteoconIcon(current->icon));
  Serial.print("Current precipInten      : "); Serial.println(current->precipIntensity);
  Serial.print("Current precipType       : "); Serial.println(getMeteoconIcon(current->precipType));
  Serial.print("Current precipProbability: "); Serial.println(current->precipProbability);
  Serial.print("Current temperature      : "); Serial.println(current->temperature);
  Serial.print("Current humidity         : "); Serial.println(current->humidity);
  Serial.print("Current pressure         : "); Serial.println(current->pressure);
  Serial.print("Current wind speed       : "); Serial.println(current->windSpeed);
  Serial.print("Current wind gust        : "); Serial.println(current->windGust);
  Serial.print("Current wind dirn        : "); Serial.println(current->windBearing);

  Serial.println();

  Serial.println("############### Hourly weather  ###############\n");
  Serial.print("Overall hourly summary : "); Serial.println(hourly->overallSummary);
  for (int i = 0; i<MAX_HOURS; i++)
  {
    Serial.print("Hourly summary  "); if (i<10) Serial.print(" ");
    Serial.print(i); Serial.print(" : "); Serial.println(hourly->summary[i]);
    Serial.print("Time               : "); Serial.print(strTime(hourly->time[i]));
    Serial.print("precipIntensity    : "); Serial.println(hourly->precipIntensity[i]);
    Serial.print("precipProbability  : "); Serial.println(hourly->precipProbability[i]);
    Serial.print("precipType         : "); Serial.println(hourly->precipType[i]);
    Serial.print("precipAccumulation : "); Serial.println(hourly->precipAccumulation[i]);
    Serial.print("temperature        : "); Serial.println(hourly->temperature[i]);
    Serial.print("pressure           : "); Serial.println(hourly->pressure[i]);
    Serial.print("cloudCover         : "); Serial.println(hourly->cloudCover[i]);
    Serial.println();
  }

  Serial.println("###############  Daily weather  ###############\n");
  Serial.print("Daily summary     : "); Serial.println(daily->overallSummary);
  Serial.println();

  for (int i = 0; i<MAX_DAYS; i++)
  {
    Serial.print("Daily summary   ");
    Serial.print(i); Serial.print(" : "); Serial.println(daily->summary[i]);
    Serial.print("time              : "); Serial.print(strTime(daily->time[i]));
    Serial.print("Icon              : "); Serial.println(getMeteoconIcon(daily->icon[i]));
    Serial.print("sunriseTime       : "); Serial.print(strTime(daily->sunriseTime[i]));
    Serial.print("sunsetTime        : "); Serial.print(strTime(daily->sunsetTime[i]));
    Serial.print("Moon phase        : "); Serial.println(daily->moonPhase[i]);
    Serial.print("precipIntensity   : "); Serial.println(daily->precipIntensity[i]);
    Serial.print("precipProbability : "); Serial.println(daily->precipProbability[i]);
    Serial.print("precipType        : "); Serial.println(daily->precipType[i]);
    Serial.print("precipAccumulation: "); Serial.println(daily->precipAccumulation[i]);
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

/***************************************************************************************
**                          Convert unix time to a time string
***************************************************************************************/
String strTime(time_t unixTime)
{
  unixTime += TIME_OFFSET;
  return ctime(&unixTime);
}

/***************************************************************************************
**                          Get the icon file name from the index number
***************************************************************************************/
const char* getMeteoconIcon(uint8_t index)
{
  if (index > MAX_ICON_INDEX) index = 0;
  return dsw.iconName(index);
}
