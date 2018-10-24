// Client library for the Dark Sky weather datapoint server
// https://darksky.net/dev

// The API server uses https, so a client library with secure support is needed

// Created by Bodmer 24/9/2018
// This is a beta test version and is subject to change!

// See license.txt in root folder of library

#ifndef DarkSkyWeather_h
#define DarkSkyWeather_h


/***************************************************************************************
** Description:   Structure for current weather
***************************************************************************************/
typedef struct DSW_current {

  //String   timezone;
  uint32_t time = 0;
  String   summary;
  uint8_t  icon;
  //float    precipIntensity = 0;
  //uint8_t  precipType;
  //uint8_t  precipProbability = 0;
  float    temperature = 0;
  uint8_t  humidity = 0;
  float    pressure = 0;
  float    windSpeed = 0;
  //float    windGust = 0;
  uint16_t windBearing = 0;
  uint8_t  cloudCover = 0;
} DSW_current;

/***************************************************************************************
** Description:   Structure for minutely weather, not supported yet
***************************************************************************************/
#define MAX_MINUTES 60 // Can be up to 60
typedef struct DSW_minutely {

  String   overallSummary;
  uint8_t  icon;
  uint32_t time[MAX_MINUTES] = { 0 }; // seems like a waste to store these....
  float    precipIntensity[MAX_MINUTES] = { 0 };
  uint8_t  precipProbability[MAX_MINUTES] = { 0 };

} DSW_minutely;

/***************************************************************************************
** Description:   Structure for hourly weather
***************************************************************************************/
#define MAX_HOURS 24 // Can be up to 48
typedef struct DSW_hourly {

  String   overallSummary;

  String   summary[MAX_HOURS];
  uint32_t time[MAX_HOURS] = { 0 };
  float    precipIntensity[MAX_HOURS] = { 0 };
  uint8_t  precipType[MAX_HOURS];
  uint8_t  precipProbability[MAX_HOURS] = { 0 };
  float    precipAccumulation[MAX_HOURS] = { 0 };
  float    temperature[MAX_HOURS] = { 0 };
  float    pressure[MAX_HOURS] = { 0 };
  uint8_t  cloudCover[MAX_HOURS] = { 0 };

} DSW_hourly;

/***************************************************************************************
** Description:   Structure for daily weather
***************************************************************************************/
#define MAX_DAYS 5 // Today + 7 days = 8 maximum (make it at least 5!)
typedef struct DSW_daily {

  String   overallSummary;

  String   summary[MAX_DAYS];
  uint32_t time[MAX_DAYS] = { 0 };
  uint8_t  icon[MAX_DAYS];
  uint32_t sunriseTime[MAX_DAYS] = { 0 };
  uint32_t sunsetTime[MAX_DAYS] = { 0 };
  uint8_t  moonPhase[MAX_DAYS] = { 0 };
  //float    precipIntensity[MAX_DAYS] = { 0 };
  //uint8_t  precipProbability[MAX_DAYS] = { 0 };
  //uint8_t  precipType[MAX_DAYS];
  //float    precipAccumulation[MAX_DAYS] = { 0 };
  float    temperatureHigh[MAX_DAYS] = { 0 };
  float    temperatureLow[MAX_DAYS] = { 0 };
  //uint8_t  humidity[MAX_DAYS] = { 0 };
  //float    pressure[MAX_DAYS] = { 0 };
  //float    windSpeed[MAX_DAYS] = { 0 };
  //float    windGust[MAX_DAYS] = { 0 };
  //uint16_t windBearing[MAX_DAYS] = { 0 };
  //uint8_t  cloudCover[MAX_DAYS] = { 0 };

} DSW_daily;

/***************************************************************************************
** Description:   JSON streaming interface class
***************************************************************************************/
class DS_Weather: public JsonListener {

  public:

    void startDocument();
    void endDocument();

    void startObject();
    void endObject();

    void startArray();
    void endArray();

    void key(const char *key);
    void value(const char *value);

    void whitespace(char c);

    void error( const char *message );

    bool getForecast(DSW_current *current, DSW_hourly *hourly, DSW_daily  *daily,
                     String api_key, String latitude, String longitude,
                     String units, String language);

    uint8_t iconIndex(const char *val);

    uint16_t hourly_index;
    uint16_t daily_index;

    DSW_current *current;
    DSW_hourly  *hourly;
    DSW_daily   *daily;
    String      data_set;

    #define MAX_ICON_INDEX 10
    #define ICON_RAIN 1
    const char* iconText[11] = {"unknown", "rain", "sleet", "snow", "clear-day",
    "clear-night", "partly-cloudy-day", "partly-cloudy-night", "cloudy", "fog",
    "wind" };

  protected:

    // Called by user to initiate a GET request and parse it
    bool   parseRequest(String url);
    String currentParent;
    String currentKey;

};

/***************************************************************************************
***************************************************************************************/
#endif
