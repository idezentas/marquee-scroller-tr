#include <algorithm>
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

#include "CurrencyConverterClient.h"
#include "math.h"

CurrencyConverterClient::CurrencyConverterClient(String BaseCurrency1, String BaseCurrency2, String TargetCurrency)
{
  updateBaseCurrency1(BaseCurrency1);
  updateBaseCurrency2(BaseCurrency2);
  updateTargetCurrency(TargetCurrency);
}

void CurrencyConverterClient::updateBaseCurrency1(String BaseCurrency1)
{
  myBaseCurrency1 = BaseCurrency1;
  if (myBaseCurrency1 == "")
  {
    myBaseCurrency1 = "EUR";
  }
}

void CurrencyConverterClient::updateBaseCurrency2(String BaseCurrency2)
{
  myBaseCurrency2 = BaseCurrency2;
  if (myBaseCurrency2 == "")
  {
    myBaseCurrency2 = "USD";
  }
}

void CurrencyConverterClient::updateTargetCurrency(String TargetCurrency)
{
  myTargetCurrency = TargetCurrency;
  if (myTargetCurrency == "")
  {
    myTargetCurrency = "TRY";
  }
}

void CurrencyConverterClient::updateCurrency(String BaseCurrency, String TargetCurrency, int index)
{
  WiFiClient currencyClient;

  String apiGetData = "GET /daily/" + BaseCurrency + ".json" + " HTTP/1.0";

  Serial.println("Getting Currency Data");
  Serial.println(apiGetData);
  currencies[index].cached = false;
  currencies[index].error = "";
  if (currencyClient.connect(servername, 80))
  { // starts client connection, checks for connection
    currencyClient.println(apiGetData);
    currencyClient.println("Host: " + String(servername));
    currencyClient.println("User-Agent: ArduinoWiFi/1.1");
    currencyClient.println("Connection: close");
    currencyClient.println();
  }
  else
  {
    Serial.println("Connection for Currency data failed"); // error message if no client connect
    Serial.println();
    currencies[index].error = "Connection for Currency data failed";
    return;
  }

  while (currencyClient.connected() && !currencyClient.available())
    delay(1); // waits for data

  Serial.println("Waiting for data");

  // Check HTTP status
  char status[32] = {0};
  currencyClient.readBytesUntil('\r', status, sizeof(status));
  Serial.println("Response Header: " + String(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0)
  {
    Serial.print("Unexpected response: ");
    Serial.println(status);
    currencies[index].error = "Currency Data Error: " + String(status);
    return;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!currencyClient.find(endOfHeaders))
  {
    Serial.println(F("Invalid response"));
    return;
  }

  String targetFilter = TargetCurrency;
  targetFilter.toLowerCase();

  JsonDocument filter;
  filter[targetFilter] = true;

  JsonDocument root;
  DeserializationError error = deserializeJson(root, currencyClient, DeserializationOption::Filter(filter));
  if (error)
  {
    Serial.println("Currency Data Parsing failed!");
    currencies[index].error = "Currency Data Parsing failed!";
    Serial.println(error.c_str());
    return;
  }

  currencyClient.stop(); // stop client

  currencies[index].base_currency = BaseCurrency;
  currencies[index].target_currency = TargetCurrency;

  JsonObject tarCurr_ = root[targetFilter];
  currencies[index].code = tarCurr_["code"].as<String>();
  currencies[index].alphaCode = tarCurr_["alphaCode"].as<String>();
  currencies[index].numericCode = tarCurr_["numericCode"].as<String>();
  currencies[index].rate = tarCurr_["rate"].as<String>();
  currencies[index].date = tarCurr_["date"].as<String>();
  currencies[index].inverseRate = tarCurr_["inverseRate"].as<String>();

  Serial.println();
  Serial.println("base_currency: " + currencies[index].base_currency);
  Serial.println("target_currency: " + currencies[index].target_currency);
  Serial.println("date: " + currencies[index].date);
  Serial.println("rate: " + currencies[index].rate);
  Serial.println("inverseRate: " + currencies[index].inverseRate);
  Serial.println("code: " + currencies[index].code);
  Serial.println("alphaCode: " + currencies[index].alphaCode);
  Serial.println("numericCode: " + currencies[index].numericCode);
  Serial.println();
}

String CurrencyConverterClient::roundValue(String value)
{
  float f = value.toFloat();
  int rounded = (int)(f + 0.5f);
  return String(rounded);
}

String CurrencyConverterClient::getTargetCurrency(int index)
{
  return currencies[index].rate;
}

String CurrencyConverterClient::getInverseCurrency(int index)
{
  return currencies[index].inverseRate;
}

String CurrencyConverterClient::getTargetCurrencyName(int index)
{
  return currencies[index].target_currency;
}

String CurrencyConverterClient::getBaseCurrencyName(int index)
{
  return currencies[index].base_currency;
}

String CurrencyConverterClient::getTargetCurrencyFormatted(int index)
{
  float f = currencies[index].rate.toFloat();
  return String(f, 2);
}

String CurrencyConverterClient::getInverseCurrencyFormatted(int index)
{
  float f = currencies[index].inverseRate.toFloat();
  return String(f, 2);
}

String CurrencyConverterClient::getRequestTime(int index)
{
  return currencies[index].date;
}

boolean CurrencyConverterClient::getCached(int index)
{
  return currencies[index].cached;
}

String CurrencyConverterClient::getError(int index)
{
  return currencies[index].error;
}
