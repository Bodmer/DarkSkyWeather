// Client library for the Dark Sky weather datapoint server
// https://darksky.net/dev

// Created by Bodmer 24/9/2018
// This is a beta test version and is subject to change!

// See license.txt in root folder of library

#ifdef ESP8266
  #include <ESP8266WiFi.h>
#else
  #include <WiFi.h>
#endif

#include <WiFiClientSecure.h>

// The streaming parser to use is not the Arduino IDE library manager default,
// but this one which is slightly different and renamed to avoid conflicts:
// https://github.com/Bodmer/JSON_Decoder


#include <JSON_Listener.h>
#include <JSON_Decoder.h>

#include "DarkSkyWeather.h"


/***************************************************************************************
** Function name:           getForecast
** Description:             Setup the weather forecast request from darksky.net
***************************************************************************************/
// The structures etc are created by the sketch and passed to this function.
// Pass a nullptr for current, hourly or daily pointers to exclude in response.
bool DS_Weather::getForecast(DSW_current *current, DSW_hourly *hourly, DSW_daily *daily,
                             String api_key, String latitude, String longitude,
                             String units, String language) {

  data_set = "";
  hourly_index = 0;
  daily_index = 0;

  // Local copies of structure pointers, the structures are filled during parsing
  this->current = current;
  this->hourly  = hourly;
  this->daily   = daily;

#if defined (MINIMISE_DATA_POINTS) // If defined in DarSkyWeather library "User_Setup.h"
   hourly = nullptr;
#endif

  // Exclude some info by passing fn a NULL pointer to reduce memory needed
  String exclude = "";
  if (!current) exclude += "currently,";   // summary, then current weather
  if (!hourly)  exclude += "hourly,";      // summary, then weather every hour for 48 hours
  if (!daily)   exclude += "daily,";       // summary, then daily detailed weather for one week (7 days)
  exclude += "minutely,"; // not supported yet, summary, then rain predictions every minute for next hour
  exclude += "alerts,";   // special warnings, typically none
  exclude += "flags";     // misc info

  String url = "https://api.darksky.net/forecast/" + api_key + "/"
               + latitude + "," + longitude + "?exclude=" + exclude
               + "&units=" + units + "&lang=" + language;

  // Send GET request and feed the parser
  bool result = parseRequest(url);

  // Null out pointers to prevent crashes
  this->current = nullptr;
  this->hourly  = nullptr;
  this->daily   = nullptr;

  return result;
}

#ifdef ESP32 // Decide if ESP32 or ESP8266 parseRequest available

/***************************************************************************************
** Function name:           parseRequest (for ESP32)
** Description:             Fetches the JSON message and feeds to the parser
***************************************************************************************/
bool DS_Weather::parseRequest(String url) {

  uint32_t dt = millis();

  // This certificate will expire in June 2019, but we can ignore it at line 111
  const char* dsw_ca_cert = \
  "-----BEGIN CERTIFICATE-----\n" \
  "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n" \
  "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n" \
  "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n" \
  "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n" \
  "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n" \
  "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n" \
  "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n" \
  "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n" \
  "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n" \
  "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n" \
  "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n" \
  "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n" \
  "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n" \
  "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n" \
  "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n" \
  "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n" \
  "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n" \
  "rqXRfboQnoZsG4q5WTP468SQvvG5\n" \
  "-----END CERTIFICATE-----\n";

  WiFiClientSecure client;

  
  //client.setCACert(dsw_ca_cert);  // Comment out to stop certificate check

  JSON_Decoder parser;
  parser.setListener(this);

  const char*  host = "api.darksky.net";

  if (!client.connect(host, 443))
  {
    Serial.println("Connection failed.");
    return false;
  }

  uint32_t timeout = millis();
  char c = 0;
  int ccount = 0;
  uint32_t readCount = 0;
  parseOK = false;

  // Send GET request
  Serial.println("\nSending GET request to api.darksky.net...");
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");

  // Pull out any header, X-Forecast-API-Calls: reports current daily API call count
  while (client.connected())
  {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("Header end found");
      break;
    }

#ifdef SHOW_HEADER
    Serial.println(line);
#else
    // Show the API call count
    if (line.indexOf("X-Forecast-API-Calls") >= 0) Serial.println(line);
#endif

    if ((millis() - timeout) > 5000UL)
    {
      Serial.println ("HTTP header timeout");
      client.stop();
      return false;
    }
  }

  Serial.println("\nParsing JSON");

  // Parse the JSON data, available() includes yields
  while ( client.available() > 0 || client.connected())
  {
    while(client.available() > 0)
    {
      c = client.read();
      parser.parse(c);
#ifdef SHOW_JSON
      if (c == '{' || c == '[' || c == '}' || c == ']') Serial.println();
      Serial.print(c); if (ccount++ > 100 && c == ',') {ccount = 0; Serial.println();}
#endif

      if ((millis() - timeout) > 8000UL)
      {
        Serial.println ("JSON parse client timeout");
        parser.reset();
        client.stop();
        return false;
      }
      yield();
    }
  }

  Serial.println("");
  Serial.print("Done in "); Serial.print(millis()-dt); Serial.println(" ms\n");

  parser.reset();

  client.stop();
  
  // A message has been parsed but the datapoint correctness is unknown
  return parseOK;
}

