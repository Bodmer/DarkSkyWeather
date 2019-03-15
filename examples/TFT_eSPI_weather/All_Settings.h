//  Use the DarkSkyWeather library: https://github.com/Bodmer/DarkSkyWeather

//  The weather icons and fonts are in the sketch data folder, press Ctrl+K
//  to view.


//                >>>       IMPORTANT TO PREVENT CRASHES      <<<
//>>>>>>  For ESP8266 set SPIFFS to at least 2Mbytes before uploading files  <<<<<<


//                >>>           DON'T FORGET THIS             <<<
//  Upload the fonts and icons to SPIFFS using the "Tools"  "ESP32 Sketch Data Upload"
//  or "ESP8266 Sketch Data Upload" menu option in the IDE.
//  To add this option follow instructions here for the ESP8266:
//  https://github.com/esp8266/arduino-esp8266fs-plugin
//  To add this option follow instructions here for the ESP32:
//  https://github.com/me-no-dev/arduino-esp32fs-plugin

//  Close the IDE and open again to see the new menu option.

//////////////////////////////
// Setttings defined below

#define WIFI_SSID "Your_SSID"
#define PASSWORD "Your_password"

#define TIMEZONE UK // See NTP_Time.h tab for other "Zone references", UK, usMT etc

// Update every 15 minutes, up to 1000 request per day are free (viz average of ~40 per hour)
const int UPDATE_INTERVAL_SECS = 15 * 60UL; // 15 minutes

// Pins for the TFT interface are defined in the User_Config.h file inside the TFT_eSPI library

// For units codes see https://darksky.net/dev/docs, not all supported by library at the moment!
const String units = "si";

// Sign up for an account at Dark Sky, change x's to your API key
const String api_key = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

// For language codes see https://darksky.net/dev/docs
const String language = "en"; // Default language = en = English

// Short day of week abbreviations used in 4 day forecast (change to your language)
const String shortDOW [8] = {"???", "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};

// Change the labels to your language here:
const char sunStr[]        = "Sun";
const char cloudStr[]      = "Cloud";
const char humidityStr[]   = "Humidity";
const String moonPhase [8] = {"New", "Waxing", "1st qtr", "Waxing", "Full", "Waning", "Last qtr", "Waning"};

// Set the forecast longitude and latitude to at least 4 decimal places
const String latitude =  "27.9881"; // 90.0000 to -90.0000 negative for Southern hemisphere
const String longitude = "86.9250"; // 180.000 to -180.000 negative for West

// End of user settings
//////////////////////////////
