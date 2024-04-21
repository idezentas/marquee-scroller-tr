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
}

void OpenWeatherMapClient::updateCityName(String CityName)
{
  myCityName = CityName;
  if (myCityName == "")
  {
    myCityName = "Ankara,TR";
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
    myWorldCityName3 = "Milano,IT";
  }
}

void OpenWeatherMapClient::updateWeatherName(String CityName, int index)
{
  WiFiClient weatherClient;
  if (myApiKey == "")
  {
    weathers[index].error = "Please provide an API key for weather.";
    Serial.println(weathers[index].error);
    return;
  }
  String apiGetData = "GET /data/2.5/weather?q=" + CityName + "&units=" + units + "&cnt=1&appid=" + myApiKey + "&lang=" + myLang + " HTTP/1.1";

  Serial.println("Getting Weather Data");
  Serial.println(apiGetData);
  weathers[index].cached = false;
  weathers[index].error = "";
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
    weathers[index].error = "Connection for weather data failed";
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
    Serial.print("Unexpected response: ");
    Serial.println(status);
    weathers[index].error = "Weather Data Error: " + String(status);
    weatherClient.stop(); // stop client
    return;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!weatherClient.find(endOfHeaders))
  {
    Serial.println("Invalid response");
    weatherClient.stop(); // stop client
    return;
  }

  JsonDocument root;
  DeserializationError error = deserializeJson(root, weatherClient);
  if (error)
  {
    Serial.println("Weather Data Parsing failed!");
    Serial.println(error.c_str());
    weathers[index].error = "Weather Data Parsing failed!" + String(error.c_str());
    weatherClient.stop(); // stop client
    return;
  }

  weatherClient.stop(); // stop client

  JsonObject coord_ = root["coord"];
  weathers[index].lat = coord_["lat"].as<String>();
  weathers[index].lon = coord_["lon"].as<String>();

  JsonObject weather_ = root["weather"][0];
  weathers[index].weatherId = weather_["id"].as<String>();
  weathers[index].condition = weather_["main"].as<String>();
  weathers[index].description = weather_["description"].as<String>();
  weathers[index].icon = weather_["icon"].as<String>();

  JsonObject main_ = root["main"];
  weathers[index].temp = main_["temp"].as<String>();
  weathers[index].feel = main_["feels_like"].as<String>();
  weathers[index].high = main_["temp_max"].as<String>();
  weathers[index].low = main_["temp_min"].as<String>();
  weathers[index].humidity = main_["humidity"].as<String>();
  weathers[index].pressure = main_["pressure"].as<String>();
  weathers[index].seaLevel = main_["sea_level"].as<String>();
  weathers[index].grndLevel = main_["grnd_level"].as<String>();

  JsonObject wind_ = root["wind"];
  weathers[index].wind = wind_["speed"].as<String>();
  weathers[index].gust = wind_["gust"].as<String>();
  weathers[index].direction = wind_["deg"].as<String>();

  JsonObject sys_ = root["sys"];
  weathers[index].country = sys_["country"].as<String>();
  weathers[index].sunRise = sys_["sunrise"].as<String>();
  weathers[index].sunSet = sys_["sunset"].as<String>();

  weathers[index].cloudcover = root["clouds"]["all"].as<String>();
  weathers[index].rain = root["rain"]["1h"].as<String>();
  weathers[index].visibility = root["visibility"].as<String>();
  weathers[index].dt = root["dt"].as<String>();
  weathers[index].id = root["id"].as<String>();
  weathers[index].city = root["name"].as<String>();
  weathers[index].timeZone = root["timezone"].as<String>();

  if (units == "metric")
  {
    // convert to km/h from m/s
    float f = (weathers[index].wind.toFloat() * 3.6);
    weathers[index].wind = String(f);

    float f2 = (weathers[index].gust.toFloat() * 3.6);
    weathers[index].gust = String(f2);
  }

  if (units != "metric")
  {
    float p = (weathers[index].pressure.toFloat() * 0.0295301); // convert millibars to inches
    weathers[index].pressure = String(p);
  }

  Serial.println("lat: " + weathers[index].lat);
  Serial.println("lon: " + weathers[index].lon);
  Serial.println("weatherId: " + weathers[index].weatherId);
  Serial.println("condition: " + weathers[index].condition);
  Serial.println("description: " + weathers[index].description);
  Serial.println("icon: " + weathers[index].icon);
  Serial.println("temp: " + weathers[index].temp);
  Serial.println("feel: " + weathers[index].feel);
  Serial.println("high: " + weathers[index].high);
  Serial.println("low: " + weathers[index].low);
  Serial.println("humidity: " + weathers[index].humidity);
  Serial.println("pressure: " + weathers[index].pressure);
  Serial.println("seaLevel: " + weathers[index].seaLevel);
  Serial.println("grndLevel: " + weathers[index].grndLevel);
  Serial.println("visibility: " + weathers[index].visibility);
  Serial.println("wind: " + weathers[index].wind);
  Serial.println("gust: " + weathers[index].gust);
  Serial.println("direction: " + weathers[index].direction);
  Serial.println("rain: " + weathers[index].rain);
  Serial.println("cloudcover: " + weathers[index].cloudcover);
  Serial.println("dt: " + weathers[index].dt);
  Serial.println("country: " + weathers[index].country);
  Serial.println("sunRise: " + weathers[index].sunRise + " | " + getSunrise(index));
  Serial.println("sunSet: " + weathers[index].sunSet + " | " + getSunset(index));
  Serial.println("sunDifference: " + getSunDifference(index));
  Serial.println("id: " + weathers[index].id);
  Serial.println("city: " + weathers[index].city);
  Serial.println("timezone: " + String(getTimeZone(index)));
  Serial.println();
}

