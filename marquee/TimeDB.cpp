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

#include "TimeDB.h"

int comm_timeout = 10000;

TimeDB::TimeDB(String apiKey)
{
  myApiKey = apiKey;
}

void TimeDB::updateConfig(String apiKey, String lat, String lon)
{
  myApiKey = apiKey;
  myLat = lat;
  myLon = lon;
}

time_t TimeDB::getTime()
{
  WiFiClient client;
  String apiGetData = "GET /v2.1/get-time-zone?key=" + myApiKey + "&format=json&by=position&lat=" + myLat + "&lng=" + myLon + " HTTP/1.1";
  Serial.println("Getting Time Data for " + myLat + ", " + myLon);
  Serial.println(apiGetData);
  String result = "";
  if (client.connect(servername, 80))
  { // starts client connection, checks for connection
    client.println(apiGetData);
    client.println("Host: " + String(servername));
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Connection: close");
    client.println();
  }
  else
  {
    Serial.println("connection for time data failed"); // error message if no client connect
    Serial.println();
    return 20;
  }

  int delay_counter = 0;
  while (client.connected() && !client.available())
  {
    delay(1); // waits for data
    delay_counter++;

    if (delay_counter > comm_timeout)
    {
      Serial.println("timeout waiting for data"); // error message if timeout
      Serial.println();
      timeStruct[0].errorMessage = "timeout waiting for data";
      client.stop();
      return 20;
    }
  }

  Serial.println("Waiting for data");

  boolean record = false;
  delay_counter = 0;
  while (client.connected() || client.available())
  {                         // connected or data available
    char c = client.read(); // gets byte from ethernet buffer
    if (String(c) == "{")
    {
      record = true;
    }
    if (record)
    {
      result = result + c;
    }
    if (String(c) == "}")
    {
      record = false;
    }
    delay(1); // waits for data
    delay_counter++;

    if (delay_counter > comm_timeout)
    {
      Serial.println("timeout waiting for data"); // error message if timeout
      Serial.println();
      timeStruct[0].errorMessage = "timeout waiting for data";
      client.stop();
      return 20;
    }
  }
  client.stop(); // stop client
  Serial.println(result);

  int timeStart = result.lastIndexOf('{'); // trim response to start of JSON -- issue 194
  result = result.substring(timeStart);
  char jsonArray[result.length() + 1];
  result.toCharArray(jsonArray, sizeof(jsonArray));
  jsonArray[result.length() + 1] = '\0';

  JsonDocument root;
  deserializeJson(root, jsonArray);

  for (int inx = 0; inx < 1; inx++)
  {
    timeStruct[inx].errorMessage = root["message"].as<String>();
    timeStruct[inx].status = root["status"].as<String>();
    timeStruct[inx].message = root["message"].as<String>();
    timeStruct[inx].countryCode = root["countryCode"].as<String>();
    timeStruct[inx].countryName = root["countryName"].as<String>();
    timeStruct[inx].regionName = root["regionName"].as<String>();
    timeStruct[inx].cityName = root["cityName"].as<String>();
    timeStruct[inx].zoneName = root["zoneName"].as<String>();
    timeStruct[inx].abbreviation = root["abbreviation"].as<String>();
    timeStruct[inx].formatted = root["formatted"].as<String>();
    timeStruct[inx].timestamp = root["timestamp"].as<String>();
    timeStruct[inx].gmtOffset = root["gmtOffset"].as<String>();
    timeStruct[inx].dst = root["dst"].as<String>();
    timeStruct[inx].zoneStart = root["zoneStart"].as<String>();
    timeStruct[inx].zoneEnd = root["zoneEnd"].as<String>();
    timeStruct[inx].nextAbbreviation = root["nextAbbreviation"].as<String>();

    Serial.println("status: " + timeStruct[inx].status);
    Serial.println("message: " + timeStruct[inx].message);
    Serial.println("countryCode: " + timeStruct[inx].countryCode);
    Serial.println("countryName: " + timeStruct[inx].countryName);
    Serial.println("regionName: " + timeStruct[inx].regionName);
    Serial.println("cityName: " + timeStruct[inx].cityName);
    Serial.println("zoneName: " + timeStruct[inx].zoneName);
    Serial.println("abbreviation: " + timeStruct[inx].abbreviation);
    Serial.println("formatted: " + timeStruct[inx].formatted);
    Serial.println("timestamp: " + timeStruct[inx].timestamp + " | " + getTimestamp2Date(inx));
    Serial.println("gmtOffset: " + String(getGmtOffset(inx)));
    Serial.println("dst: " + useDST(inx));
    Serial.println("zoneStart: " + timeStruct[inx].zoneStart + " | " + getZoneStart(inx));
    Serial.println("zoneEnd: " + timeStruct[inx].zoneEnd + " | " + getZoneEnd(inx));
    Serial.println("nextAbbreviation: " + timeStruct[inx].nextAbbreviation);
    Serial.println();
  }

  localMillisAtUpdate = millis();
  Serial.println();
  if (root["timestamp"] == 0)
  {
    return 20;
  }
  else
  {
    return root["timestamp"].as<long>();
  }
}

