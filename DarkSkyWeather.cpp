// Client library for the Dark Sky weather datapoint server
// https://darksky.net/dev

// Created by Bodmer 24/9/2018
// This is a beta test version and is subject to change!

// See license.txt in root folder of library

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <JsonListener.h>
#include <JsonStreamingParser.h>

#include "DarkSkyWeather.h"

/***************************************************************************************
** Function name:           parseRequest
** Description:             Fetches the JSON message and feeds to the parser
***************************************************************************************/
//#define SHOW_JSON // Debug only - simple formatting of whole JSON message
void DSWrequest::parseRequest(String url) {

  uint32_t dt = millis();

  // SHA1 certificate fingerprint
  const char* fingerprint = "EB:C2:67:D1:B1:C6:77:90:51:C1:4A:0A:BA:83:E1:F0:6D:73:DD:B8";

  // Only tested with axTLS but BearSSL should work
  WiFiClientSecure client;
  //BearSSL::WiFiClientSecure client;
  
  JsonStreamingParser parser;
  parser.setListener(this);

  const char*  host = "api.darksky.net";

  if (!client.connect(host, 443)) {
      Serial.println("Connection failed.");
      return;
  }

  if (client.verify(fingerprint, host)) {
      Serial.println("Certificate OK");
  } else {
      Serial.println("Bad certificate");
      client.stop();
      return;
  }

  bool isBody = false;
  char c;
  int ccount = 0;

  // Send GET request
  Serial.println("Sending GET request to api.darksky.net...");
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("Headers stripped");
      break;
    }
  }

  // Parse the JSON data
  while(client.connected()) {
    while(client.available() > 0) {
      c = client.read();
      if (c == '{' || c == '[') {
        ccount = 0;
        isBody = true;
      }
      if (isBody) {
        parser.parse(c);
      }
#ifdef SHOW_JSON
      if (c == '{' || c == '[' || c == '}' || c == ']') Serial.println();
      Serial.print(c); if (ccount++ > 100 && c == ',') {ccount = 0; Serial.println();}
#endif
    }
  }
  Serial.println("");
  Serial.print("Done in "); Serial.print(millis()-dt); Serial.println(" ms\n");

  parser.reset();

  client.stop();
}

/***************************************************************************************
** Function name:           key etc
** Description:             These are called when parseing the JSON message
***************************************************************************************/
void DSWrequest::key(String key) {
  //Serial.print(">>> Key >>>" + key);
  currentKey = key;
}

void DSWrequest::startDocument() {
  //Serial.print(">>> Start document >>>");
  currentParent = currentKey = "";
}

void DSWrequest::endDocument() {
  //Serial.print("<<< End document <<<");
}

void DSWrequest::startObject() {
  //Serial.print(">>> Start object >>>");
  currentParent = currentKey;
}

void DSWrequest::endObject() {
  //Serial.print("<<< End object <<<");
  currentParent = "";
}

void DSWrequest::startArray() {
  //Serial.print(">>> Start array >>>");
}

void DSWrequest::endArray() {
  //Serial.print("<<< End array <<<");
}

void DSWrequest::whitespace(char c) {
}

/***************************************************************************************
** Function name:           getForecast
** Description:             Setup the weather forecast request from darksky.net
***************************************************************************************/
// The structures etc are created by the sketch and passed to this function.
void DSWforecast::getForecast(DSW_current *current, DSW_hourly *hourly, DSW_daily  *daily, String api_key, String latitude, String longitude, String units, String language) {

  data_set = "";
  hourly_index = 0;
  daily_index = 0;

  // Local copies of structure pointers
  this->current = current;
  this->hourly  = hourly;
  this->daily   = daily;

  // Exclude some info to shorten JSON message
  String exclude = "";
  //exclude += "hourly,";   // summary, then weather every hour for 48 hours
  exclude += "minutely,"; // summary, then rain predictions every minute for next hour, gives huge message response!
  //exclude += "daily,";    // summary, then daily detailed weather for one week (7 days)
  exclude += "alerts,";   // special warnings, typically none
  exclude += "flags";     // misc info

  String url = "https://api.darksky.net/forecast/" + api_key + "/" + latitude + "," + longitude + "?exclude=" + exclude + "&units=" + units;

  // Send GET request and feed the parser
  parseRequest(url);

  // Null out pointers to prevent crashes
  this->current = nullptr;
  this->hourly  = nullptr;
  this->daily   = nullptr;
}