void OpenWeatherMapClient::updateCityAirPollution(String latitude, String longitude, int index)
{
  WiFiClient weatherClient;
  if (myApiKey == "")
  {
    weathers[index].error = "Please provide an API key for weather.";
    Serial.println(weathers[index].error);
    return;
  }

  String apiGetData = "GET /data/2.5/air_pollution?lat=" + latitude + "&lon=" + longitude + "&appid=" + myApiKey + " HTTP/1.1";

  Serial.println("Getting Air Pollution Data for " + latitude + ", " + longitude);
  Serial.println(apiGetData);
  weathers[index].cached = false;
  weathers[index].error = "";
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
    weathers[index].error = "Connection for air pollution data failed";
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
    Serial.print("Unexpected response: ");
    Serial.println(status);
    weathers[index].error = "Air Pollution Data Error: " + String(status);
    weatherClient.stop(); // stop client
    return;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!weatherClient.find(endOfHeaders))
  {
    Serial.println("Invalid response");
    weatherClient.stop(); // stop client
    return;
  }

  JsonDocument root;
  DeserializationError error = deserializeJson(root, weatherClient);
  if (error)
  {
    Serial.println("Air Pollution Data Parsing failed!");
    Serial.println(error.c_str());
    weathers[index].error = "Air Pollution Data Parsing failed!" + String(error.c_str());
    weatherClient.stop(); // stop client
    return;
  }

  weatherClient.stop(); // stop client

  int list_0 = 0;
  weathers[index].aqi = root["list"][list_0]["main"]["aqi"].as<String>();

  JsonObject components_ = root["list"][list_0]["components"];
  weathers[index].co = components_["co"].as<String>();
  weathers[index].no = components_["no"].as<String>();
  weathers[index].no2 = components_["no2"].as<String>();
  weathers[index].o3 = components_["o3"].as<String>();
  weathers[index].so2 = components_["so2"].as<String>();
  weathers[index].pm2_5 = components_["pm2_5"].as<String>();
  weathers[index].pm10 = components_["pm10"].as<String>();
  weathers[index].nh3 = components_["nh3"].as<String>();

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