void TimeDB::getCityTime(String apiKey, String lat, String lon, int index)
{
  WiFiClient client;
  String apiGetData = "GET /v2.1/get-time-zone?key=" + apiKey + "&format=json&by=position&lat=" + lat + "&lng=" + lon + " HTTP/1.1";
  Serial.println("Getting Time Data for " + lat + ", " + lon);
  Serial.println(apiGetData);
  String result = "";
  if (client.connect(servername, 80))
  { // starts client connection, checks for connection
    client.println(apiGetData);
    client.println("Host: " + String(servername));
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Connection: close");
    client.println();
  }
  else
  {
    Serial.println("connection for time data failed"); // error message if no client connect
    Serial.println();
    timeStruct[index].errorMessage = "connection for time data failed";
    return;
  }

  int delay_counter = 0;
  while (client.connected() && !client.available())
  {
    delay(1); // waits for data
    delay_counter++;

    if (delay_counter > comm_timeout)
    {
      Serial.println("timeout waiting for data"); // error message if timeout
      Serial.println();
      timeStruct[index].errorMessage = "timeout waiting for data";
      client.stop();
      return;
    }
  }

  Serial.println("Waiting for data");

  boolean record = false;
  delay_counter = 0;
  while (client.connected() || client.available())
  {                         // connected or data available
    char c = client.read(); // gets byte from ethernet buffer
    if (String(c) == "{")
    {
      record = true;
    }
    if (record)
    {
      result = result + c;
    }
    if (String(c) == "}")
    {
      record = false;
    }
    delay(1); // waits for data
    delay_counter++;

    if (delay_counter > comm_timeout)
    {
      Serial.println("timeout waiting for data"); // error message if timeout
      Serial.println();
      timeStruct[index].errorMessage = "timeout waiting for data";
      client.stop();
      return;
    }
  }
  client.stop(); // stop client
  Serial.println(result);

  int timeStart = result.lastIndexOf('{'); // trim response to start of JSON -- issue 194
  result = result.substring(timeStart);
  char jsonArray[result.length() + 1];
  result.toCharArray(jsonArray, sizeof(jsonArray));
  jsonArray[result.length() + 1] = '\0';

  JsonDocument root;
  deserializeJson(root, jsonArray);

  timeStruct[index].errorMessage = root["message"].as<String>();
  timeStruct[index].status = root["status"].as<String>();
  timeStruct[index].message = root["message"].as<String>();
  timeStruct[index].countryCode = root["countryCode"].as<String>();
  timeStruct[index].countryName = root["countryName"].as<String>();
  timeStruct[index].regionName = root["regionName"].as<String>();
  timeStruct[index].cityName = root["cityName"].as<String>();
  timeStruct[index].zoneName = root["zoneName"].as<String>();
  timeStruct[index].abbreviation = root["abbreviation"].as<String>();
  timeStruct[index].timestamp = root["timestamp"].as<String>();
  timeStruct[index].formatted = root["formatted"].as<String>();
  timeStruct[index].gmtOffset = root["gmtOffset"].as<String>();
  timeStruct[index].dst = root["dst"].as<String>();
  timeStruct[index].zoneStart = root["zoneStart"].as<String>();
  timeStruct[index].zoneEnd = root["zoneEnd"].as<String>();
  timeStruct[index].nextAbbreviation = root["nextAbbreviation"].as<String>();

  Serial.println("status: " + timeStruct[index].status);
  Serial.println("message: " + timeStruct[index].message);
  Serial.println("countryCode: " + timeStruct[index].countryCode);
  Serial.println("countryName: " + timeStruct[index].countryName);
  Serial.println("regionName: " + timeStruct[index].regionName);
  Serial.println("cityName: " + timeStruct[index].cityName);
  Serial.println("zoneName: " + timeStruct[index].zoneName);
  Serial.println("abbreviation: " + timeStruct[index].abbreviation);
  Serial.println("timestamp: " + timeStruct[index].timestamp + " | " + getTimestamp2Date(index));
  Serial.println("formatted: " + timeStruct[index].formatted);
  Serial.println("gmtOffset: " + String(getGmtOffset(index)));
  Serial.println("dst: " + useDST(index));
  Serial.println("zoneStart: " + timeStruct[index].zoneStart + " | " + getZoneStart(index));
  Serial.println("zoneEnd: " + timeStruct[index].zoneEnd + " | " + getZoneEnd(index));
  Serial.println("nextAbbreviation: " + timeStruct[index].nextAbbreviation);
  Serial.println();
}