#else // ESP8266 version

/***************************************************************************************
** Function name:           parseRequest (for ESP8266)
** Description:             Fetches the JSON message and feeds to the parser
***************************************************************************************/
bool DS_Weather::parseRequest(String url) {

  uint32_t dt = millis();

  // SHA1 certificate fingerprint
  #if defined(AXTLS)
    const char* fingerprint = "EB:C2:67:D1:B1:C6:77:90:51:C1:4A:0A:BA:83:E1:F0:6D:73:DD:B8";
    WiFiClientSecure client;
  #else
    // Must use namespace:: to select BearSSL
    BearSSL::WiFiClientSecure client;

    #ifdef SECURE_SSL
      // BearSSL requires a different fingerprint format and setFingerprint() must be called
      const uint8_t fp[20] = {0xEB,0xC2,0x67,0xD1,0xB1,0xC6,0x77,0x90,0x51,0xC1,0x4A,0x0A,0xBA,0x83,0xE1,0xF0,0x6D,0x73,0xDD,0xB8};
      client.setFingerprint(fp);
    #else
      client.setInsecure();
    #endif
  #endif

  JSON_Decoder parser;
  parser.setListener(this);

  const char*  host = "api.darksky.net";

  if (!client.connect(host, 443))
  {
    Serial.println("Connection failed.");
    return false;
  }

#if defined(AXTLS)
  // BearSSL does not support verify() and always returns false.
  if (client.verify(fingerprint, host))
  {
    Serial.println("Certificate OK");
  }
  else
  {
      Serial.println("Bad certificate");
      client.stop();
      return false;
  }
#endif

  uint32_t timeout = millis();
  char c = 0;
  int ccount = 0;
  uint32_t readCount = 0;
  parseOK = false;

  // Send GET request
  Serial.println("Sending GET request to api.darksky.net...");
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");

  // Pull out any header, X-Forecast-API-Calls: reports current daily API call count
  while (client.available() || client.connected())
  {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("Header end found");
      break;
    }

#ifdef SHOW_HEADER
    Serial.println(line);
#else
    // Show the API call count
    if (line.indexOf("X-Forecast-API-Calls") >= 0) Serial.println(line);
#endif

    if ((millis() - timeout) > 5000UL)
    {
      Serial.println ("HTTP header timeout");
      client.stop();
      return false;
    }
  }

  Serial.println("Parsing JSON");
  
  // Parse the JSON data, available() includes yields
  while (client.available() || client.connected())
  {
    while (client.available())
    {
      c = client.read();
      parser.parse(c);
  #ifdef SHOW_JSON
      if (c == '{' || c == '[' || c == '}' || c == ']') Serial.println();
      Serial.print(c); if (ccount++ > 100 && c == ',') {ccount = 0; Serial.println();}
  #endif
    }

    if ((millis() - timeout) > 8000UL)
    {
      Serial.println ("JSON client timeout");
      parser.reset();
      client.stop();
      return false;
    }
  }

  Serial.println("");
  Serial.print("Done in "); Serial.print(millis()-dt); Serial.println(" ms\n");

  parser.reset();

  client.stop();
  
  // A message has been parsed without error but the datapoint correctness is unknown
  return parseOK;
}

