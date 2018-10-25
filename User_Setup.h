
// Configuration settings for DarkSkyWeather library

//#define SHOW_HEADER // Debug only - for checking response header via serial message
//#define SHOW_JSON   // Debug only - simple serial output formatting of whole JSON message

//#define AXTLS       // For ESP8266 only: use older axTLS secure client instead of BearSSL
//#define SECURE_SSL  // For ESP8266 only: use SHA1 fingerprint with BearSSL

// >>>>>>>>>>>>  NOTE: MINUTELY FORCAST DISABLED AT THE MOMENT IN THIS LIBRARY <<<<<<<<<<<<<<

// These parameters set the data point count stored in program memory, not the datapoint
// count sent by the Dark Sky server. So they determine the memory used during collection
// of the data points.

#define MAX_MINUTES 60 // Maximum "minutely" forecast period, can be up 1 to 60

#define MAX_HOURS 24   // Maximum "hourly" forecast period, can be up 1 to 48
//                     // Hourly forecast not used by TFT_eSPI examples

#define MAX_DAYS 8     // Maximum "daily" forecast periods can be 1 to 8 (Today + 7 days = 8 maximum)
                       // TFT_eSPI examples require this must be 5 (today + 4 forecast days)

// Note: If MINIMISE_DATA_POINTS is defined a compile error "no member named..."
// will occur in "DarkSkyWeather_Test" example since data points will be missing!




// ###############################################################################
// DO NOT tinker below, this is bad configuration checking that halps stop crashes
// ###############################################################################

// Check and correct bad setting
#if (MAX_MINUTES > 60) || (MAX_MINUTES < 1)
  #undef  MAX_MINUTES
  #define MAX_MINUTES 60
#endif

// Check and correct bad setting
#if (MAX_NOURS > 24) || (MAX_HOURS < 1)
  #undef  MAX_HOURS
  #define MAX_HOURS 24
#endif

// Check and correct bad setting
#if (MAX_DAYS > 8) || (MAX_DAYS < 1)
  #undef  MAX_DAYS
  #define MAX_DAYS 8
#endif