void TimeDB::convertTimezone(String apiKey, String fromTimezone, String toTimezone, int index)
{
  WiFiClient client;
  String apiGetData = "GET /v2.1/convert-time-zone?key=" + apiKey + "&format=json&by=position&from=" + fromTimezone + "&to=" + toTimezone + " HTTP/1.1";
  Serial.println("Getting Time Difference for " + fromTimezone + " to " + toTimezone);
  Serial.println(apiGetData);
  String result = "";
  if (client.connect(servername, 80))
  { // starts client connection, checks for connection
    client.println(apiGetData);
    client.println("Host: " + String(servername));
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Connection: close");
    client.println();
  }
  else
  {
    Serial.println("connection for time data failed"); // error message if no client connect
    Serial.println();
    timeStruct[index].errorMessage = "connection for time data failed";
    return;
  }

  int delay_counter = 0;

  while (client.connected() && !client.available())
  {
    delay(1); // waits for data
    delay_counter++;

    if (delay_counter > comm_timeout)
    {
      Serial.println("timeout waiting for data"); // error message if timeout
      Serial.println();
      timeStruct[index].errorMessage = "timeout waiting for data";
      client.stop();
      return;
    }
  }

  Serial.println("Waiting for data");

  boolean record = false;
  delay_counter = 0;
  while (client.connected() || client.available())
  {                         // connected or data available
    char c = client.read(); // gets byte from ethernet buffer
    if (String(c) == "{")
    {
      record = true;
    }
    if (record)
    {
      result = result + c;
    }
    if (String(c) == "}")
    {
      record = false;
    }
    delay(1); // waits for data
    delay_counter++;

    if (delay_counter > comm_timeout)
    {
      Serial.println("timeout waiting for data"); // error message if timeout
      Serial.println();
      timeStruct[index].errorMessage = "timeout waiting for data";
      client.stop();
      return;
    }
  }
  client.stop(); // stop client
  Serial.println(result);

  int timeStart = result.lastIndexOf('{'); // trim response to start of JSON -- issue 194
  result = result.substring(timeStart);
  char jsonArray[result.length() + 1];
  result.toCharArray(jsonArray, sizeof(jsonArray));
  jsonArray[result.length() + 1] = '\0';

  JsonDocument root;
  deserializeJson(root, jsonArray);

  timeStruct[index].errorMessage = root["message"].as<String>();

  timeStruct[index].status = root["status"].as<String>();
  timeStruct[index].message = root["message"].as<String>();
  timeStruct[index].fromZoneName = root["fromZoneName"].as<String>();
  timeStruct[index].fromAbbreviation = root["fromAbbreviation"].as<String>();
  timeStruct[index].fromTimestamp = root["fromTimestamp"].as<String>();
  timeStruct[index].toZoneName = root["toZoneName"].as<String>();
  timeStruct[index].toAbbreviation = root["toAbbreviation"].as<String>();
  timeStruct[index].abbreviation = root["abbreviation"].as<String>();
  timeStruct[index].toTimestamp = root["toTimestamp"].as<String>();
  timeStruct[index].offsetDifference = root["offset"].as<String>();

  Serial.println("status: " + timeStruct[index].status);
  Serial.println("message: " + timeStruct[index].message);
  Serial.println("fromZoneName: " + timeStruct[index].fromZoneName);
  Serial.println("fromAbbreviation: " + timeStruct[index].fromAbbreviation);
  Serial.println("fromTimestamp: " + timeStruct[index].fromTimestamp + " | " + getFromTimestamp2Date(index));
  Serial.println("toZoneName: " + timeStruct[index].toZoneName);
  Serial.println("toAbbreviation: " + timeStruct[index].toAbbreviation);
  Serial.println("toTimestamp: " + timeStruct[index].toTimestamp + " | " + getToTimestamp2Date(index));
  Serial.println("offsetDifference: " + String(getOffsetDifference(index)));
  Serial.println();
}

