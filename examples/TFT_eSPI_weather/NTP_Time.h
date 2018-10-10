//====================================================================================
//                                  Definitions
//====================================================================================
//#define PRINT_TIME // Print time every second to serial port
//====================================================================================
//                                  Libraries
//====================================================================================

// Time library:
// http://www.arduino.cc/playground/Code/Time
// latest may be here: https://www.pjrc.com/teensy/td_libs_Time.html
#include <Time.h>

// Time zone correction library:
// https://github.com/JChristensen/Timezone
#include <Timezone.h>

// Libraries built into IDE
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

//====================================================================================
//                                  Settings
//====================================================================================

// Try to use pool url instead so the server IP address is looked up from those available
// (use a pool server in your own country to improve response time and reliability)
//const char* ntpServerName = "time.nist.gov";
const char* ntpServerName = "pool.ntp.org";

// Try not to use hard-coded IP addresses which might change, you can if you want though...
//IPAddress timeServerIP(129, 6, 15, 30); // time-c.nist.gov NTP server
//IPAddress timeServerIP(24, 56, 178, 140); // wwv.nist.gov NTP server

// Example time zone and DST corrections, see Timezone library documents to see how
// to add more time zones

// Zone reference "UK" United Kingdom (London, Belfast)
TimeChangeRule BST = {"BST", Last, Sun, Mar, 1, 60};        //British Summer (Daylight saving) Time
TimeChangeRule GMT = {"GMT", Last, Sun, Oct, 2, 0};         //Standard Time
Timezone UK(BST, GMT);

// Zone reference "euCET" Central European Time (Frankfurt, Paris)
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Summer Time
TimeChangeRule  CET = {"CET ", Last, Sun, Oct, 3, 60};      //Central European Standard Time
Timezone euCET(CEST, CET);

// Zone reference "ausET" Australia Eastern Time Zone (Sydney, Melbourne)
TimeChangeRule aEDT = {"AEDT", First, Sun, Oct, 2, 660};    //UTC + 11 hours
TimeChangeRule aEST = {"AEST", First, Sun, Apr, 3, 600};    //UTC + 10 hours
Timezone ausET(aEDT, aEST);

// Zone reference "usET US Eastern Time Zone (New York, Detroit)
TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -240};  //Eastern Daylight Time = UTC - 4 hours
TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -300};   //Eastern Standard Time = UTC - 5 hours
Timezone usET(usEDT, usEST);

// Zone reference "usCT" US Central Time Zone (Chicago, Houston)
TimeChangeRule usCDT = {"CDT", Second, dowSunday, Mar, 2, -300};
TimeChangeRule usCST = {"CST", First, dowSunday, Nov, 2, -360};
Timezone usCT(usCDT, usCST);

// Zone reference "usMT" US Mountain Time Zone (Denver, Salt Lake City)
TimeChangeRule usMDT = {"MDT", Second, dowSunday, Mar, 2, -360};
TimeChangeRule usMST = {"MST", First, dowSunday, Nov, 2, -420};
Timezone usMT(usMDT, usMST);

// Zone reference "usAZ" Arizona is US Mountain Time Zone but does not use DST
Timezone usAZ(usMST, usMST);

// Zone reference "usPT" US Pacific Time Zone (Las Vegas, Los Angeles)
TimeChangeRule usPDT = {"PDT", Second, dowSunday, Mar, 2, -420};
TimeChangeRule usPST = {"PST", First, dowSunday, Nov, 2, -480};
Timezone usPT(usPDT, usPST);

IPAddress timeServerIP; // Use server pool

//====================================================================================
//                                  Variables
//====================================================================================
TimeChangeRule *tz1_Code;   // Pointer to the time change rule, use to get the TZ abbrev, e.g. "GMT"
TimeChangeRule *tz2_Code;   // Pointer to anoth time change rule

time_t utc = 0;

bool timeValid = false;

unsigned int localPort = 2390;      // local port to listen for UDP packets

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE ]; //buffer to hold incoming and outgoing packets

uint8_t lastMinute = 0;

uint32_t nextSendTime = 0;
uint32_t newRecvTime = 0;
uint32_t lastRecvTime = 0;

uint32_t newTickTime = 0;
uint32_t lastTickTime = 0;

bool rebooted = 1;

uint32_t no_packet_count = 0;


//====================================================================================
//                                    Update Time
//====================================================================================

void requestTime(void);
void displayTime(void);
void printTime(time_t zone, char *tzCode);
void decodeNTP(void);
unsigned long sendNTPpacket(IPAddress& address);

void requestTime(void)
{
  // Don't send too often so we don't trigger Denial of Service
  if (nextSendTime < millis()) {
    // Get a random server from the pool
    WiFi.hostByName(ntpServerName, timeServerIP);
    nextSendTime = millis() + 5000;
    sendNTPpacket(timeServerIP); // send an NTP packet to a time server
    decodeNTP();
  }
}