#endif // ESP32 or ESP8266 parseRequest

/***************************************************************************************
** Function name:           key etc
** Description:             These functions are called while parsing the JSON message
***************************************************************************************/
void DS_Weather::key(const char *key) {

  currentKey = key;

#ifdef SHOW_CALLBACK
  Serial.print("\n>>> Key >>>" + (String)key);
#endif
}

void DS_Weather::startDocument() {

  currentParent = currentKey = "";
  objectLevel = 0;
  valuePath = "";
  arrayIndex = 0;
  parseOK = true;

#ifdef SHOW_CALLBACK
  Serial.print("\n>>> Start document >>>");
#endif
}

void DS_Weather::endDocument() {

  currentParent = currentKey = "";
  objectLevel = 0;
  valuePath = "";
  arrayIndex = 0;

#ifdef SHOW_CALLBACK
  Serial.print("\n<<< End document <<<");
#endif
}

void DS_Weather::startObject() {

  currentParent = currentKey;
  objectLevel++;

#ifdef SHOW_CALLBACK
  Serial.print("\n>>> Start object level:" + (String) objectLevel + " index:" + (String) arrayIndex +" >>>");
#endif
}

void DS_Weather::endObject() {

  currentParent = "";
  arrayIndex++;
  objectLevel--;

#ifdef SHOW_CALLBACK
  Serial.print("\n<<< End object <<<");
#endif
}

void DS_Weather::startArray() {

  arrayIndex  = 0;
  valuePath = currentParent + "/" + currentKey; // aka = current Object, e.g. "daily:data"

#ifdef SHOW_CALLBACK
  Serial.print("\n>>> Start array " + valuePath + "/" + (String) arrayIndex +" >>>");
#endif
}

void DS_Weather::endArray() {

  valuePath = "";

#ifdef SHOW_CALLBACK
  Serial.print("\n<<< End array <<<");
#endif
}

void DS_Weather::whitespace(char c) {
}

void DS_Weather::error( const char *message ) {
  Serial.print("\nParse error message: ");
  Serial.print(message);
  parseOK = false;
}

/***************************************************************************************
** Function name:           iconIndex
** Description:             Convert the icon name to an array index to save memory
***************************************************************************************/
uint8_t DS_Weather::iconIndex(const char *val)
{
  if (*val == 0) return MAX_ICON_INDEX; // null so return index for none

  uint8_t i = 0;
  for( i = 0; i <= MAX_ICON_INDEX; i++)
  {
    if (strcmp(iconList[i], val) == 0) break;
  }
  if ( i >= MAX_ICON_INDEX) i = 0;
  return i;
}

/***************************************************************************************
** Function name:           iconFilename
** Description:             Convert the icon array index to an icon filename
***************************************************************************************/
const char* DS_Weather::iconName(uint8_t index)
{
  return iconList[index];
}

/***************************************************************************************
** Function name:           value (full data set)
** Description:             Stores the parsed data in the structures for sketch access
***************************************************************************************/
 // Nested "if" with "return" reduces comparison count for each key
 
#ifndef MINIMISE_DATA_POINTS   // Collect full data point set if this is NOT defined <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