String TimeDB::getError(int index)
{
  return timeStruct[index].errorMessage;
}

String TimeDB::getAbbreviation(int index)
{
  String rtnValue = "";
  if (timeStruct[index].abbreviation == "null")
  {
    return rtnValue;
  }
  rtnValue = timeStruct[index].abbreviation;
  return rtnValue;
}

String TimeDB::getFormatted(int index)
{
  String rtnValue = "";
  if (timeStruct[index].formatted == "null")
  {
    return rtnValue;
  }
  rtnValue = timeStruct[index].formatted;
  return rtnValue;
}

String TimeDB::getTimestamp2Date(int index)
{
  String rtnValue = "";
  String monthValue = "";
  String weekdayValue = "";

  if (timeStruct[index].timestamp == "null")
  {
    return rtnValue;
  }

  long epoc = timeStruct[index].timestamp.toInt();
  time_t epoch_time_as_time_t = epoc;
  struct tm *time_local = localtime(&epoch_time_as_time_t);

  switch (month(mktime(time_local)))
  {
  case 1:
    monthValue = "Ocak";
    break;
  case 2:
    monthValue = "Şubat";
    break;
  case 3:
    monthValue = "Mart";
    break;
  case 4:
    monthValue = "Nisan";
    break;
  case 5:
    monthValue = "Mayıs";
    break;
  case 6:
    monthValue = "Haziran";
    break;
  case 7:
    monthValue = "Temmuz";
    break;
  case 8:
    monthValue = "Ağustos";
    break;
  case 9:
    monthValue = "Eylül";
    break;
  case 10:
    monthValue = "Ekim";
    break;
  case 11:
    monthValue = "Kasım";
    break;
  case 12:
    monthValue = "Aralık";
    break;
  default:
    rtnValue = "";
  }

  switch (weekday(mktime(time_local)))
  {
  case 1:
    weekdayValue = "Pazar";
    break;
  case 2:
    weekdayValue = "Pazartesi";
    break;
  case 3:
    weekdayValue = "Salı";
    break;
  case 4:
    weekdayValue = "Çarşamba";
    break;
  case 5:
    weekdayValue = "Perşembe";
    break;
  case 6:
    weekdayValue = "Cuma";
    break;
  case 7:
    weekdayValue = "Cumartesi";
    break;
  default:
    weekdayValue = "";
  }

  rtnValue = weekdayValue + ", " + String(day(mktime(time_local))) + " " + monthValue + " " + String(year(mktime(time_local))) + ", " + zeroPad(hour(mktime(time_local))) + ":" + zeroPad(minute(mktime(time_local))) + ":" + zeroPad(second(mktime(time_local)));

  return rtnValue;
}

