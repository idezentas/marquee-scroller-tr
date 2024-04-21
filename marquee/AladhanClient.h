#include "WString.h"
/** The MIT License (MIT)

Copyright (c) 2023 idezentas

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
#include <ArduinoJson.h>
#include <TimeLib.h>

class PrayersClient
{

private:
  String myMethodID;

  const char *servername = "api.aladhan.com"; // remote server we will connect to

  typedef struct
  {
    String Fajr;
    String Sunrise;
    String Dhuhr;
    String Asr;
    String Sunset;
    String Maghrib;
    String Isha;
    String Imsak;
    String Midnight;
    String hijriDate;
    String hijriCalender;
    String gregorianDate;
    String gregorianCalender;
    String methodName;
    boolean cached;
    String error;
  } prayer;

  prayer prayers[5];

  String roundValue(String value);

public:
  void updatePrayerTimesAddress(String address, int index);
  PrayersClient(String id);
  void updateMethodID(String methodID);

  String getFajr(int index);
  String getSunrise(int index);
  String getDhuhr(int index);
  String getAsr(int index);
  String getSunset(int index);
  String getMaghrib(int index);
  String getIsha(int index);
  String getImsak(int index);
  String getMidnight(int index);
  String getHijriDate(int index);
  String getHijriCalender(int index);
  String getGregorianDate(int index);
  String getGregorianCalender(int index);
  String getMethodName(int index);
  boolean getCached(int index);
  String getError(int index);
  String cleanText(String text);
  String encodeHtmlString(String msg);
};
