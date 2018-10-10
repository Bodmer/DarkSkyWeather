// Client library for the Dark Sky weather datapoint server
// https://darksky.net/dev

// Created by Bodmer 24/9/2018
// This is a beta test version and is subject to change!

// See license.txt in root folder of library


#ifndef DarkSkyWeather_h
#define DarkSkyWeather_h

/***************************************************************************************
** Description:   JSON streaming interface class
***************************************************************************************/
class DSWrequest: public JsonListener {

  public:
    DSWrequest() : JsonListener() { }

    // Virtual because JSON parser will call them
    virtual void key(String key);

    virtual void startDocument();
    virtual void endDocument();

    virtual void startObject();
    virtual void endObject();

    virtual void startArray();
    virtual void endArray();

    virtual void whitespace(char c);

  protected:

    // Called by user to initiate a GET request and parse it
    bool   parseRequest(String url);
    String currentParent;
    String currentKey;
};

/*
    SI units used:

    summary: Any summaries containing temperature or snow accumulation units will have
             their values in degrees Celsius or in centimeters (respectively).
    nearestStormDistance: Kilometers.
    precipIntensity: Millimeters per hour.
    precipIntensityMax: Millimeters per hour.
    precipAccumulation: Centimeters.
    temperature: Degrees Celsius.
    temperatureMin: Degrees Celsius.
    temperatureMax: Degrees Celsius.
    apparentTemperature: Degrees Celsius.
    dewPoint: Degrees Celsius.
    windSpeed: Meters per second.
    windGust: Meters per second.
    pressure: Hectopascals.
    visibility: Kilometers.

    Dark Sky provide a float 0-1, but here Moon phase, precipProbability, cloud cover
    is a percentage value from 0-100
*/
/***************************************************************************************
** Description:   Structure for current weather
***************************************************************************************/
typedef struct DSW_current {

  String   timezone;
  uint32_t time = 0;
  String   summary;
  String   icon;
  float    precipIntensity = 0;
  String   precipType;
  uint8_t  precipProbability = 0;
  float    temperature = 0;
  uint8_t  humidity = 0;
  float    pressure = 0;
  float    windSpeed = 0;
  float    windGust = 0;
  uint16_t windBearing = 0;
  uint8_t  cloudCover = 0;
} DSW_current;

/***************************************************************************************
** Description:   Structure for minutely weather, not supported yet
***************************************************************************************/
#define MAX_MINUTES 60 // Can be up to 60
typedef struct DSW_minutely {

  String   overallSummary;
  String   icon;
  uint32_t time[MAX_MINUTES] = { 0 }; // seems like a waste to store these....
  float    precipIntensity[MAX_MINUTES] = { 0 };
  uint8_t  precipProbability[MAX_MINUTES] = { 0 };

} DSW_minutely;

/***************************************************************************************
** Description:   Structure for hourly weather
***************************************************************************************/
#define MAX_HOURS 12 // Can be up to 48 but may run out of memory for > 12
typedef struct DSW_hourly {

  String   overallSummary;

  String   summary[MAX_HOURS];
  uint32_t time[MAX_HOURS] = { 0 };
  float    precipIntensity[MAX_HOURS] = { 0 };
  String   precipType[MAX_HOURS];
  uint8_t  precipProbability[MAX_HOURS] = { 0 };
  float    precipAccumulation[MAX_HOURS] = { 0 };
  float    temperature[MAX_HOURS] = { 0 };
  float    pressure[MAX_HOURS] = { 0 };
  uint8_t  cloudCover[MAX_HOURS] = { 0 };

} DSW_hourly;

/***************************************************************************************
** Description:   Structure for daily weather
***************************************************************************************/
#define MAX_DAYS 7 // Today + 7 days = 8 maximum
typedef struct DSW_daily {

  String   overallSummary;

  String   summary[MAX_DAYS];
  uint32_t time[MAX_DAYS] = { 0 };
  String   icon[MAX_DAYS];
  uint32_t sunriseTime[MAX_DAYS] = { 0 };
  uint32_t sunsetTime[MAX_DAYS] = { 0 };
  uint8_t  moonPhase[MAX_DAYS] = { 0 };
  float    precipIntensity[MAX_DAYS] = { 0 };
  uint8_t  precipProbability[MAX_DAYS] = { 0 };
  String   precipType[MAX_DAYS];
  float    precipAccumulation[MAX_DAYS] = { 0 };
  float    temperatureHigh[MAX_DAYS] = { 0 };
  float    temperatureLow[MAX_DAYS] = { 0 };
  uint8_t  humidity[MAX_DAYS] = { 0 };
  float    pressure[MAX_DAYS] = { 0 };
  float    windSpeed[MAX_DAYS] = { 0 };
  float    windGust[MAX_DAYS] = { 0 };
  uint16_t windBearing[MAX_DAYS] = { 0 };
  uint8_t  cloudCover[MAX_DAYS] = { 0 };

} DSW_daily;

/***************************************************************************************
** Description:   Forecast client class
***************************************************************************************/
class DSWforecast : public DSWrequest {
  public:
    DSWforecast() { }

    bool    getForecast(DSW_current *current, DSW_hourly *hourly, DSW_daily  *daily,
                        String api_key, String latitude, String longitude,
                        String units, String language);

    virtual void value(String value);

    uint16_t hourly_index;
    uint16_t daily_index;

    DSW_current *current;
    DSW_hourly  *hourly;
    DSW_daily   *daily;
    String      data_set;
};


#endif