String TimeDB::getCityName(int index)
{
  String rtnValue = "";
  if (timeStruct[index].cityName == "null")
  {
    return rtnValue;
  }
  rtnValue = timeStruct[index].cityName;
  return rtnValue;
}

String TimeDB::getCountryCode(int index)
{
  String rtnValue = "";
  if (timeStruct[index].countryCode == "null")
  {
    return rtnValue;
  }
  rtnValue = timeStruct[index].countryCode;
  return rtnValue;
}

String TimeDB::getCountryName(int index)
{
  String rtnValue = "";
  if (timeStruct[index].countryName == "null")
  {
    return rtnValue;
  }
  rtnValue = timeStruct[index].countryName;
  return rtnValue;
}

String TimeDB::getRegionName(int index)
{
  String rtnValue = "";
  if (timeStruct[index].regionName == "null")
  {
    return rtnValue;
  }
  rtnValue = timeStruct[index].regionName;
  return rtnValue;
}

String TimeDB::getZoneName(int index)
{
  String rtnValue = "";
  if (timeStruct[index].zoneName == "null")
  {
    return rtnValue;
  }
  rtnValue = timeStruct[index].zoneName;
  return rtnValue;
}

String TimeDB::getNextAbbreviation(int index)
{
  String rtnValue = "";
  if (timeStruct[index].nextAbbreviation == "null")
  {
    return rtnValue;
  }
  rtnValue = timeStruct[index].nextAbbreviation;
  return rtnValue;
}

String TimeDB::getFromZoneName(int index)
{
  String rtnValue = "";
  if (timeStruct[index].fromZoneName == "null")
  {
    return rtnValue;
  }
  rtnValue = timeStruct[index].fromZoneName;
  return rtnValue;
}

String TimeDB::getFromAbbreviation(int index)
{
  String rtnValue = "";
  if (timeStruct[index].fromAbbreviation == "null")
  {
    return rtnValue;
  }
  rtnValue = timeStruct[index].fromAbbreviation;
  return rtnValue;
}

String TimeDB::getFromTimestamp(int index)
{
  String rtnValue = "";
  if (timeStruct[index].fromTimestamp == "null")
  {
    return rtnValue;
  }
  rtnValue = timeStruct[index].fromTimestamp;
  return rtnValue;
}

String TimeDB::getToZoneName(int index)
{
  String rtnValue = "";
  if (timeStruct[index].toZoneName == "null")
  {
    return rtnValue;
  }
  rtnValue = timeStruct[index].toZoneName;
  return rtnValue;
}

String TimeDB::getToAbbreviation(int index)
{
  String rtnValue = "";
  if (timeStruct[index].toAbbreviation == "null")
  {
    return rtnValue;
  }
  rtnValue = timeStruct[index].toAbbreviation;
  return rtnValue;
}

String TimeDB::getToTimestamp(int index)
{
  String rtnValue = "";
  if (timeStruct[index].toTimestamp == "null")
  {
    return rtnValue;
  }
  rtnValue = timeStruct[index].toTimestamp;
  return rtnValue;
}

int TimeDB::getGmtOffset(int index)
{
  int rtnValue = timeStruct[index].gmtOffset.toInt();
  rtnValue = rtnValue / 3600;
  return rtnValue;
}

String TimeDB::getGmtOffsetString(int index)
{
  String rtnValue = "";
  int offDiff = getGmtOffset(index);
  if (offDiff > 0)
  {
    rtnValue = "GMT+" + String(offDiff);
    return rtnValue;
  }
  else
  {
    rtnValue = "GMT" + String(offDiff);
    return rtnValue;
  }
}

int TimeDB::getOffsetDifference(int index)
{
  int rtnValue = timeStruct[index].offsetDifference.toInt();
  rtnValue = rtnValue / 3600;
  return rtnValue;
}