void OpenWeatherMapClient::updateSunMoonTime(time_t currentTime, String latitude, String longitude, int index)
{
  Serial.print("Getting Sun and Moon Data for ");
  Serial.print(latitude);
  Serial.print(", ");
  Serial.println(longitude);
  double myLatM = latitude.toDouble();
  double myLonM = longitude.toDouble();
  SunMoonCalc smCalc = SunMoonCalc(currentTime, myLatM, myLonM);
  const SunMoonCalc::Result result = smCalc.calculateSunAndMoonData();

  moonStruct[index].sunRise = (String)result.sun.rise;
  moonStruct[index].sunNoon = (String)result.sun.transit;
  moonStruct[index].sunSet = (String)result.sun.set;
  moonStruct[index].sunAzimuth = (String)result.sun.azimuth;
  moonStruct[index].sunElevation = (String)result.sun.elevation;
  moonStruct[index].sunDistance = (String)result.sun.distance;
  moonStruct[index].moonRise = (String)result.moon.rise;
  moonStruct[index].moonNoon = (String)result.moon.transit;
  moonStruct[index].moonSet = (String)result.moon.set;
  moonStruct[index].moonAzimuth = (String)result.moon.azimuth;
  moonStruct[index].moonElevation = (String)result.moon.elevation;
  moonStruct[index].moonDistance = (String)result.moon.distance;
  moonStruct[index].moonAge = (String)result.moon.age;
  moonStruct[index].moonIllumination = (String)(result.moon.illumination * 100);
  moonStruct[index].moonPhase = (String)result.moon.phase.name;
  moonStruct[index].moonBrightLimbAngle = (String)result.moon.brightLimbAngle;
  moonStruct[index].moonAxisPositionAngle = (String)result.moon.axisPositionAngle;
  moonStruct[index].moonParallacticAngle = (String)result.moon.parallacticAngle;

  Serial.println("sunRise: " + moonStruct[index].sunRise);
  Serial.println("sunNoon: " + moonStruct[index].sunNoon);
  Serial.println("sunSet: " + moonStruct[index].sunSet);
  Serial.println("sunAzimuth: " + moonStruct[index].sunAzimuth);
  Serial.println("sunElevation: " + moonStruct[index].sunElevation);
  Serial.println("sunDistance: " + moonStruct[index].sunDistance);
  Serial.println("moonRise: " + moonStruct[index].moonRise + " | " + getMoonRise(index));
  Serial.println("moonNoon: " + moonStruct[index].moonNoon);
  Serial.println("moonSet: " + moonStruct[index].moonSet + " | " + getMoonSet(index));
  Serial.println("moonAzimuth: " + moonStruct[index].moonAzimuth);
  Serial.println("moonElevation: " + moonStruct[index].moonElevation);
  Serial.println("moonDistance: " + moonStruct[index].moonDistance);
  Serial.println("moonAge: " + moonStruct[index].moonAge);
  Serial.println("moonIllumination: " + moonStruct[index].moonIllumination);
  Serial.println("moonPhase: " + moonStruct[index].moonPhase);
  Serial.println("moonBrightLimbAngle: " + moonStruct[index].moonBrightLimbAngle);
  Serial.println("moonAxisPositionAngle: " + moonStruct[index].moonAxisPositionAngle);
  Serial.println("moonParallacticAngle: " + moonStruct[index].moonParallacticAngle);
  Serial.println();
}

time_t OpenWeatherMapClient::getCityTimeStamp(int index)
{
  return weathers[index].dt.toInt();
}

String OpenWeatherMapClient::roundValue(String value)
{
  float f = value.toFloat();
  int rounded = (int)(f + 0.5f);
  return String(rounded);
}

String OpenWeatherMapClient::getMonthNameT(struct tm *time_local)
{
  String monthValue = "";
  switch (month(mktime(time_local)))
  {
  case 1:
    monthValue = monthsArr[(month(mktime(time_local))) - 1];
    break;
  case 2:
    monthValue = monthsArr[(month(mktime(time_local))) - 1];
    break;
  case 3:
    monthValue = monthsArr[(month(mktime(time_local))) - 1];
    break;
  case 4:
    monthValue = monthsArr[(month(mktime(time_local))) - 1];
    break;
  case 5:
    monthValue = monthsArr[(month(mktime(time_local))) - 1];
    break;
  case 6:
    monthValue = monthsArr[(month(mktime(time_local))) - 1];
    break;
  case 7:
    monthValue = monthsArr[(month(mktime(time_local))) - 1];
    break;
  case 8:
    monthValue = monthsArr[(month(mktime(time_local))) - 1];
    break;
  case 9:
    monthValue = monthsArr[(month(mktime(time_local))) - 1];
    break;
  case 10:
    monthValue = monthsArr[(month(mktime(time_local))) - 1];
    break;
  case 11:
    monthValue = monthsArr[(month(mktime(time_local))) - 1];
    break;
  case 12:
    monthValue = monthsArr[(month(mktime(time_local))) - 1];
    break;
  default:
    monthValue = "";
  }
  return monthValue;
}

