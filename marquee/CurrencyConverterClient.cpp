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

CurrencyConverterClient::CurrencyConverterClient(String ApiKey, String BaseCurrency1, String BaseCurrency2, String TargetCurrency)
{
  updateBaseCurrency1(BaseCurrency1);
  updateBaseCurrency2(BaseCurrency2);
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
    currencies[index].error = "Please provide an API key for currency.";
    Serial.println(currencies[index].error);
    return;
  }

  String apiGetData = "GET /v1-convert-currency?from=" + BaseCurrency + "&to=" + TargetCurrency + "&x-api-key=" + myApiKey + " HTTP/1.1";

  Serial.println("Getting Currency Data for " + BaseCurrency + " / " + TargetCurrency);
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
    Serial.print(F("Unexpected response: "));
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

  JsonDocument root;
  DeserializationError error = deserializeJson(root, currencyClient);
  if (error)
  {
    Serial.println(F("Currency Data Parsing failed!"));
    currencies[index].error = "Currency Data Parsing failed!";
    return;
  }
  currencyClient.stop(); // stop client

  currencies[index].target = root["exchangeRate"].as<String>();
  currencies[index].targetCurrencyName = root["to"].as<String>();
  currencies[index].baseCurrencyName = root["from"].as<String>();
  currencies[index].convertedText = root["convertedText"].as<String>();
  currencies[index].requested_time = root["rateTime"].as<String>();

  Serial.println("baseCurrencyName: " + currencies[index].baseCurrencyName);
  Serial.println("targetCurrencyName: " + currencies[index].targetCurrencyName);
  Serial.println("target: " + currencies[index].target);
  Serial.println("convertedText: " + currencies[index].convertedText);
  Serial.println("requested_time: " + currencies[index].requested_time);
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

String CurrencyConverterClient::getRequestTime(int index)
{
  return currencies[index].requested_time;
}

boolean CurrencyConverterClient::getCached(int index)
{
  return currencies[index].cached;
}

String CurrencyConverterClient::getError(int index)
{
  return currencies[index].error;
}