String TimeDB::getOffsetDifferenceString(int index)
{
  String rtnValue = "";
  int offDiff = getOffsetDifference(index);
  if (offDiff > 0)
  {
    int offDiffAbs = abs(offDiff);
    rtnValue = String(offDiffAbs) + " Saat İleride";
    return rtnValue;
  }
  else if (offDiff < 0)
  {
    int offDiffAbs = abs(offDiff);
    rtnValue = String(offDiffAbs) + " Saat Geride";
    return rtnValue;
  }
  else
  {
    rtnValue = "Saat farkı bulunmamaktadır.";
    return rtnValue;
  }
}

String TimeDB::useDST(int index)
{
  String rtnValue = "";
  int dstN = timeStruct[index].dst.toInt();
  switch (dstN)
  {
  case 0:
    rtnValue = "Hayır";
    break;
  case 1:
    rtnValue = "Evet";
    break;
  default:
    break;
  }
  return rtnValue;
}

String TimeDB::getZoneStart(int index)
{
  String rtnValue = "";
  String monthValue = "";
  String weekdayValue = "";

  if (timeStruct[index].zoneStart == "null")
  {
    return rtnValue;
  }

  long epoc = timeStruct[index].zoneStart.toInt();
  time_t epoch_time_as_time_t = epoc;
  struct tm *time_local = localtime(&epoch_time_as_time_t);

  int dstValue = timeStruct[index].gmtOffset.toInt();
  if (dstValue != 0)
  {
    dstValue = dstValue / 3600;
  }

  time_local->tm_hour += dstValue;

  switch (month(mktime(time_local)))
  {
  case 1:
    monthValue = "Ocak";
    break;
  case 2:
    monthValue = "Şubat";
    break;
  case 3:
    monthValue = "Mart";
    break;
  case 4:
    monthValue = "Nisan";
    break;
  case 5:
    monthValue = "Mayıs";
    break;
  case 6:
    monthValue = "Haziran";
    break;
  case 7:
    monthValue = "Temmuz";
    break;
  case 8:
    monthValue = "Ağustos";
    break;
  case 9:
    monthValue = "Eylül";
    break;
  case 10:
    monthValue = "Ekim";
    break;
  case 11:
    monthValue = "Kasım";
    break;
  case 12:
    monthValue = "Aralık";
    break;
  default:
    rtnValue = "";
  }

  switch (weekday(mktime(time_local)))
  {
  case 1:
    weekdayValue = "Pazar";
    break;
  case 2:
    weekdayValue = "Pazartesi";
    break;
  case 3:
    weekdayValue = "Salı";
    break;
  case 4:
    weekdayValue = "Çarşamba";
    break;
  case 5:
    weekdayValue = "Perşembe";
    break;
  case 6:
    weekdayValue = "Cuma";
    break;
  case 7:
    weekdayValue = "Cumartesi";
    break;
  default:
    weekdayValue = "";
  }

  rtnValue = weekdayValue + ", " + String(day(mktime(time_local))) + " " + monthValue + " " + String(year(mktime(time_local))) + ", " + zeroPad(hour(mktime(time_local))) + ":" + zeroPad(minute(mktime(time_local))) + ":" + zeroPad(second(mktime(time_local)));

  return rtnValue;
}