//====================================================================================
//                                  Supporting functions
//====================================================================================
void printTime(time_t zone, char *tzCode)
{
   String dateString = dayStr(weekday(zone));
   dateString += " ";
   dateString += day(zone);
   if (day(zone) == 1 || day(zone) == 21 || day(zone) == 31) dateString += "st";
   else if (day(zone) == 2 || day(zone) == 22) dateString += "nd";
   else if (day(zone) == 3 || day(zone) == 23) dateString += "rd";
   else dateString += "th";

   dateString += " ";
   dateString += monthStr(month(zone));
   dateString += " ";
   dateString += year(zone);

   // Print time to serial port
   Serial.print(hour(zone));
   Serial.print(":");
   Serial.print(minute(zone));
   Serial.print(":");
   Serial.print(second(zone));
   Serial.print(" ");
   // Print time zone
   Serial.print(tzCode);
   Serial.print(" ");

   // Print date
   Serial.print(day(zone));
   Serial.print("/");
   Serial.print(month(zone));
   Serial.print("/");
   Serial.print(year(zone));
   Serial.print("  ");

   // Now test some other functions that might be useful one day!
   Serial.print(dayStr(weekday(zone)));
   Serial.print(" ");
   Serial.print(monthStr(month(zone)));
   Serial.print(" ");
   Serial.print(dayShortStr(weekday(zone)));
   Serial.print(" ");
   Serial.print(monthShortStr(month(zone)));
   Serial.println();
}

//====================================================================================
// Decode the NTP message and print to serial port
//====================================================================================
void decodeNTP(void)
{
  timeValid = false;
  uint32_t waitTime = millis() + 300;
  while (millis() < waitTime && !timeValid)
  {
    yield();
    if (udp.parsePacket()) {
      newRecvTime = millis();
      // We've received a packet, read the data from it
      udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

      Serial.print("NTP response time was : ");
      Serial.print(300 - (waitTime - newRecvTime));
      Serial.println(" ms");

      Serial.print("Time since last sync is: ");
      Serial.print((newRecvTime - lastRecvTime) / 1000.0);
      Serial.println(" s");
      lastRecvTime = newRecvTime;

      // The timestamp starts at byte 40 of the received packet and is four bytes,
      // or two words, long. First, extract the two words:
      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);

      // Combine the four bytes (two words) into a long integer
      // this is NTP time (seconds since Jan 1 1900):
      unsigned long secsSince1900 = highWord << 16 | lowWord;
      utc = secsSince1900 - 2208988800UL;

      setTime(utc); // Set UTC time
      timeValid = true;

      // Now convert NTP Unix time (Seconds since Jan 1 1900) into everyday time:
      // UTC time starts on Jan 1 1970. In seconds the difference is 2208988800:
      const unsigned long seventyYears = 2208988800UL;
      // Subtract seventy years:
      unsigned long epoch = secsSince1900 - seventyYears;

      // Print the hour, minute and second:
      Serial.print("Received NTP UTC time : ");       // UTC is the time at Greenwich Meridian (GMT)

      uint8_t hh = (epoch  % 86400L) / 3600;
      Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
      Serial.print(':');
      if ( ((epoch % 3600) / 60) < 10 ) {
        // In the first 10 minutes of each hour, we'll want a leading '0'
        Serial.print('0');
      }

      uint8_t mm = (epoch  % 3600) / 60;
      Serial.print(mm); // print the minute (3600 equals secs per minute)
      Serial.print(':');

      uint8_t ss = epoch % 60;
      if ( ss < 10 ) {
        // In the first 10 seconds of each minute, we'll want a leading '0'
        Serial.print('0');
      }
      Serial.println(ss); // print the second
    }
  }

  // Keep a count of missing or bad NTP replies
  
  if ( timeValid ) {
    no_packet_count = 0;
  }
  else
  {
    Serial.println("No NTP packet");
    no_packet_count++;
  }

  if (no_packet_count >=10) {
    no_packet_count = 0;
    // TODO: Flag the lack of sync on the display here
    Serial.println("No NTP packet in last 10 minutes");
  }
}

//====================================================================================
// Send an NTP request to the time server at the given address
//====================================================================================
unsigned long sendNTPpacket(IPAddress& address)
{
  // Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

//====================================================================================

// This function is not used at the moment
void displayTime(void)
{

  if (now() != utc) { //update the display only if time has changed
    newTickTime = millis();
    utc = now(); // utc is a static copy of the running utc time now

 // ##################################################################################################
 // #    The next 2 code lines are the ones to change if different time zones are to be displayed    #
 // #                                                                                                #
    time_t analogue_time =    UK.toLocal(utc, &tz1_Code);  // Convert UTC to UK time, get zone code  #
    time_t digital_time  =    UK.toLocal(utc, &tz2_Code);  // Convert UTC to EU time, get zone code  #
 // #                      ^^^^^  change this, see "Setting" section above for zone references       #
 // #                                                                                                #
 // ##################################################################################################


    // If we rebooted and time is now valid then update the clock hands
    if (rebooted && timeValid) {
      rebooted = 0;
    }

    // If minute has changed then request new time from NTP server
    if (minute() != lastMinute) {
      lastMinute = minute();
      requestTime();
    }

    lastTickTime = newTickTime;
  }
}

