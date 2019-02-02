// Client library for the Dark Sky weather datapoint server
// https://darksky.net/dev

// The API server uses https, so a client library with secure support is needed

// Created by Bodmer 24/9/2018
// This is a beta test version and is subject to change!

// See license.txt in root folder of library
// iconList[] ndex default

#define MAX_ICON_INDEX 11 // Maximum for weather icon index
#define ICON_RAIN 1       // Index for the rain icon bitmap (bmp file)
#define NO_VALUE 11       // for precipType default (none)

#ifndef DarkSkyWeather_h
#define DarkSkyWeather_h

#include "User_Setup.h"
#include "Data_Point_Set.h"


/***************************************************************************************
** Description:   JSON interface class
***************************************************************************************/
class DS_Weather: public JsonListener {

  public:
    // Sketch calls this forecast request, it returns true if no parse errors encountered
    bool getForecast(DSW_current *current, DSW_hourly *hourly, DSW_daily  *daily,
                     String api_key, String latitude, String longitude,
                     String units, String language);

    // Called by library (or user sketch), sends a GET request to a https (secure) url
    bool parseRequest(String url); // and parses response, returns true if no parse errors

    // Convert the icon index to a name e.g. "partly-cloudy"
    const char* iconName(uint8_t index);

  private: // Streaming parser callback functions, allow tracking and decisions

    void startDocument(); // JSON document has started, typically starts once
                          // Initialises varaibles used, e.g. sets objectLayer = 0
                          // and arrayIndex =0
    void endDocument();   // JSON document has ended, typically ends once

    void startObject();   // Called every time and Object start detected
                          // may be called multiple times as object layers entered
                          // Used to increment objectLayer
    void endObject();     // Called every time an object ends
                          // Used to decrement objectLayer and zero arrayIndex


    void startArray();    // An array of name:value pairs entered
    void endArray();      // Array member ended, increments arrayIndex

    void key(const char *key);            // The current "object" or "name for a name:value pair"
    void value(const char *value);        // String value from name:value pair e.g. "1.23" or "rain"

    void whitespace(char c);              // Whitespace character in JSON - not used

    void error( const char *message );    // Error message is sent to serial port


    uint8_t iconIndex(const char *val);   // Convert the icon name e.g. "partly-cloudy" to an array
                                          // index to save memory, range 0 to MAX_ICON_INDEX

  private: // Variables used internal to library

    uint16_t hourly_index; // index into the DSW_hourly structure's data arrays
    uint16_t daily_index;  // index into the DSW_daily structure's data arrays

    // The value storage structures are created and deleted by the sketch and
    // a pointer passed via the library getForecast() call the value() function
    // is then used to populate the structs with values
    DSW_current *current;   // pointer provided by sketch to the DSW_current struct
    DSW_hourly  *hourly;    // pointer provided by sketch to the DSW_hourly struct
    DSW_daily   *daily;     // pointer provided by sketch to the DSW_daily struct

    String      valuePath;  // object (i.e. sequential key) path (like a "file path")
                            // taken to the name:value pair in the form "hourly/data"
                            // so values can be pulled from the correct array.
                            // Needed since different objects contain "data" arrays.

    String data_set;        // A copy of the last object name at the head of an array
                            // short equivalent to path.

    bool     parseOK;       // true if the parse been completed
                            // (does not mean data values gathered are good!)

    String   currentParent; // Current object e.g. "daily"
    uint16_t objectLevel;   // Object level, increments for new object, decrements at end
    String   currentKey;    // Name key of the name:value pair e.g "temperature"
    String   arrayPath;     // Path to name:value pair e.g.  "daily/data"
    uint16_t arrayIndex;    // Array index e.g. 5 for day 5 forecast, qualify with arrayPath

    // Lookup table to convert  an array index to a weather icon bmp filename e.g. rain.bmp

// A partly-cloudy-night means a clear day as noted in issue #7
    const char* iconList[MAX_ICON_INDEX + 1] = {"unknown", "rain", "sleet", "snow", "clear-day",
    "clear-night", "partly-cloudy-day", "partly-cloudy-night", "cloudy", "fog",
    "wind", "none" };

};

/***************************************************************************************
***************************************************************************************/
#endif