String TimeDB::getZoneEnd(int index)
{
  String rtnValue = "";
  String monthValue = "";
  String weekdayValue = "";

  if (timeStruct[index].zoneEnd == "null")
  {
    return rtnValue;
  }

  long epoc = timeStruct[index].zoneEnd.toInt();
  time_t epoch_time_as_time_t = epoc;
  struct tm *time_local = localtime(&epoch_time_as_time_t);

  int dstValue = timeStruct[index].gmtOffset.toInt();
  if (dstValue != 0)
  {
    dstValue = dstValue / 3600;
  }

  time_local->tm_hour += dstValue;

  switch (month(mktime(time_local)))
  {
  case 1:
    monthValue = "Ocak";
    break;
  case 2:
    monthValue = "Şubat";
    break;
  case 3:
    monthValue = "Mart";
    break;
  case 4:
    monthValue = "Nisan";
    break;
  case 5:
    monthValue = "Mayıs";
    break;
  case 6:
    monthValue = "Haziran";
    break;
  case 7:
    monthValue = "Temmuz";
    break;
  case 8:
    monthValue = "Ağustos";
    break;
  case 9:
    monthValue = "Eylül";
    break;
  case 10:
    monthValue = "Ekim";
    break;
  case 11:
    monthValue = "Kasım";
    break;
  case 12:
    monthValue = "Aralık";
    break;
  default:
    rtnValue = "";
  }

  switch (weekday(mktime(time_local)))
  {
  case 1:
    weekdayValue = "Pazar";
    break;
  case 2:
    weekdayValue = "Pazartesi";
    break;
  case 3:
    weekdayValue = "Salı";
    break;
  case 4:
    weekdayValue = "Çarşamba";
    break;
  case 5:
    weekdayValue = "Perşembe";
    break;
  case 6:
    weekdayValue = "Cuma";
    break;
  case 7:
    weekdayValue = "Cumartesi";
    break;
  default:
    weekdayValue = "";
  }

  rtnValue = weekdayValue + ", " + String(day(mktime(time_local))) + " " + monthValue + " " + String(year(mktime(time_local))) + ", " + zeroPad(hour(mktime(time_local))) + ":" + zeroPad(minute(mktime(time_local))) + ":" + zeroPad(second(mktime(time_local)));

  return rtnValue;
}

String TimeDB::getDayName()
{
  switch (weekday())
  {
  case 1:
    return "Pazar";
    break;
  case 2:
    return "Pazartesi";
    break;
  case 3:
    return "Salı";
    break;
  case 4:
    return "Çarşamba";
    break;
  case 5:
    return "Perşembe";
    break;
  case 6:
    return "Cuma";
    break;
  case 7:
    return "Cumartesi";
    break;
  default:
    return "";
  }
}

String TimeDB::getMonthName()
{
  String rtnValue = "";
  switch (month())
  {
  case 1:
    rtnValue = "Ocak";
    break;
  case 2:
    rtnValue = "Şubat";
    break;
  case 3:
    rtnValue = "Mart";
    break;
  case 4:
    rtnValue = "Nisan";
    break;
  case 5:
    rtnValue = "Mayıs";
    break;
  case 6:
    rtnValue = "Haziran";
    break;
  case 7:
    rtnValue = "Temmuz";
    break;
  case 8:
    rtnValue = "Ağustos";
    break;
  case 9:
    rtnValue = "Eylül";
    break;
  case 10:
    rtnValue = "Ekim";
    break;
  case 11:
    rtnValue = "Kasım";
    break;
  case 12:
    rtnValue = "Aralık";
    break;
  default:
    rtnValue = "";
  }
  return rtnValue;
}

String TimeDB::getAmPm()
{
  String ampmValue = "AM";
  if (isPM())
  {
    ampmValue = "PM";
  }
  return ampmValue;
}

String TimeDB::zeroPad(int number)
{
  if (number < 10)
  {
    return "0" + String(number);
  }
  else
  {
    return String(number);
  }
}

