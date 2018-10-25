// Client library for the Dark Sky weather datapoint server
// https://darksky.net/dev

// The API server uses https, so a client library with secure support is needed

// Created by Bodmer 24/9/2018
// This is a beta test version and is subject to change!

// See license.txt in root folder of library

#ifndef DarkSkyWeather_h
#define DarkSkyWeather_h

#include "User_Setup.h"
#include "Data_Point_Set.h"


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
