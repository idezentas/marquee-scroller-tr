#include <algorithm>
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

#include "CurrencyConverterClient.h"
#include "math.h"

CurrencyConverterClient::CurrencyConverterClient(String ApiKey, String BaseCurrency1, String BaseCurrency2, String BaseCurrency3, String TargetCurrency)
{
  updateBaseCurrency1(BaseCurrency1);
  updateBaseCurrency2(BaseCurrency2);
  updateBaseCurrency3(BaseCurrency3);
  updateTargetCurrency(TargetCurrency);
  myApiKey = ApiKey;
}

void CurrencyConverterClient::updateCurrencyApiKey(String ApiKey)
{
  myApiKey = ApiKey;
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

void CurrencyConverterClient::updateBaseCurrency3(String BaseCurrency3)
{
  myBaseCurrency3 = BaseCurrency3;
  if (myBaseCurrency3 == "")
  {
    myBaseCurrency3 = "GBP";
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

  if (myApiKey == "")
  {
    currencies[0].error = "Please provide an API key for currency.";
    Serial.println(currencies[0].error);
    return;
  }

  String currencyCode = BaseCurrency + "_" + TargetCurrency;
  String apiGetData = "GET /api/v7/convert?q=" + currencyCode + "&compact=ultra&apiKey=" + myApiKey + " HTTP/1.1";

  Serial.println("Getting Currency Data for " + BaseCurrency + " / " + TargetCurrency);
  Serial.println(apiGetData);
  currencies[0].cached = false;
  currencies[0].error = "";
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
    currencies[0].error = "Connection for Currency data failed";
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
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    currencies[0].error = "Currency Data Error: " + String(status);
    return;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!currencyClient.find(endOfHeaders))
  {
    Serial.println(F("Invalid response"));
    return;
  }

  // V6
  const size_t bufferSize = 50;
  DynamicJsonDocument root(bufferSize);
  DeserializationError error = deserializeJson(root, currencyClient);
  if (error)
  {
    Serial.println(F("Currency Data Parsing failed!"));
    currencies[0].error = "Currency Data Parsing failed!";
    return;
  }
  currencyClient.stop(); // stop client

  size_t msrLen = bufferSize / 5;

  if (measureJson(root) <= msrLen)
  {
    Serial.println("Error Does not look like we got the data.  Size: " + String(measureJson(root)));
    currencies[0].cached = true;
    currencies[0].error = root["message"].as<String>();
    Serial.println("Error: " + currencies[0].error);
    return;
  }

  currencies[index].target = root[currencyCode].as<String>();
  currencies[index].targetCurrencyName = TargetCurrency;
  currencies[index].baseCurrencyName = BaseCurrency;

  Serial.println("baseCurrencyName: " + currencies[index].baseCurrencyName);
  Serial.println("targetCurrencyName: " + currencies[index].targetCurrencyName);
  Serial.println("target (" + currencyCode + "): " + currencies[index].target);
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
  return currencies[index].target;
}

String CurrencyConverterClient::getTargetCurrencyName(int index)
{
  return currencies[index].targetCurrencyName;
}

String CurrencyConverterClient::getBaseCurrencyName(int index)
{
  return currencies[index].baseCurrencyName;
}

String CurrencyConverterClient::getTargetCurrencyFormatted(int index)
{
  float f = currencies[index].target.toFloat();
  return String(f, 2);
}

boolean CurrencyConverterClient::getCached()
{
  return currencies[0].cached;
}

String CurrencyConverterClient::getError()
{
  return currencies[0].error;
}
