#include <algorithm>
/** The MIT License (MIT)

Copyright (c) 2018 David Payne

Air Quality and Name Search added by idezentas

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

#include "OpenWeatherMapClient.h"
#include "math.h"

OpenWeatherMapClient::OpenWeatherMapClient(String ApiKey, boolean isMetric, String CityName, String language)
{
  myApiKey = ApiKey;
  setMetric(isMetric);
  updateCityName(CityName);
  updateLanguage(language);
}

void OpenWeatherMapClient::updateWeatherApiKey(String ApiKey)
{
  myApiKey = ApiKey;
}

void OpenWeatherMapClient::updateLanguage(String language)
{
  myLang = language;
  if (myLang == "")
  {
    myLang = "en";
  }
}

void OpenWeatherMapClient::updateCityName(String CityName)
{
  myCityName = CityName;
  if (myCityName == "")
  {
    myCityName = "Mesa,US";
  }
}

void OpenWeatherMapClient::updateWorldCityName1(String WorldCityName1)
{
  myWorldCityName1 = WorldCityName1;
  if (myWorldCityName1 == "")
  {
    myWorldCityName1 = "London,GB";
  }
}

void OpenWeatherMapClient::updateWorldCityName2(String WorldCityName2)
{
  myWorldCityName2 = WorldCityName2;
  if (myWorldCityName2 == "")
  {
    myWorldCityName2 = "Los Angeles,US";
  }
}

void OpenWeatherMapClient::updateWorldCityName3(String WorldCityName3)
{
  myWorldCityName3 = WorldCityName3;
  if (myWorldCityName3 == "")
  {
    myWorldCityName3 = "Chicago,US";
  }
}

void OpenWeatherMapClient::updateWorldCityName4(String WorldCityName4)
{
  myWorldCityName4 = WorldCityName4;
  if (myWorldCityName4 == "")
  {
    myWorldCityName4 = "Miami,US";
  }
}

void OpenWeatherMapClient::updateWorldCityName5(String WorldCityName5)
{
  myWorldCityName5 = WorldCityName5;
  if (myWorldCityName5 == "")
  {
    myWorldCityName5 = "Milano,IT";
  }
}

void OpenWeatherMapClient::updateWeatherName(String CityName, int index)
{
  WiFiClient weatherClient;
  if (myApiKey == "")
  {
    weathers[0].error = "Please provide an API key for weather.";
    Serial.println(weathers[0].error);
    return;
  }
  String apiGetData = "GET /data/2.5/weather?q=" + CityName + "&units=" + units + "&cnt=1&appid=" + myApiKey + "&lang=" + myLang + " HTTP/1.1";

  Serial.println("Getting Weather Data");
  Serial.println(apiGetData);
  weathers[0].cached = false;
  weathers[0].error = "";
  if (weatherClient.connect(servername, 80))
  { // starts client connection, checks for connection
    weatherClient.println(apiGetData);
    weatherClient.println("Host: " + String(servername));
    weatherClient.println("User-Agent: ArduinoWiFi/1.1");
    weatherClient.println("Connection: close");
    weatherClient.println();
  }
  else
  {
    Serial.println("Connection for weather data failed"); // error message if no client connect
    Serial.println();
    weathers[0].error = "Connection for weather data failed";
    return;
  }

  while (weatherClient.connected() && !weatherClient.available())
    delay(1); // waits for data

  Serial.println("Waiting for data");

  // Check HTTP status
  char status[32] = {0};
  weatherClient.readBytesUntil('\r', status, sizeof(status));
  Serial.println("Response Header: " + String(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0)
  {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    weathers[0].error = "Weather Data Error: " + String(status);
    return;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!weatherClient.find(endOfHeaders))
  {
    Serial.println(F("Invalid response"));
    return;
  }

  const size_t bufferSize = 1024;
  DynamicJsonDocument root(bufferSize);
  DeserializationError error = deserializeJson(root, weatherClient);
  if (error)
  {
    Serial.println(F("Weather Data Parsing failed!"));
    weathers[0].error = "Weather Data Parsing failed!";
    return;
  }

  weatherClient.stop(); // stop client

  size_t msrLen = bufferSize / 5;

  if (measureJson(root) <= msrLen)
  {
    Serial.println("Error Does not look like we got the data.  Size: " + String(measureJson(root)));
    weathers[0].cached = true;
    weathers[0].error = root["message"].as<String>();
    Serial.println("Error: " + weathers[0].error);
    return;
  }

  weathers[index].lat = root["coord"]["lat"].as<String>();
  weathers[index].lon = root["coord"]["lon"].as<String>();
  weathers[index].weatherId = root["weather"][0]["id"].as<String>();
  weathers[index].condition = root["weather"][0]["main"].as<String>();
  weathers[index].description = root["weather"][0]["description"].as<String>();
  weathers[index].icon = root["weather"][0]["icon"].as<String>();
  weathers[index].temp = root["main"]["temp"].as<String>();
  weathers[index].feel = root["main"]["feels_like"].as<String>();
  weathers[index].high = root["main"]["temp_max"].as<String>();
  weathers[index].low = root["main"]["temp_min"].as<String>();
  weathers[index].humidity = root["main"]["humidity"].as<String>();
  weathers[index].pressure = root["main"]["pressure"].as<String>();
  weathers[index].wind = root["wind"]["speed"].as<String>();
  weathers[index].direction = root["wind"]["deg"].as<String>();
  weathers[index].cloudcover = root["clouds"]["all"].as<String>();
  weathers[index].dt = root["dt"].as<String>();
  weathers[index].country = root["sys"]["country"].as<String>();
  weathers[index].sunRise = root["sys"]["sunrise"].as<String>();
  weathers[index].sunSet = root["sys"]["sunset"].as<String>();
  weathers[index].id = root["id"].as<String>();
  weathers[index].city = root["name"].as<String>();
  weathers[index].timeZone = root["timezone"].as<String>();

  if (units == "metric")
  {
    // convert to km/h from m/s
    float f = (weathers[index].wind.toFloat() * 3.6);
    weathers[index].wind = String(f);
  }

  if (units != "metric")
  {
    float p = (weathers[index].pressure.toFloat() * 0.0295301); // convert millibars to inches
    weathers[index].pressure = String(p);
  }

  Serial.println("lat: " + weathers[index].lat);
  Serial.println("lon: " + weathers[index].lon);
  Serial.println("dt: " + weathers[index].dt);
  Serial.println("id: " + weathers[index].id);
  Serial.println("city: " + weathers[index].city);
  Serial.println("country: " + weathers[index].country);
  Serial.println("temp: " + weathers[index].temp);
  Serial.println("feel: " + weathers[index].feel);
  Serial.println("high: " + weathers[index].high);
  Serial.println("low: " + weathers[index].low);
  Serial.println("cloudcover: " + weathers[index].cloudcover);
  Serial.println("humidity: " + weathers[index].humidity);
  Serial.println("condition: " + weathers[index].condition);
  Serial.println("wind: " + weathers[index].wind);
  Serial.println("direction: " + weathers[index].direction);
  Serial.println("weatherId: " + weathers[index].weatherId);
  Serial.println("description: " + weathers[index].description);
  Serial.println("icon: " + weathers[index].icon);
  Serial.println("timezone: " + String(getTimeZone(index)));
  Serial.println("sunRise: " + getSunrise(index));
  Serial.println("sunSet: " + getSunset(index));
  Serial.println();
}

void OpenWeatherMapClient::updateCityAirPollution(String latitude, String longitude, int index)
{
  WiFiClient weatherClient;
  if (myApiKey == "")
  {
    weathers[0].error = "Please provide an API key for weather.";
    Serial.println(weathers[0].error);
    return;
  }

  String apiGetData = "GET /data/2.5/air_pollution?lat=" + latitude + "&lon=" + longitude + "&appid=" + myApiKey + " HTTP/1.1";

  Serial.println("Getting Air Pollution Data for " + latitude + ", " + longitude);
  Serial.println(apiGetData);
  weathers[0].cached = false;
  weathers[0].error = "";
  if (weatherClient.connect(servername, 80))
  { // starts client connection, checks for connection
    weatherClient.println(apiGetData);
    weatherClient.println("Host: " + String(servername));
    weatherClient.println("User-Agent: ArduinoWiFi/1.1");
    weatherClient.println("Connection: close");
    weatherClient.println();
  }
  else
  {
    Serial.println("Connection for air pollution data failed"); // error message if no client connect
    Serial.println();
    weathers[0].error = "Connection for air pollution data failed";
    return;
  }

  while (weatherClient.connected() && !weatherClient.available())
    delay(1); // waits for data

  Serial.println("Waiting for data");

  // Check HTTP status
  char status[32] = {0};
  weatherClient.readBytesUntil('\r', status, sizeof(status));
  Serial.println("Response Header: " + String(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0)
  {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    weathers[0].error = "Air Pollution Data Error: " + String(status);
    return;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!weatherClient.find(endOfHeaders))
  {
    Serial.println(F("Invalid response"));
    return;
  }

  const size_t bufferSize = 384;
  DynamicJsonDocument root(bufferSize);
  DeserializationError error = deserializeJson(root, weatherClient);
  if (error)
  {
    Serial.println(F("Air Pollution Data Parsing failed!"));
    weathers[0].error = "Air Pollution Data Parsing failed!";
    return;
  }

  weatherClient.stop(); // stop client

  size_t msrLen = bufferSize / 5;

  if (measureJson(root) <= msrLen)
  {
    Serial.println("Error Does not look like we got the data.  Size: " + String(measureJson(root)));
    weathers[0].cached = true;
    weathers[0].error = root["message"].as<String>();
    Serial.println("Error: " + weathers[0].error);
    return;
  }

  int list_0 = 0;
  weathers[index].aqi = root["list"][list_0]["main"]["aqi"].as<String>();
  weathers[index].co = root["list"][list_0]["components"]["co"].as<String>();
  weathers[index].no = root["list"][list_0]["components"]["no"].as<String>();
  weathers[index].no2 = root["list"][list_0]["components"]["no2"].as<String>();
  weathers[index].o3 = root["list"][list_0]["components"]["o3"].as<String>();
  weathers[index].so2 = root["list"][list_0]["components"]["so2"].as<String>();
  weathers[index].pm2_5 = root["list"][list_0]["components"]["pm2_5"].as<String>();
  weathers[index].pm10 = root["list"][list_0]["components"]["pm10"].as<String>();
  weathers[index].nh3 = root["list"][list_0]["components"]["nh3"].as<String>();

  Serial.println("aqi: " + weathers[index].aqi);
  Serial.println("co: " + weathers[index].co);
  Serial.println("no: " + weathers[index].no);
  Serial.println("o3: " + weathers[index].o3);
  Serial.println("so2: " + weathers[index].so2);
  Serial.println("pm2_5: " + weathers[index].pm2_5);
  Serial.println("pm10: " + weathers[index].pm10);
  Serial.println("nh3: " + weathers[index].nh3);
  Serial.println();
}

String OpenWeatherMapClient::roundValue(String value)
{
  float f = value.toFloat();
  int rounded = (int)(f + 0.5f);
  return String(rounded);
}

void OpenWeatherMapClient::setMetric(boolean isMetric)
{
  if (isMetric)
  {
    units = "metric";
  }
  else
  {
    units = "imperial";
  }
}

String OpenWeatherMapClient::getLat(int index)
{
  return weathers[index].lat;
}

String OpenWeatherMapClient::getLon(int index)
{
  return weathers[index].lon;
}

String OpenWeatherMapClient::getDt(int index)
{
  return weathers[index].dt;
}

String OpenWeatherMapClient::getID(int index)
{
  return weathers[index].id;
}

String OpenWeatherMapClient::getCity(int index)
{
  return weathers[index].city;
}

String OpenWeatherMapClient::getCountry(int index)
{
  return weathers[index].country;
}

String OpenWeatherMapClient::getTemp(int index)
{
  return weathers[index].temp;
}

String OpenWeatherMapClient::getFeel(int index)
{
  return weathers[index].feel;
}

String OpenWeatherMapClient::getFeelRounded(int index)
{
  return roundValue(weathers[index].feel);
}

String OpenWeatherMapClient::getCloudcover(int index)
{
  return roundValue(weathers[index].cloudcover);
}

String OpenWeatherMapClient::getTempRounded(int index)
{
  return roundValue(getTemp(index));
}

String OpenWeatherMapClient::getHumidity(int index)
{
  return weathers[index].humidity;
}

String OpenWeatherMapClient::getHumidityRounded(int index)
{
  return roundValue(getHumidity(index));
}

String OpenWeatherMapClient::getCondition(int index)
{
  return weathers[index].condition;
}

String OpenWeatherMapClient::getWind(int index)
{
  return weathers[index].wind;
}

String OpenWeatherMapClient::getWindRounded(int index)
{
  return roundValue(getWind(index));
}

String OpenWeatherMapClient::getDirection(int index)
{
  return weathers[index].direction;
}

String OpenWeatherMapClient::getDirectionRounded(int index)
{
  return roundValue(getDirection(index));
}

String OpenWeatherMapClient::getDirectionText(int index)
{
  int num = getDirectionRounded(index).toInt();
  int val = floor((num / 22.5) + 0.5);
  String arr[] = {"K", "KKD", "KD", "DKD", "D", "DGD", "GD", "GGD", "G", "GGB", "GB", "BGB", "B", "BKB", "KB", "KKB"};
  return arr[(val % 16)];
}

String OpenWeatherMapClient::getWeatherId(int index)
{
  return weathers[index].weatherId;
}

String OpenWeatherMapClient::getDescription(int index)
{
  return weathers[index].description;
}

String OpenWeatherMapClient::getPressure(int index)
{
  return weathers[index].pressure;
}

String OpenWeatherMapClient::getHigh(int index)
{
  return weathers[index].high;
}

String OpenWeatherMapClient::getHighRounded(int index)
{
  return roundValue(weathers[index].high);
}

String OpenWeatherMapClient::getLow(int index)
{
  return weathers[index].low;
}

String OpenWeatherMapClient::getLowRounded(int index)
{
  return roundValue(weathers[index].low);
}

String OpenWeatherMapClient::getIcon(int index)
{
  return weathers[index].icon;
}

boolean OpenWeatherMapClient::getCached()
{
  return weathers[0].cached;
}

String OpenWeatherMapClient::getMyCityName()
{
  return myCityName;
}

String OpenWeatherMapClient::getCO(int index)
{
  return weathers[index].co;
}

String OpenWeatherMapClient::getNO(int index)
{
  return weathers[index].no;
}

String OpenWeatherMapClient::getNO2(int index)
{
  return weathers[index].no2;
}

String OpenWeatherMapClient::getO3(int index)
{
  return weathers[index].o3;
}

String OpenWeatherMapClient::getSO2(int index)
{
  return weathers[index].so2;
}

String OpenWeatherMapClient::getPM10(int index)
{
  return weathers[index].pm10;
}

String OpenWeatherMapClient::getPM2_5(int index)
{
  return weathers[index].pm2_5;
}

String OpenWeatherMapClient::getNH3(int index)
{
  return weathers[index].nh3;
}

String OpenWeatherMapClient::getError(int index)
{
  return weathers[index].error;
}

String OpenWeatherMapClient::getWeekDay(int index, float offset)
{
  String rtnValue = "";
  long epoc = weathers[index].dt.toInt();
  long day = 0;
  if (epoc != 0)
  {
    day = (((epoc + (3600 * (int)offset)) / 86400) + 4) % 7;
    switch (day)
    {
    case 0:
      rtnValue = "Pazar";
      break;
    case 1:
      rtnValue = "Pazartesi";
      break;
    case 2:
      rtnValue = "Salı";
      break;
    case 3:
      rtnValue = "Çarşamba";
      break;
    case 4:
      rtnValue = "Perşembe";
      break;
    case 5:
      rtnValue = "Cuma";
      break;
    case 6:
      rtnValue = "Cumartesi";
      break;
    default:
      break;
    }
  }
  return rtnValue;
}

String OpenWeatherMapClient::getAqi(int index)
{
  String rtnValue = "";
  int aqiN = weathers[index].aqi.toInt();
  if (aqiN != 0)
  {
    switch (aqiN)
    {
    case 1:
      rtnValue = "İyi";
      break;
    case 2:
      rtnValue = "İdare Eder";
      break;
    case 3:
      rtnValue = "Orta";
      break;
    case 4:
      rtnValue = "Kötü";
      break;
    case 5:
      rtnValue = "Çok Kötü";
      break;
    default:
      break;
    }
  }
  return rtnValue;
}

String OpenWeatherMapClient::getSunrise(int index)
{
  String rtnValue = "";
  long epoc = weathers[index].sunRise.toInt();
  time_t epoch_time_as_time_t = epoc;
  struct tm *time_local = localtime(&epoch_time_as_time_t);
  int dstValue = weathers[index].timeZone.toInt();
  if (dstValue != 0)
  {
    dstValue = dstValue / 3600;
  }
  time_local->tm_hour += dstValue;
  rtnValue = zeroPad(hour(mktime(time_local))) + ":" + zeroPad(minute(mktime(time_local)));
  return rtnValue;
}

String OpenWeatherMapClient::getSunset(int index)
{
  String rtnValue = "";
  long epoc = weathers[index].sunSet.toInt();
  time_t epoch_time_as_time_t = epoc;
  struct tm *time_local = localtime(&epoch_time_as_time_t);
  int dstValue = weathers[index].timeZone.toInt();
  if (dstValue != 0)
  {
    dstValue = dstValue / 3600;
  }
  time_local->tm_hour += dstValue;
  rtnValue = zeroPad(hour(mktime(time_local))) + ":" + zeroPad(minute(mktime(time_local)));
  return rtnValue;
}

String OpenWeatherMapClient::zeroPad(int number)
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

int OpenWeatherMapClient::getTimeZone(int index)
{
  int rtnValue = weathers[index].timeZone.toInt();
  if (rtnValue != 0)
  {
    rtnValue = rtnValue / 3600;
  }
  return rtnValue;
}

String OpenWeatherMapClient::getWeatherIcon(int index)
{
  int id = getWeatherId(index).toInt();
  String W = ")";
  switch (id)
  {
  case 800:
    W = "B";
    break;
  case 801:
    W = "Y";
    break;
  case 802:
    W = "H";
    break;
  case 803:
    W = "H";
    break;
  case 804:
    W = "Y";
    break;

  case 200:
    W = "0";
    break;
  case 201:
    W = "0";
    break;
  case 202:
    W = "0";
    break;
  case 210:
    W = "0";
    break;
  case 211:
    W = "0";
    break;
  case 212:
    W = "0";
    break;
  case 221:
    W = "0";
    break;
  case 230:
    W = "0";
    break;
  case 231:
    W = "0";
    break;
  case 232:
    W = "0";
    break;

  case 300:
    W = "R";
    break;
  case 301:
    W = "R";
    break;
  case 302:
    W = "R";
    break;
  case 310:
    W = "R";
    break;
  case 311:
    W = "R";
    break;
  case 312:
    W = "R";
    break;
  case 313:
    W = "R";
    break;
  case 314:
    W = "R";
    break;
  case 321:
    W = "R";
    break;

  case 500:
    W = "R";
    break;
  case 501:
    W = "R";
    break;
  case 502:
    W = "R";
    break;
  case 503:
    W = "R";
    break;
  case 504:
    W = "R";
    break;
  case 511:
    W = "R";
    break;
  case 520:
    W = "R";
    break;
  case 521:
    W = "R";
    break;
  case 522:
    W = "R";
    break;
  case 531:
    W = "R";
    break;

  case 600:
    W = "W";
    break;
  case 601:
    W = "W";
    break;
  case 602:
    W = "W";
    break;
  case 611:
    W = "W";
    break;
  case 612:
    W = "W";
    break;
  case 615:
    W = "W";
    break;
  case 616:
    W = "W";
    break;
  case 620:
    W = "W";
    break;
  case 621:
    W = "W";
    break;
  case 622:
    W = "W";
    break;

  case 701:
    W = "M";
    break;
  case 711:
    W = "M";
    break;
  case 721:
    W = "M";
    break;
  case 731:
    W = "M";
    break;
  case 741:
    W = "M";
    break;
  case 751:
    W = "M";
    break;
  case 761:
    W = "M";
    break;
  case 762:
    W = "M";
    break;
  case 771:
    W = "M";
    break;
  case 781:
    W = "M";
    break;

  default:
    break;
  }
  return W;
}

String OpenWeatherMapClient::cleanText(String text)
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