String TimeDB::getFromTimestamp2Date(int index)
{
  String rtnValue = "";
  String monthValue = "";
  String weekdayValue = "";

  if (timeStruct[index].fromTimestamp == "null")
  {
    return rtnValue;
  }

  long epoc = timeStruct[index].fromTimestamp.toInt();
  time_t epoch_time_as_time_t = epoc;
  struct tm *time_local = localtime(&epoch_time_as_time_t);

  switch (month(mktime(time_local)))
  {
  case 1:
    monthValue = "Ocak";
    break;
  case 2:
    monthValue = "Şubat";
    break;
  case 3:
    monthValue = "Mart";
    break;
  case 4:
    monthValue = "Nisan";
    break;
  case 5:
    monthValue = "Mayıs";
    break;
  case 6:
    monthValue = "Haziran";
    break;
  case 7:
    monthValue = "Temmuz";
    break;
  case 8:
    monthValue = "Ağustos";
    break;
  case 9:
    monthValue = "Eylül";
    break;
  case 10:
    monthValue = "Ekim";
    break;
  case 11:
    monthValue = "Kasım";
    break;
  case 12:
    monthValue = "Aralık";
    break;
  default:
    rtnValue = "";
  }

  switch (weekday(mktime(time_local)))
  {
  case 1:
    weekdayValue = "Pazar";
    break;
  case 2:
    weekdayValue = "Pazartesi";
    break;
  case 3:
    weekdayValue = "Salı";
    break;
  case 4:
    weekdayValue = "Çarşamba";
    break;
  case 5:
    weekdayValue = "Perşembe";
    break;
  case 6:
    weekdayValue = "Cuma";
    break;
  case 7:
    weekdayValue = "Cumartesi";
    break;
  default:
    weekdayValue = "";
  }

  rtnValue = weekdayValue + ", " + String(day(mktime(time_local))) + " " + monthValue + " " + String(year(mktime(time_local))) + ", " + zeroPad(hour(mktime(time_local))) + ":" + zeroPad(minute(mktime(time_local))) + ":" + zeroPad(second(mktime(time_local)));

  return rtnValue;
}

String TimeDB::getToTimestamp2Date(int index)
{
  String rtnValue = "";
  String monthValue = "";
  String weekdayValue = "";

  if (timeStruct[index].toTimestamp == "null")
  {
    return rtnValue;
  }

  long epoc = timeStruct[index].toTimestamp.toInt();
  time_t epoch_time_as_time_t = epoc;
  struct tm *time_local = localtime(&epoch_time_as_time_t);

  switch (month(mktime(time_local)))
  {
  case 1:
    monthValue = "Ocak";
    break;
  case 2:
    monthValue = "Şubat";
    break;
  case 3:
    monthValue = "Mart";
    break;
  case 4:
    monthValue = "Nisan";
    break;
  case 5:
    monthValue = "Mayıs";
    break;
  case 6:
    monthValue = "Haziran";
    break;
  case 7:
    monthValue = "Temmuz";
    break;
  case 8:
    monthValue = "Ağustos";
    break;
  case 9:
    monthValue = "Eylül";
    break;
  case 10:
    monthValue = "Ekim";
    break;
  case 11:
    monthValue = "Kasım";
    break;
  case 12:
    monthValue = "Aralık";
    break;
  default:
    rtnValue = "";
  }

  switch (weekday(mktime(time_local)))
  {
  case 1:
    weekdayValue = "Pazar";
    break;
  case 2:
    weekdayValue = "Pazartesi";
    break;
  case 3:
    weekdayValue = "Salı";
    break;
  case 4:
    weekdayValue = "Çarşamba";
    break;
  case 5:
    weekdayValue = "Perşembe";
    break;
  case 6:
    weekdayValue = "Cuma";
    break;
  case 7:
    weekdayValue = "Cumartesi";
    break;
  default:
    weekdayValue = "";
  }

  rtnValue = weekdayValue + ", " + String(day(mktime(time_local))) + " " + monthValue + " " + String(year(mktime(time_local))) + ", " + zeroPad(hour(mktime(time_local))) + ":" + zeroPad(minute(mktime(time_local))) + ":" + zeroPad(second(mktime(time_local)));

  return rtnValue;
}

String TimeDB::cleanText(String text)
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
  text.replace("Ç", "C");
  text.replace("ç", "c");
  text.replace("Ş", "S");
  text.replace("ş", "s");
  text.replace("Ö", "O");
  text.replace("ö", "o");
  text.replace("İ", "I");
  text.replace("ı", "i");
  text.replace("Ü", "U");
  text.replace("ü", "u");
  text.replace("ğ", "g");
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
  return text;
}