void DS_Weather::value(const char *val) {

   String value = val;

  // Start of JSON
  //if (currentParent == "") {
  //  if (currentKey == "timezone") current->timezone = value;
  //}

  // Current forecast - no array index - short path
  if (currentParent == "currently") {
    data_set = "currently";
    if (currentKey == "time") current->time = (uint32_t)value.toInt();
    else
    if (currentKey == "summary") current->summary = value;
    else
    if (currentKey == "icon") current->icon = iconIndex(val);
    else
    if (currentKey == "precipIntensity") current->precipIntensity = value.toFloat();
    else
    if (currentKey == "precipType") current->precipType = iconIndex(val);
    else
    if (currentKey == "precipProbability") current->precipProbability = (uint8_t)(100 * (value.toFloat()));
    else
    if (currentKey == "temperature") current->temperature = value.toFloat();
    else
    if (currentKey == "humidity") current->humidity = (uint8_t)(100 * (value.toFloat()));
    else
    if (currentKey == "pressure") current->pressure = value.toFloat();
    else
    if (currentKey == "windSpeed") current->windSpeed = value.toFloat();
    else
    if (currentKey == "windGust") current->windGust = value.toFloat();
    else
    if (currentKey == "windBearing") current->windBearing = (uint16_t)value.toInt();
    else
    if (currentKey == "cloudCover") current->cloudCover = (uint8_t)(100 * (value.toFloat()));
    //else
    //if (currentKey == "x") current->x = value;
    return;
  }

  // Hourly data collection
  if (currentParent == "hourly") {
    data_set = currentParent; // Save parent object to trigger the hourly array
    hourly->time[0] = 0;
    if (currentKey == "summary") hourly->overallSummary = value;
    //else
    //if (currentKey == "x") hourly->x = value;
    return;
  }

  // Daily data collection
  if (currentParent == "daily") {
    data_set = currentParent; // Save parent to trigger the daily array
    daily->time[0] = 0;
    if (currentKey == "summary") daily->overallSummary = value;
    //else
    //if (currentKey == "x") daily->x = value;
    return;
  }

  // Collect array data after "data_set" has been set by parent

  // Hourly data[N] array
  if (data_set == "hourly") {
    if (hourly_index >= MAX_HOURS) return;
    if (currentKey == "time") {
      // Only increment after the first entry
      if (hourly->time[0] > 0)
      {
        hourly_index++;
        if (hourly_index >= MAX_HOURS) return;
      }
      hourly->time[hourly_index] = (uint32_t)value.toInt();
    }
    else
    if (currentKey == "summary") hourly->summary[hourly_index] = value;
    else
    if (currentKey == "precipIntensity") hourly->precipIntensity[hourly_index] = value.toFloat();
    else
    if (currentKey == "precipType") hourly->precipType[hourly_index] = iconIndex(val);
    else
    if (currentKey == "precipProbability") hourly->precipProbability[hourly_index] = (uint8_t)(100 * (value.toFloat()));
    else
    if (currentKey == "precipAccumulation") hourly->precipAccumulation[hourly_index] = value.toFloat();
    else
    if (currentKey == "temperature") hourly->temperature[hourly_index] = value.toFloat();
    else
    if (currentKey == "pressure") hourly->pressure[hourly_index] = value.toFloat();
    else
    if (currentKey == "cloudCover") hourly->cloudCover[hourly_index] = (uint8_t)(100 * (value.toFloat()));
    //else
    //if (currentKey == "x") hourly->x[hourly_index] = value;
    return;
  }


  // Daily data[N] array
  if (data_set == "daily") {
    if (daily_index >= MAX_DAYS) return;
    if (currentKey == "time") {
      // Only increment after the first entry
      if (daily->time[0] > 0)
      {
        daily_index++;
        if (daily_index >= MAX_DAYS) return;
      }
      daily->time[daily_index] = (uint32_t)value.toInt();
    }
    else
    if (currentKey == "summary") daily->summary[daily_index] = value;
    else
    if (currentKey == "icon") daily->icon[daily_index] = iconIndex(val);
    else
    if (currentKey == "sunriseTime") daily->sunriseTime[daily_index] = (uint32_t)value.toInt();
    else
    if (currentKey == "sunsetTime") daily->sunsetTime[daily_index] = (uint32_t)value.toInt();
    else
    if (currentKey == "moonPhase") daily->moonPhase[daily_index] = (uint8_t)(100 * (value.toFloat()));
    else
    if (currentKey == "precipIntensity") daily->precipIntensity[daily_index] = value.toFloat();
    else
    if (currentKey == "precipProbability") daily->precipProbability[daily_index] = (uint8_t)(100 * (value.toFloat()));
    else
    if (currentKey == "precipType") daily->precipType[daily_index] = iconIndex(val);
    else
    if (currentKey == "precipAccumulation") daily->precipAccumulation[daily_index] = value.toFloat();
    else
    if (currentKey == "temperatureHigh") daily->temperatureHigh[daily_index] = value.toFloat();
    else
    if (currentKey == "temperatureLow") daily->temperatureLow[daily_index] = value.toFloat();
    else
    if (currentKey == "humidity") daily->humidity[daily_index] = (uint8_t)(100 * (value.toFloat()));
    else
    if (currentKey == "pressure") daily->pressure[daily_index] = value.toFloat();
    else
    if (currentKey == "windSpeed") daily->windSpeed[daily_index] = value.toFloat();
    else
    if (currentKey == "windGust") daily->windGust[daily_index] = value.toFloat();
    else
    if (currentKey == "windBearing") daily->windBearing[daily_index] = (uint16_t)value.toInt();
    else
    if (currentKey == "cloudCover") daily->cloudCover[daily_index] = (uint8_t)(100 * (value.toFloat()));
    //else
    //if (currentKey == "x") daily->x[daily_index] = value;
    //return;
  }

}


