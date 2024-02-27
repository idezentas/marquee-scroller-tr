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

#include "AladhanClient.h"
#include "math.h"

PrayersClient::PrayersClient(String methodID)
{
  myMethodID = methodID;
}

void PrayersClient::updateMethodID(String methodID)
{
  myMethodID = methodID;
}

void PrayersClient::updatePrayerTimesAddress(String address, int index)
{
  WiFiClient prayersClient;

  if (address == "")
  {
    prayers[0].error = "Please enter address for prayes time.";
    Serial.println(prayers[0].error);
    return;
  }

  String apiGetData = "GET /v1/timingsByAddress/now?address=" + address + "&method=" + myMethodID + " HTTP/1.1";

  // Serial.println("Getting Prayers Time Data");
  Serial.println(apiGetData);
  prayers[0].cached = false;
  prayers[0].error = "";
  if (prayersClient.connect(servername, 80))
  { // starts client connection, checks for connection
    prayersClient.println(apiGetData);
    prayersClient.println("Host: " + String(servername));
    prayersClient.println("User-Agent: ArduinoWiFi/1.1");
    prayersClient.println("Connection: close");
    prayersClient.println();
  }
  else
  {
    Serial.println("Connection for prayers time data failed"); // error message if no client connect
    Serial.println();
    prayers[0].error = "Connection for prayers time data failed";
    return;
  }

  while (prayersClient.connected() && !prayersClient.available())
    delay(1); // waits for data

  Serial.println("Waiting for data");

  // Check HTTP status
  char status[32] = {0};
  prayersClient.readBytesUntil('\r', status, sizeof(status));
  Serial.println("Response Header: " + String(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0)
  {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    prayers[0].error = "Prayers Time Data Error: " + String(status);
    return;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!prayersClient.find(endOfHeaders))
  {
    Serial.println(F("Invalid response"));
    return;
  }

  JsonDocument root;
  DeserializationError error = deserializeJson(root, prayersClient);
  if (error)
  {
    Serial.println(F("Prayers Time Data Parsing failed!"));
    prayers[0].error = "Prayers Time Data Parsing failed!";
    return;
  }

  prayersClient.stop(); // stop client

  prayers[index].Fajr = root["data"]["timings"]["Fajr"].as<String>();
  prayers[index].Sunrise = root["data"]["timings"]["Sunrise"].as<String>();
  prayers[index].Dhuhr = root["data"]["timings"]["Dhuhr"].as<String>();
  prayers[index].Asr = root["data"]["timings"]["Asr"].as<String>();
  prayers[index].Sunset = root["data"]["timings"]["Sunset"].as<String>();
  prayers[index].Maghrib = root["data"]["timings"]["Maghrib"].as<String>();
  prayers[index].Isha = root["data"]["timings"]["Isha"].as<String>();
  prayers[index].Imsak = root["data"]["timings"]["Imsak"].as<String>();
  prayers[index].Midnight = root["data"]["timings"]["Midnight"].as<String>();
  prayers[index].hijriDate = root["data"]["date"]["hijri"]["date"].as<String>();
  prayers[index].hijriCalender = root["data"]["date"]["hijri"]["designation"]["expanded"].as<String>();
  prayers[index].gregorianDate = root["data"]["date"]["gregorian"]["date"].as<String>();
  prayers[index].gregorianCalender = root["data"]["date"]["gregorian"]["designation"]["expanded"].as<String>();
  prayers[index].methodName = root["data"]["meta"]["method"]["name"].as<String>();

  Serial.println("Fajr: " + prayers[index].Fajr);
  Serial.println("Sunrise: " + prayers[index].Sunrise);
  Serial.println("Dhuhr: " + prayers[index].Dhuhr);
  Serial.println("Asr: " + prayers[index].Asr);
  Serial.println("Sunset: " + prayers[index].Sunset);
  Serial.println("Maghrib: " + prayers[index].Maghrib);
  Serial.println("Imsak: " + prayers[index].Imsak);
  Serial.println("Midnight: " + prayers[index].Midnight);
  Serial.println("hijriDate: " + prayers[index].hijriDate);
  Serial.println("hijriCalender: " + prayers[index].hijriCalender);
  Serial.println("gregorianDate: " + prayers[index].gregorianDate);
  Serial.println("gregorianCalender: " + prayers[index].gregorianCalender);
  Serial.println("methodName: " + prayers[index].methodName);
  Serial.println();
}

String PrayersClient::getFajr(int index)
{
  return prayers[index].Fajr;
}

String PrayersClient::getSunrise(int index)
{
  return prayers[index].Sunrise;
}

String PrayersClient::getDhuhr(int index)
{
  return prayers[index].Dhuhr;
}

String PrayersClient::getAsr(int index)
{
  return prayers[index].Asr;
}

String PrayersClient::getSunset(int index)
{
  return prayers[index].Sunset;
}

String PrayersClient::getMaghrib(int index)
{
  return prayers[index].Maghrib;
}

String PrayersClient::getIsha(int index)
{
  return prayers[index].Isha;
}

String PrayersClient::getImsak(int index)
{
  return prayers[index].Imsak;
}

String PrayersClient::getMidnight(int index)
{
  return prayers[index].Midnight;
}

String PrayersClient::getHijriDate(int index)
{
  return prayers[index].hijriDate;
}

String PrayersClient::getHijriCalender(int index)
{
  return prayers[index].hijriCalender;
}

String PrayersClient::getGregorianDate(int index)
{
  return prayers[index].gregorianDate;
}

String PrayersClient::getGregorianCalender(int index)
{
  return prayers[index].gregorianCalender;
}

String PrayersClient::getMethodName(int index)
{
  return prayers[index].methodName;
}

boolean PrayersClient::getCached()
{
  return prayers[0].cached;
}

String PrayersClient::getError()
{
  return prayers[0].error;
}

String PrayersClient::roundValue(String value)
{
  float f = value.toFloat();
  int rounded = (int)(f + 0.5f);
  return String(rounded);
}

String PrayersClient::cleanText(String text)
{
  text.replace("’", "'");
  text.replace("“", "\"");
  text.replace("”", "\"");
  text.replace("`", "'");
  text.replace("‘", "'");
  text.replace("„", "'");
  text.replace("\\\"", "'");
  text.replace("•", "-");
  text.replace("é", "e");
  text.replace("è", "e");
  text.replace("ë", "e");
  text.replace("ê", "e");
  text.replace("à", "a");
  text.replace("â", "a");
  text.replace("ù", "u");
  text.replace("Ğ", "G");
  text.replace("î", "i");
  text.replace("ï", "i");
  text.replace("ô", "o");
  text.replace("…", "...");
  text.replace("–", "-");
  text.replace("Â", "A");
  text.replace("À", "A");
  text.replace("æ", "ae");
  text.replace("Æ", "AE");
  text.replace("É", "E");
  text.replace("È", "E");
  text.replace("Ë", "E");
  text.replace("Ô", "O");
  text.replace("Ö", "Oe");
  text.replace("ö", "oe");
  text.replace("œ", "oe");
  text.replace("Œ", "OE");
  text.replace("Ù", "U");
  text.replace("Û", "U");
  text.replace("Ä", "Ae");
  text.replace("ä", "ae");
  text.replace("ß", "ss");
  text.replace("»", "'");
  text.replace("«", "'");
  text.replace("u0130", "İ");
  text.replace("u0131", "ı");
  text.replace("u015f", "ş");
  text.replace("u015e", "Ş");
  text.replace("u011f", "ğ");
  text.replace("u011e", "Ğ");
  text.replace("u00c7", "Ç");
  text.replace("u00e7", "ç");
  text.replace("u00f6", "ö");
  text.replace("u00d6", "Ö");
  text.replace("u00fc", "ü");
  text.replace("u00dc", "Ü");
  return text;
}
