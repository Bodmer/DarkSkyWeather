// The structures below are the repository for the data values extracted from the
// JSON message. The structures are popolated with the extracted data by the "value()"
// member function in the main DarkSkyWeather.cpp file.

// Some structs contain arrays so watch out for memory consumption. With DarkSky you can
// request a subset of the full weather report but this library grabs all values with
// one GET request to avoid exceeding the 1000 free request count per day (count reset
// at 00:00 UTC). 1000 per day means ~40 per hour. As the weather forcast changes slowly
// the example requests the forecast every 15 minutes, so adapting to reduce memory
// by requesting current, daily, hourly etc forescasts individually can be done.

// The content is zero or "" when first created.

#ifndef MINIMISE_DATA_POINTS // Full set of values populated if not defined
/***************************************************************************************
** Description:   Structure for current weather
***************************************************************************************/
typedef struct DSW_current {

  String   timezone;
  uint32_t time = 0;
  String   summary;
  uint8_t  icon = 0;
  float    precipIntensity = 0;
  uint8_t  precipType = NO_VALUE;
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
typedef struct DSW_minutely {

  String   overallSummary;
  uint8_t  icon = 0;
  uint32_t time[MAX_MINUTES] = { 0 }; // maybe store as a minute start time + minute
                                      // count uint8_t as 0-59
  float    precipIntensity[MAX_MINUTES] = { 0 }; // Q: what is the value range for this?
  uint8_t  precipProbability[MAX_MINUTES] = { 0 };// float 0.0 to 1.0 -> 0-100%

} DSW_minutely;

/***************************************************************************************
** Description:   Structure for hourly weather
***************************************************************************************/
typedef struct DSW_hourly {

  String   overallSummary;

  String   summary[MAX_HOURS];
  uint32_t time[MAX_HOURS] = { 0 };
  float    precipIntensity[MAX_HOURS] = { 0 };
  uint8_t  precipType[MAX_HOURS] = { NO_VALUE };
  uint8_t  precipProbability[MAX_HOURS] = { 0 };
  float    precipAccumulation[MAX_HOURS] = { 0 };
  float    temperature[MAX_HOURS] = { 0 };
  float    pressure[MAX_HOURS] = { 0 };
  uint8_t  cloudCover[MAX_HOURS] = { 0 };

} DSW_hourly;

/***************************************************************************************
** Description:   Structure for daily weather
***************************************************************************************/
typedef struct DSW_daily {

  String   overallSummary;

  String   summary[MAX_DAYS];
  uint32_t time[MAX_DAYS] = { 0 };
  uint8_t  icon[MAX_DAYS] = { 0 };
  uint32_t sunriseTime[MAX_DAYS] = { 0 };
  uint32_t sunsetTime[MAX_DAYS] = { 0 };
  uint8_t  moonPhase[MAX_DAYS] = { 0 };
  float    precipIntensity[MAX_DAYS] = { 0 };
  uint8_t  precipProbability[MAX_DAYS] = { 0 };
  uint8_t  precipType[MAX_DAYS] = { NO_VALUE };
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


#else // Collect minimal set of data points for TFT_eSPI examples to reduce RAM needs

/***************************************************************************************
** Description:   Structure for current weather
***************************************************************************************/
typedef struct TFT_current {

  //String   timezone;
  uint32_t time = 0;
  String   summary;
  uint8_t  icon = 0;
  //float    precipIntensity = 0;
  //uint8_t  precipType = NO_VALUE;
  //uint8_t  precipProbability = 0;
  float    temperature = 0;
  uint8_t  humidity = 0;
  float    pressure = 0;
  float    windSpeed = 0;
  //float    windGust = 0;
  uint16_t windBearing = 0;
  uint8_t  cloudCover = 0;
} TFT_current;

/***************************************************************************************
** Description:   Structure for minutely weather, not supported yet
***************************************************************************************/
#define MAX_MINUTES 60 // Can be up to 60 - not used by TFT_eSPI
typedef struct TFT_minutely {

  //String   overallSummary;
  //uint8_t  icon = 0;
  //uint32_t time[MAX_MINUTES] = { 0 }; // seems like a waste to store these....
  //float    precipIntensity[MAX_MINUTES] = { 0 };
  //uint8_t  precipProbability[MAX_MINUTES] = { 0 };

} TFT_minutely;

/***************************************************************************************
** Description:   Structure for hourly weather
***************************************************************************************/
#define MAX_HOURS 24 // Can be up to 48 - not used by TFT_eSPI
typedef struct TFT_hourly {

  //String   overallSummary;

  //String   summary[MAX_HOURS];
  //uint32_t time[MAX_HOURS] = { 0 };
  //float    precipIntensity[MAX_HOURS] = { 0 };
  //uint8_t  precipType[MAX_HOURS] = { NO_VALUE };
  //uint8_t  precipProbability[MAX_HOURS] = { 0 };
  //float    precipAccumulation[MAX_HOURS] = { 0 };
  //float    temperature[MAX_HOURS] = { 0 };
  //float    pressure[MAX_HOURS] = { 0 };
  //uint8_t  cloudCover[MAX_HOURS] = { 0 };

} TFT_hourly;

/***************************************************************************************
** Description:   Structure for daily weather
***************************************************************************************/
#define MAX_DAYS 5 // Today + 7 days = 8 maximum, make it 5 for TFT_eSPI example
typedef struct TFT_daily {

  String   overallSummary;

  String   summary[MAX_DAYS];
  uint32_t time[MAX_DAYS] = { 0 };
  uint8_t  icon[MAX_DAYS] = { 0 };
  uint32_t sunriseTime[MAX_DAYS] = { 0 };
  uint32_t sunsetTime[MAX_DAYS] = { 0 };
  uint8_t  moonPhase[MAX_DAYS] = { 0 };
  //float    precipIntensity[MAX_DAYS] = { 0 };
  //uint8_t  precipProbability[MAX_DAYS] = { 0 };
  //uint8_t  precipType[MAX_DAYS] = { NO_VALUE };
  //float    precipAccumulation[MAX_DAYS] = { 0 };
  float    temperatureHigh[MAX_DAYS] = { 0 };
  float    temperatureLow[MAX_DAYS] = { 0 };
  //uint8_t  humidity[MAX_DAYS] = { 0 };
  //float    pressure[MAX_DAYS] = { 0 };
  //float    windSpeed[MAX_DAYS] = { 0 };
  //float    windGust[MAX_DAYS] = { 0 };
  //uint16_t windBearing[MAX_DAYS] = { 0 };
  //uint8_t  cloudCover[MAX_DAYS] = { 0 };

} TFT_daily;

#endif