#else  // MINIMISE_DATA_POINTS: Collect full data point set if this IS defined <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
       // data points reduced for TFT_eSPI example to reduce memory requirements

/***************************************************************************************
** Function name:           value (partial data set)
** Description:             Stores the parsed data in the structures for sketch access
***************************************************************************************/
void DS_Weather::value(const char *val) {

   String value = val;

  // Start of JSON
  //if (currentParent == "") {
  //  if (currentKey == "timezone") current->timezone = value;
  //}

  // Current forecast - no array index
  if (currentParent == "currently") {
    data_set = "currently";
    if (currentKey == "time") current->time = (uint32_t)value.toInt();
    else
    if (currentKey == "summary") current->summary = value;
    else
    if (currentKey == "icon") current->icon = iconIndex(val);
    //else
    //if (currentKey == "precipIntensity") current->precipIntensity = value.toFloat();
    //else
    //if (currentKey == "precipType") current->precipType = iconIndex(val);
    //else
    //if (currentKey == "precipProbability") current->precipProbability = (uint8_t)(100 * (value.toFloat()));
    else
    if (currentKey == "temperature") current->temperature = value.toFloat();
    else
    if (currentKey == "humidity") current->humidity = (uint8_t)(100 * (value.toFloat()));
    else
    if (currentKey == "pressure") current->pressure = value.toFloat();
    else
    if (currentKey == "windSpeed") current->windSpeed = value.toFloat();
    //else
    //if (currentKey == "windGust") current->windGust = value.toFloat();
    else
    if (currentKey == "windBearing") current->windBearing = (uint16_t)value.toInt();
    else
    if (currentKey == "cloudCover") current->cloudCover = (uint8_t)(100 * (value.toFloat()));
    //else
    //if (currentKey == "x") current->x = value;
    return;
  }

#if !defined (MINIMISE_DATA_POINTS)
  // Hourly data collection
  if (currentParent == "hourly") {
    data_set = currentParent; // Save parent to trigger the hourly array
    hourly->time[0] = 0;
    if (currentKey == "summary") hourly->overallSummary = value;
    //else
    //if (currentKey == "x") hourly->x = value;
    return;
  }
#endif

  // Daily data collection
  if (currentParent == "daily") {
    data_set = currentParent; // Save parent to trigger the daily array
    daily->time[0] = 0;
    if (currentKey == "summary") daily->overallSummary = value;
    //else
    //if (currentKey == "x") daily->x = value;
    return;
  }

  // Collect array data after "data_set" has been set by parent
#if !defined (MINIMISE_DATA_POINTS)
  // Hourly data[N] array
  if (data_set == "hourly") {
    if (hourly_index >= MAX_HOURS) return;
    if (currentKey == "time") {
      // Only increment after the first entry
      if (hourly->time[0] > 0)
      {
        hourly_index++;
        if (hourly_index >= MAX_HOURS) return;
      }
      hourly->time[hourly_index] = (uint32_t)value.toInt();
    }
    else
    if (currentKey == "summary") hourly->summary[hourly_index] = value;
    else
    if (currentKey == "precipIntensity") hourly->precipIntensity[hourly_index] = value.toFloat();
    else
    if (currentKey == "precipType") hourly->precipType[hourly_index] = iconIndex(val);
    else
    if (currentKey == "precipProbability") hourly->precipProbability[hourly_index] = (uint8_t)(100 * (value.toFloat()));
    else
    if (currentKey == "precipAccumulation") hourly->precipAccumulation[hourly_index] = value.toFloat();
    else
    if (currentKey == "temperature") hourly->temperature[hourly_index] = value.toFloat();
    else
    if (currentKey == "pressure") hourly->pressure[hourly_index] = value.toFloat();
    else
    if (currentKey == "cloudCover") hourly->cloudCover[hourly_index] = (uint8_t)(100 * (value.toFloat()));
    //else
    //if (currentKey == "x") hourly->x[hourly_index] = value;
    return;
  }
#endif

  // Daily data[N] array
  if (data_set == "daily") {
    if (daily_index >= MAX_DAYS) return;
    if (currentKey == "time") {
      // Only increment after the first entry
      if (daily->time[0] > 0)
      {
        daily_index++;
        if (daily_index >= MAX_DAYS) return;
      }
      daily->time[daily_index] = (uint32_t)value.toInt();
    }
    else
    if (currentKey == "summary") daily->summary[daily_index] = value;
    else
    if (currentKey == "icon") daily->icon[daily_index] = iconIndex(val);
    else
    if (currentKey == "sunriseTime") daily->sunriseTime[daily_index] = (uint32_t)value.toInt();
    else
    if (currentKey == "sunsetTime") daily->sunsetTime[daily_index] = (uint32_t)value.toInt();
    else
    if (currentKey == "moonPhase") daily->moonPhase[daily_index] = (uint8_t)(100 * (value.toFloat()));
    //else
    //if (currentKey == "precipIntensity") daily->precipIntensity[daily_index] = value.toFloat();
    //else
    //if (currentKey == "precipProbability") daily->precipProbability[daily_index] = (uint8_t)(100 * (value.toFloat()));
    //else
    //if (currentKey == "precipType") daily->precipType[daily_index] = iconIndex(val);
    //else
    //if (currentKey == "precipAccumulation") daily->precipAccumulation[daily_index] = value.toFloat();
    else
    if (currentKey == "temperatureHigh") daily->temperatureHigh[daily_index] = value.toFloat();
    else
    if (currentKey == "temperatureLow") daily->temperatureLow[daily_index] = value.toFloat();
    //else
    //if (currentKey == "humidity") daily->humidity[daily_index] = (uint8_t)(100 * (value.toFloat()));
    //else
    //if (currentKey == "pressure") daily->pressure[daily_index] = value.toFloat();
    //else
    //if (currentKey == "windSpeed") daily->windSpeed[daily_index] = value.toFloat();
    //else
    //if (currentKey == "windGust") daily->windGust[daily_index] = value.toFloat();
    //else
    //if (currentKey == "windBearing") daily->windBearing[daily_index] = (uint16_t)value.toInt();
    //else
    //if (currentKey == "cloudCover") daily->cloudCover[daily_index] = (uint8_t)(100 * (value.toFloat()));
    //else
    //if (currentKey == "x") daily->x[daily_index] = value;
    //return;
  }

}
  
#endif // MINIMISE_DATA_POINTS