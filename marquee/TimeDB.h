/** The MIT License (MIT)

Copyright (c) 2019 magnum129@github

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once
#include <ESP8266WiFi.h>
#include <TimeLib.h> // https://github.com/PaulStoffregen/Time
#include <ArduinoJson.h>

class TimeDB
{
public:
  TimeDB(String apiKey);
  void updateConfig(String apiKey, String lat, String lon);
  time_t getTime();
  void getCityTime(String apiKey, String lat, String lon, int index);
  void convertTimezone(String apiKey, String fromTimezone, String toTimezone, int index);
  String getDayName();
  String getMonthName();
  String getAmPm();
  String zeroPad(int number);
  String getError(int index);
  String cleanText(String text);
  String getCountryCode(int index);
  String getCountryName(int index);
  String getRegionName(int index);
  String getCityName(int index);
  String getZoneName(int index);
  String getAbbreviation(int index);
  String getFormatted(int index);
  String getTimestamp2Date(int index);
  String useDST(int index);
  String getNextAbbreviation(int index);
  String getZoneStart(int index);
  String getZoneEnd(int index);
  String getFromZoneName(int index);
  String getFromAbbreviation(int index);
  String getFromTimestamp(int index);
  String getToZoneName(int index);
  String getToAbbreviation(int index);
  String getToTimestamp(int index);
  String getFromTimestamp2Date(int index);
  String getToTimestamp2Date(int index);
  String getOffsetDifferenceString(int index);
  String getGmtOffsetString(int index);
  int getGmtOffset(int index);
  int getOffsetDifference(int index);

private:
  const char *servername = "api.timezonedb.com"; // remote server we will connect to
  long localMillisAtUpdate;
  String myApiKey;
  String myLat;
  String myLon;

  typedef struct
  {
    String errorMessage;
    String status;
    String message;
    String countryCode;
    String countryName;
    String regionName;
    String cityName;
    String zoneName;
    String abbreviation;
    String timestamp;
    String formatted;
    String gmtOffset;
    String dst;
    String zoneStart;
    String zoneEnd;
    String nextAbbreviation;
    String fromZoneName;
    String fromAbbreviation;
    String fromTimestamp;
    String toZoneName;
    String toAbbreviation;
    String toTimestamp;
    String offsetDifference;
  } timeS;

  timeS timeStruct[5];
};
