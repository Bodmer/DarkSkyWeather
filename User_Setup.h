
// Configuration settings for DarkSkyWeather library

// >>>>>>>>>>>>  NOTE: MINUTELY FORCAST DISABLED AT THE MOMENT IN THIS LIBRARY <<<<<<<<<<<<<<

// These parameters set the data point count stored in program memory (not the datapoint
// count sent by the Dark Sky server). So they determine the memory used during collection
// of the data points.

#define MAX_MINUTES 60 // Maximum "minutely" forecast period, can be up 1 to 60

#define MAX_HOURS 24   // Maximum "hourly" forecast period, can be up 1 to 48
//                     // Hourly forecast not used by TFT_eSPI examples

#define MAX_DAYS 8     // Maximum "daily" forecast periods can be 1 to 8 (Today + 7 days = 8 maximum)
                       // TFT_eSPI example requires this to be >= 5 (today + 4 forecast days)

// #define MINIMISE_DATA_POINTS // option to minimise stored values for TFT_eSPI_Weather example

// Note: If MINIMISE_DATA_POINTS is defined and the "DarkSkyWeather_Test" example
// compiled then a compile error "no member named..." will occur in  since data points
// will be missing!  Unfortnately compile time options for a library cannot be set in
// when using the Arduino IDE.

//#define AXTLS       // For ESP8266 only: use older axTLS secure client instead of BearSSL
//#define SECURE_SSL  // For ESP8266 only: use SHA1 fingerprint with BearSSL


//#define SHOW_HEADER // Debug only - for checking response header via serial message
//#define SHOW_JSON   // Debug only - simple serial output formatting of whole JSON message
//#define SHOW_CALLBACK // Debug only to show the decode tree

// ###############################################################################
// DO NOT tinker below, this is configuration checking that helps stop crashes:
// ###############################################################################

// Check and correct bad setting
#if (MAX_MINUTES > 60) || (MAX_MINUTES < 1)
  #undef  MAX_MINUTES
  #define MAX_MINUTES 60 // Ignore compiler warning!
#endif

// Check and correct bad setting
#if (MAX_NOURS > 24) || (MAX_HOURS < 1)
  #undef  MAX_HOURS
  #define MAX_HOURS 24 // Ignore compiler warning!
#endif

// Check and correct bad setting
#if (MAX_DAYS > 8) || (MAX_DAYS < 1)
  #undef  MAX_DAYS
  #define MAX_DAYS 8  // Ignore compiler warning!
#endif