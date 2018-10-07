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
    virtual void key(String key);

    virtual void startDocument();
    virtual void endDocument();

    virtual void startObject();
    virtual void endObject();

    virtual void startArray();
    virtual void endArray();

    virtual void whitespace(char c);

  protected:
    void   parseRequest(String url);
    String currentKey;
    String currentParent;

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

*/
/***************************************************************************************
** Description:   Structure for current weather
***************************************************************************************/
typedef struct DSW_current {

  String timezone;
  uint32_t time;
  String summary;
  String icon;
  float precipIntensity;
  String precipType;
  float temperature;
  float humidity;
  float pressure;
  float windSpeed;
  float windGust;
  uint16_t windBearing;

} DSW_current;

/***************************************************************************************
** Description:   Structure for minutely weather, not supported yet
***************************************************************************************/
#define MAX_MINUTES 60 // Can be up to 60
typedef struct DSW_minutely {

  String overallSummary;
  String icon;
  uint32_t time[MAX_MINUTES] = { 0 }; // seems like a waste to store these....
  float precipIntensity[MAX_MINUTES];
  float precipProbability[MAX_MINUTES];

} DSW_minutely;

/***************************************************************************************
** Description:   Structure for hourly weather
***************************************************************************************/
#define MAX_HOURS 24 // Can be up to 48
typedef struct DSW_hourly {

  String overallSummary;

  String summary[MAX_HOURS];
  uint32_t time[MAX_HOURS] = { 0 };
  float precipIntensity[MAX_HOURS];
  String precipType[MAX_HOURS];
  float temperature[MAX_HOURS];
  float pressure[MAX_HOURS];

} DSW_hourly;

/***************************************************************************************
** Description:   Structure for daily weather
***************************************************************************************/
#define MAX_DAYS 8 // Today + 7 days = 8 maximum
typedef struct DSW_daily {

  String overallSummary;

  String summary[MAX_DAYS];
  uint32_t time[MAX_DAYS] = { 0 };
  String icon[MAX_DAYS];
  uint32_t sunriseTime[MAX_DAYS] = { 0 };
  uint32_t sunsetTime[MAX_DAYS] = { 0 };
  uint8_t moonPhase[MAX_DAYS] = { 0 };
  float precipIntensity[MAX_DAYS];
  float precipProbability[MAX_DAYS];
  String precipType[MAX_DAYS];
  float temperatureHigh[MAX_DAYS];
  float temperatureLow[MAX_DAYS];
  float humidity[MAX_DAYS];
  float pressure[MAX_DAYS];
  float windSpeed[MAX_DAYS];
  float windGust[MAX_DAYS];
  uint16_t windBearing[MAX_DAYS];
  float cloudCover[MAX_DAYS];

} DSW_daily;

/***************************************************************************************
** Description:   Forecast client class
***************************************************************************************/
class DSWforecast : public DSWrequest {
  public:
    DSWforecast() { }

    void    getForecast(DSW_current *current, DSW_hourly *hourly, DSW_daily  *daily,
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