/***************************************************************************************
** Function name:           updateForecast
** Description:             Store the parse data in the structures for sketch access
***************************************************************************************/
void DSWforecast::value(String value) {

  // Start of JSON
  if (currentParent == "") {
    if (currentKey == "timezone") current->timezone = value;
  }

  // Current forecast - no array index
  if (currentParent == "currently") {
    data_set = "currently";
    if (currentKey == "time") current->time = value.toInt();
    else
    if (currentKey == "summary") current->summary = value;
    else
    if (currentKey == "icon") current->icon = value;
    else
    if (currentKey == "precipIntensity") current->precipIntensity = value.toFloat();
    else
    if (currentKey == "precipType") current->precipType = value;
    else
    if (currentKey == "temperature") current->temperature = value.toFloat();
    else
    if (currentKey == "humidity") current->humidity = value.toFloat();
    else
    if (currentKey == "pressure") current->pressure = value.toFloat();
    else
    if (currentKey == "windSpeed") current->windSpeed = value.toFloat();
    else
    if (currentKey == "windGust") current->windGust = value.toFloat();
    else
    if (currentKey == "windBearing") current->windBearing = value.toInt();
    //else
    //if (currentKey == "x") current->x = value;
  }

  // Hourly data collection
  if (currentParent == "hourly") {
    data_set = currentParent; // Save parent to trigger the hourly array
    hourly->time[0] = 0;
    if (currentKey == "summary") hourly->overallSummary = value;
    //else
    //if (currentKey == "x") hourly->x = value;
  }

  // Daily data collection
  if (currentParent == "daily") {
    data_set = currentParent; // Save parent to trigger the daily array
    daily->time[0] = 0;
    if (currentKey == "summary") daily->overallSummary = value;
    //else
    //if (currentKey == "x") daily->x = value;
  }

  // Collect array data after "data_set" has been set by parent

  // Hourly data[N] array
  if (data_set == "hourly") {
    if (currentKey == "time" && hourly_index < MAX_HOURS - 1) {
      // Only increment after the first entry
      if (hourly->time[0] > 0) hourly_index++;
      hourly->time[hourly_index] = value.toInt();
    }
    if (currentKey == "summary") hourly->summary[hourly_index] = value;
    else
    if (currentKey == "precipIntensity") hourly->precipIntensity[hourly_index] = value.toFloat();
    else
    if (currentKey == "precipType") hourly->precipType[hourly_index] = value;
    else
    if (currentKey == "temperature") hourly->temperature[hourly_index] = value.toFloat();
    else
    if (currentKey == "pressure") hourly->pressure[hourly_index] = value.toFloat();
    //else
    //if (currentKey == "x") hourly->x[hourly_index] = value;
  }


  // Daily data[N] array
  if (data_set == "daily") {
    if (currentKey == "time"  && daily_index < MAX_DAYS - 1) {
      // Only increment after the first entry
      if (daily->time[0] > 0) daily_index++;
      daily->time[daily_index] = value.toInt();
    }
    if (currentKey == "summary") daily->summary[daily_index] = value;
    else
    if (currentKey == "time") daily->time[daily_index] = value.toInt();
    else
    if (currentKey == "icon") daily->icon[daily_index] = value;
    else
    if (currentKey == "sunriseTime") daily->sunriseTime[daily_index] = value.toInt();
    else
    if (currentKey == "sunsetTime") daily->sunsetTime[daily_index] = value.toInt();
    else
    if (currentKey == "moonPhase") daily->moonPhase[daily_index] = 100 * (value.toFloat());
    else
    if (currentKey == "precipIntensity") daily->precipIntensity[daily_index] = value.toFloat();
    else
    if (currentKey == "precipProbability") daily->precipProbability[daily_index] = value.toFloat();
    else
    if (currentKey == "precipType") daily->precipType[daily_index] = value;
    else
    if (currentKey == "temperatureHigh") daily->temperatureHigh[daily_index] = value.toFloat();
    else
    if (currentKey == "temperatureLow") daily->temperatureLow[daily_index] = value.toFloat();
    else
    if (currentKey == "humidity") daily->humidity[daily_index] = value.toFloat();
    else
    if (currentKey == "pressure") daily->pressure[daily_index] = value.toFloat();
    else
    if (currentKey == "windSpeed") daily->windSpeed[daily_index] = value.toFloat();
    else
    if (currentKey == "windGust") daily->windGust[daily_index] = value.toFloat();
    else
    if (currentKey == "windBearing") daily->windBearing[daily_index] = value.toInt();
    else
    if (currentKey == "cloudCover") daily->cloudCover[daily_index] = value.toFloat();
    //else
    //if (currentKey == "x") daily->x[daily_index] = value;
  }

}
