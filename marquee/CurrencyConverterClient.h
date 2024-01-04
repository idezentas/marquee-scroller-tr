#include "WString.h"
/** The MIT License (MIT)

Copyright (c) 2024 idezentas

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

class CurrencyConverterClient
{

private:
  String myApiKey = "";
  String myBaseCurrency1 = "";
  String myBaseCurrency2 = "";
  String myBaseCurrency3 = "";
  String myTargetCurrency = "";

  const char *servername = "api.dev.me"; // remote server we will connect to

  typedef struct
  {
    String target;
    String targetCurrencyName;
    String baseCurrencyName;
    String requested_time;
    String convertedText;
    boolean cached;
    String error;
  } currency;

  currency currencies[5];

  String roundValue(String value);

public:
  CurrencyConverterClient(String ApiKey, String BaseCurrency1, String BaseCurrency2, String BaseCurrency3, String TargetCurrency);
  void updateCurrency(String BaseCurrency, String TargetCurrency, int index);
  void updateCurrencyApiKey(String ApiKey);
  void updateBaseCurrency1(String BaseCurrency1);
  void updateBaseCurrency2(String BaseCurrency2);
  void updateBaseCurrency3(String BaseCurrency3);
  void updateTargetCurrency(String TargetCurrency);

  String getTargetCurrency(int index);
  String getTargetCurrencyFormatted(int index);
  String getTargetCurrencyName(int index);
  String getBaseCurrencyName(int index);
  String getRequestTime(int index);
  boolean getCached(int index);
  String getError(int index);
};