String OpenWeatherMapClient::getWeekNameT(struct tm *time_local)
{
  String weekdayValue = "";
  switch (weekday(mktime(time_local)))
  {
  case 1:
    weekdayValue = daysArr[(weekday(mktime(time_local))) - 1];
    break;
  case 2:
    weekdayValue = daysArr[(weekday(mktime(time_local))) - 1];
    break;
  case 3:
    weekdayValue = daysArr[(weekday(mktime(time_local))) - 1];
    break;
  case 4:
    weekdayValue = daysArr[(weekday(mktime(time_local))) - 1];
    break;
  case 5:
    weekdayValue = daysArr[(weekday(mktime(time_local))) - 1];
    break;
  case 6:
    weekdayValue = daysArr[(weekday(mktime(time_local))) - 1];
    break;
  case 7:
    weekdayValue = daysArr[(weekday(mktime(time_local))) - 1];
    break;
  default:
    weekdayValue = "";
  }
  return weekdayValue;
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

String OpenWeatherMapClient::getRain(int index)
{
  String rtnValue = "";
  int rainInt = weathers[index].rain.toInt();
  if (rainInt != 0)
  {
    rtnValue = String(rainInt);
    return rtnValue;
  }
  else
  {
    rtnValue = "0";
    return rtnValue;
  }
}

String OpenWeatherMapClient::getVisibility(int index)
{
  return weathers[index].visibility;
}

String OpenWeatherMapClient::getVisibilityOtherUnit(int index)
{
  String rtnValue = "";
  int visiInt = weathers[index].visibility.toInt();
  int visiIntKm = visiInt / 1000;
  rtnValue = String(visiIntKm);
  return rtnValue;
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

String OpenWeatherMapClient::getGust(int index)
{
  String rtnValue = "";
  int gustInt = weathers[index].gust.toInt();
  if (gustInt != 0)
  {
    rtnValue = String(gustInt);
    return rtnValue;
  }
  else
  {
    rtnValue = "0";
    return rtnValue;
  }
}

String OpenWeatherMapClient::getGustRounded(int index)
{
  String rtnValue = "";
  String gustStr = roundValue(getGust(index));
  int gustInt = gustStr.toInt();
  if (gustInt != 0)
  {
    rtnValue = String(gustInt);
    return rtnValue;
  }
  else
  {
    rtnValue = "0";
    return rtnValue;
  }
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
  return dirArr[(val % 16)];
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

boolean OpenWeatherMapClient::getCached(int index)
{
  return weathers[index].cached;
}

String OpenWeatherMapClient::getMyCityName()
{
  return myCityName;
}

String OpenWeatherMapClient::getSeaLevel(int index)
{
  String rtnValue = "";
  int seaInt = weathers[index].seaLevel.toInt();
  if (seaInt != 0)
  {
    rtnValue = String(seaInt);
    return rtnValue;
  }
  else
  {
    rtnValue = "0";
    return rtnValue;
  }
}

String OpenWeatherMapClient::getGrndLevel(int index)
{
  String rtnValue = "";
  int grndInt = weathers[index].grndLevel.toInt();
  if (grndInt != 0)
  {
    rtnValue = String(grndInt);
    return rtnValue;
  }
  else
  {
    rtnValue = "0";
    return rtnValue;
  }
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

String OpenWeatherMapClient::getMoonPhase(int index)
{
  return moonStruct[index].moonPhase;
}

String OpenWeatherMapClient::getMoonIllumination(int index)
{
  return moonStruct[index].moonIllumination;
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
      rtnValue = daysArr[day];
      break;
    case 1:
      rtnValue = daysArr[day];
      break;
    case 2:
      rtnValue = daysArr[day];
      break;
    case 3:
      rtnValue = daysArr[day];
      break;
    case 4:
      rtnValue = daysArr[day];
      break;
    case 5:
      rtnValue = daysArr[day];
      break;
    case 6:
      rtnValue = daysArr[day];
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
      rtnValue = airArr[(aqiN)-1];
      break;
    case 2:
      rtnValue = airArr[(aqiN)-1];
      break;
    case 3:
      rtnValue = airArr[(aqiN)-1];
      break;
    case 4:
      rtnValue = airArr[(aqiN)-1];
      break;
    case 5:
      rtnValue = airArr[(aqiN)-1];
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
  String monthValue = "";
  String weekdayValue = "";
  long epoc = weathers[index].sunRise.toInt();
  time_t epoch_time_as_time_t = epoc;
  struct tm *time_local = localtime(&epoch_time_as_time_t);
  int dstValue = weathers[index].timeZone.toInt();
  if (dstValue != 0)
  {
    dstValue = dstValue / 3600;
  }
  time_local->tm_hour += dstValue;

    monthValue = getMonthNameT(time_local);
  weekdayValue = getWeekNameT(time_local);

  rtnValue = zeroPad(hour(mktime(time_local))) + ":" + zeroPad(minute(mktime(time_local))) + " (" + weekdayValue + ", " + String(day(mktime(time_local))) + " " + monthValue + " " + String(year(mktime(time_local))) + ")";
  return rtnValue;
}

String OpenWeatherMapClient::getSunset(int index)
{
  String rtnValue = "";
  String monthValue = "";
  String weekdayValue = "";
  long epoc = weathers[index].sunSet.toInt();
  time_t epoch_time_as_time_t = epoc;
  struct tm *time_local = localtime(&epoch_time_as_time_t);
  int dstValue = weathers[index].timeZone.toInt();
  if (dstValue != 0)
  {
    dstValue = dstValue / 3600;
  }
  time_local->tm_hour += dstValue;

    monthValue = getMonthNameT(time_local);
  weekdayValue = getWeekNameT(time_local);

  rtnValue = zeroPad(hour(mktime(time_local))) + ":" + zeroPad(minute(mktime(time_local))) + " (" + weekdayValue + ", " + String(day(mktime(time_local))) + " " + monthValue + " " + String(year(mktime(time_local))) + ")";
  return rtnValue;
}

String OpenWeatherMapClient::getSunDifference(int index)
{
  String rtnValue = "";
  long sunRise = weathers[index].sunRise.toInt();
  long sunSet = weathers[index].sunSet.toInt();
  time_t sunRiseTime = sunRise;
  time_t sunSetTime = sunSet;
  long epoc = sunSetTime - sunRiseTime;
  time_t epoch_time_as_time_t = epoc;
  struct tm *time_local = localtime(&epoch_time_as_time_t);
  rtnValue = zeroPad(hour(mktime(time_local))) + ":" + zeroPad(minute(mktime(time_local)));
  return rtnValue;
}

String OpenWeatherMapClient::getMoonRise(int index)
{
  String rtnValue = "";
  String monthValue = "";
  String weekdayValue = "";
  long epoc = moonStruct[index].moonRise.toInt();
  time_t epoch_time_as_time_t = epoc;
  struct tm *time_local = localtime(&epoch_time_as_time_t);
  int dstValue = weathers[index].timeZone.toInt();
  if (dstValue != 0)
  {
    dstValue = dstValue / 3600;
  }
  time_local->tm_hour += dstValue;

    monthValue = getMonthNameT(time_local);
  weekdayValue = getWeekNameT(time_local);

  rtnValue = zeroPad(hour(mktime(time_local))) + ":" + zeroPad(minute(mktime(time_local))) + " (" + weekdayValue + ", " + String(day(mktime(time_local))) + " " + monthValue + " " + String(year(mktime(time_local))) + ")";
  return rtnValue;
}

String OpenWeatherMapClient::getMoonSet(int index)
{
  String rtnValue = "";
  String monthValue = "";
  String weekdayValue = "";
  long epoc = moonStruct[index].moonSet.toInt();
  time_t epoch_time_as_time_t = epoc;
  struct tm *time_local = localtime(&epoch_time_as_time_t);
  int dstValue = weathers[index].timeZone.toInt();
  if (dstValue != 0)
  {
    dstValue = dstValue / 3600;
  }
  time_local->tm_hour += dstValue;

    monthValue = getMonthNameT(time_local);
  weekdayValue = getWeekNameT(time_local);

  rtnValue = zeroPad(hour(mktime(time_local))) + ":" + zeroPad(minute(mktime(time_local))) + " (" + weekdayValue + ", " + String(day(mktime(time_local))) + " " + monthValue + " " + String(year(mktime(time_local))) + ")";
  return rtnValue;
}

String OpenWeatherMapClient::getMoonDifference(int index)
{
  String rtnValue = "";
  long moonRise = moonStruct[index].moonRise.toInt();
  long moonSet = moonStruct[index].moonSet.toInt();
  time_t moonRiseTime = moonRise;
  time_t moonSetTime = moonSet;
  long epoc = moonSetTime - moonRiseTime;
  time_t epoch_time_as_time_t = epoc;
  struct tm *time_local = localtime(&epoch_time_as_time_t);
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
  text.replace("ó", "o");
  text.replace("ò", "o");
  text.replace("Ó", "O");
  text.replace("Ò", "O");
  text.replace("°", (String) char(247));
  return text;
}