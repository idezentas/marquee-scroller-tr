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
    prayers[index].error = "Please enter address for prayes time.";
    Serial.println(prayers[index].error);
    return;
  }

  String apiGetData = "GET /v1/timingsByAddress/now?address=" + address + "&method=" + myMethodID + " HTTP/1.0";

  Serial.println("Getting Prayers Time Data");
  Serial.println(apiGetData);
  prayers[index].cached = false;
  prayers[index].error = "";
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
    prayers[index].error = "Connection for prayers time data failed";
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
    Serial.print("Unexpected response: ");
    Serial.println(status);
    prayers[index].error = "Prayers Time Data Error: " + String(status);
    return;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!prayersClient.find(endOfHeaders))
  {
    Serial.println("Invalid response");
    return;
  }

  JsonDocument root;
  DeserializationError error = deserializeJson(root, prayersClient);
  if (error)
  {
    Serial.println("Prayers Time Data Parsing failed!");
    Serial.println(error.c_str());
    prayers[index].error = "Prayers Time Data Parsing failed!" + String(error.c_str());
    return;
  }

  prayersClient.stop(); // stop client

  JsonObject timings_ = root["data"]["timings"];
  prayers[index].Fajr = timings_["Fajr"].as<String>();
  prayers[index].Sunrise = timings_["Sunrise"].as<String>();
  prayers[index].Dhuhr = timings_["Dhuhr"].as<String>();
  prayers[index].Asr = timings_["Asr"].as<String>();
  prayers[index].Sunset = timings_["Sunset"].as<String>();
  prayers[index].Maghrib = timings_["Maghrib"].as<String>();
  prayers[index].Isha = timings_["Isha"].as<String>();
  prayers[index].Imsak = timings_["Imsak"].as<String>();
  prayers[index].Midnight = timings_["Midnight"].as<String>();

  JsonObject hijri_ = root["data"]["date"]["hijri"];
  prayers[index].hijriDate = hijri_["date"].as<String>();
  prayers[index].hijriCalender = hijri_["designation"]["expanded"].as<String>();

  JsonObject gregorian_ = root["data"]["date"]["gregorian"];
  prayers[index].gregorianDate = gregorian_["date"].as<String>();
  prayers[index].gregorianCalender = gregorian_["designation"]["expanded"].as<String>();

  prayers[index].methodName = root["data"]["meta"]["method"]["name"].as<String>();

  Serial.println("Fajr: " + prayers[index].Fajr);
  Serial.println("Sunrise: " + prayers[index].Sunrise);
  Serial.println("Dhuhr: " + prayers[index].Dhuhr);
  Serial.println("Asr: " + prayers[index].Asr);
  Serial.println("Maghrib: " + prayers[index].Maghrib);
  Serial.println("Isha: " + prayers[index].Isha);
  Serial.println("Sunset: " + prayers[index].Sunset);
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

boolean PrayersClient::getCached(int index)
{
  return prayers[index].cached;
}

String PrayersClient::getError(int index)
{
  return prayers[index].error;
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

String PrayersClient::encodeHtmlString(String msg)
{
  String encodedMsg = msg;
  // Restore special characters that are misformed to %char by the client browser
  encodedMsg.replace(" ", "%20");
  encodedMsg.replace("Ç", "%C3%87");
  encodedMsg.replace("ç", "%C3%A7");
  encodedMsg.replace("Ö", "%C3%96");
  encodedMsg.replace("ö", "%C3%B6");
  encodedMsg.replace("Ü", "%C3%9C");
  encodedMsg.replace("ü", "%C3%BC");
  encodedMsg.replace("ı", "%C4%B1");
  encodedMsg.replace("İ", "%C4%B0");
  encodedMsg.replace("Ş", "%C5%9E");
  encodedMsg.replace("ş", "%C5%9F");
  encodedMsg.replace("Ğ", "%C4%9E");
  encodedMsg.replace("ğ", "%C4%9F");
  encodedMsg.trim();
  return encodedMsg;
}