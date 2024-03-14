/** The MIT License (MIT)

Copyright (c) 2018 David Payne

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

#include "PiHoleClient.h"

PiHoleClient::PiHoleClient()
{
  // Constructor
}

void PiHoleClient::getPiHoleData(String server, int port, String apiKey)
{

  WiFiClient wifiClient;
  errorMessage = "";
  String response = "";

  if (apiKey == "")
  {
    errorMessage = "Pi-hole API Key is required to view Summary Data.";
    Serial.println(errorMessage);
    return;
  }

  String apiGetData = "http://" + server + ":" + String(port) + "/admin/api.php?summary&auth=" + apiKey;
  Serial.println("Sending: " + apiGetData);
  HTTPClient http;                    // Object of class HTTPClient
  http.begin(wifiClient, apiGetData); // get the result
  int httpCode = http.GET();
  // Check the returning code
  if (httpCode > 0)
  {
    response = http.getString();
    http.end(); // Close connection
    if (httpCode != 200)
    {
      // Bad Response Code
      errorMessage = "Error response (" + String(httpCode) + "): " + response;
      Serial.println(errorMessage);
      return;
    }
    Serial.println("Response Code: " + String(httpCode));
    // Serial.println("Response: " + response);
  }
  else
  {
    errorMessage = "Failed to connect and get data: " + apiGetData;
    Serial.println(errorMessage);
    return;
  }

  JsonDocument root;
  DeserializationError error = deserializeJson(root, response);
  if (error)
  {
    Serial.println(F("Data Summary Parsing failed!"));
    errorMessage = "Data Summary Parsing failed: ";
    return;
  }

  piHoleData.domains_being_blocked = root["domains_being_blocked"].as<String>();
  piHoleData.dns_queries_today = root["dns_queries_today"].as<String>();
  piHoleData.ads_blocked_today = root["ads_blocked_today"].as<String>();
  piHoleData.ads_percentage_today = root["ads_percentage_today"].as<String>();
  piHoleData.unique_domains = root["unique_domains"].as<String>();
  piHoleData.queries_forwarded = root["queries_forwarded"].as<String>();
  piHoleData.queries_cached = root["queries_cached"].as<String>();
  piHoleData.clients_ever_seen = root["clients_ever_seen"].as<String>();
  piHoleData.unique_clients = root["unique_clients"].as<String>();
  piHoleData.dns_queries_all_types = root["dns_queries_all_types"].as<String>();
  piHoleData.reply_NODATA = root["reply_NODATA"].as<String>();
  piHoleData.reply_NXDOMAIN = root["reply_NXDOMAIN"].as<String>();
  piHoleData.reply_CNAME = root["reply_CNAME"].as<String>();
  piHoleData.reply_IP = root["reply_IP"].as<String>();
  piHoleData.privacy_level = root["privacy_level"].as<String>();
  piHoleData.piHoleStatus = root["status"].as<String>();

  Serial.println("Pi-Hole Status: " + piHoleData.piHoleStatus);
  Serial.println("Todays Percentage Blocked: " + piHoleData.ads_percentage_today);
  Serial.println();
}

void PiHoleClient::getTopClientsBlocked(String server, int port, String apiKey)
{
  WiFiClient wifiClient;
  errorMessage = "";
  resetClientsBlocked();

  if (apiKey == "")
  {
    errorMessage = "Pi-hole API Key is required to view Top Clients Blocked.";
    Serial.println(errorMessage);
    return;
  }

  String response = "";

  String apiGetData = "http://" + server + ":" + String(port) + "/admin/api.php?topClientsBlocked=3&auth=" + apiKey;
  Serial.println("Sending: " + apiGetData);
  HTTPClient http;                    // Object of class HTTPClient
  http.begin(wifiClient, apiGetData); // get the result
  int httpCode = http.GET();
  // Check the returning code
  if (httpCode > 0)
  {
    response = http.getString();
    http.end(); // Close connection
    if (httpCode != 200)
    {
      // Bad Response Code
      errorMessage = "Error response (" + String(httpCode) + "): " + response;
      Serial.println(errorMessage);
      return;
    }
    Serial.println("Response Code: " + String(httpCode));
    // Serial.println("Response: " + response);
  }
  else
  {
    errorMessage = "Failed to get data: " + apiGetData;
    Serial.println(errorMessage);
    return;
  }

  JsonDocument root;
  DeserializationError error = deserializeJson(root, response);
  if (error)
  {
    errorMessage = "Data Parsing failed -- verify your Pi-hole API key.";
    Serial.println(errorMessage);
    return;
  }

  JsonObject blocked = root["top_sources_blocked"];

  int count = 0;
  for (JsonPair p : blocked)
  {
    blockedClients[count].clientAddress = p.key().c_str();
    blockedClients[count].blockedCount = p.value().as<int>();
    Serial.println("Blocked Client " + String(count + 1) + ": " + blockedClients[count].clientAddress + " (" + String(blockedClients[count].blockedCount) + ")");
    count++;
  }
  Serial.println();
}

void PiHoleClient::getGraphData(String server, int port, String apiKey)
{
  WiFiClient wifiClient;
  HTTPClient http;

  errorMessage = "";

  if (apiKey == "")
  {
    errorMessage = "Pi-hole API Key is required to view Graph Data.";
    Serial.println(errorMessage);
    return;
  }

  String apiGetData = "http://" + server + ":" + String(port) + "/admin/api.php?overTimeData10mins&auth=" + apiKey;
  resetBlockedGraphData();
  Serial.println("Getting Pi-Hole Graph Data");
  Serial.println(apiGetData);
  http.begin(wifiClient, apiGetData);
  int httpCode = http.GET();

  String result = "";
  boolean track = false;
  int countBracket = 0;
  blockedCount = 0;

  if (httpCode > 0)
  { // checks for connection
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    if (httpCode == HTTP_CODE_OK)
    {
      // get length of document (is -1 when Server sends no Content-Length header)
      int len = http.getSize();
      // create buffer for read
      char buff[128] = {0};
      // get tcp stream
      WiFiClient *stream = http.getStreamPtr();
      // read all data from server
      Serial.println("Start reading...");
      while (http.connected() && (len > 0 || len == -1))
      {
        // get available data size
        size_t size = stream->available();
        if (size)
        {
          // read up to 128 byte
          int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
          for (int i = 0; i < c; i++)
          {
            if (track && countBracket >= 3)
            {
              if ((buff[i] == ',' || buff[i] == '}') && (blockedCount < 144))
              {
                blocked[blockedCount] = result.toInt();
                if (blocked[blockedCount] > blockedHigh)
                {
                  blockedHigh = blocked[blockedCount];
                }
                // Serial.println("Pi-hole Graph point (" + String(blockedCount+1) + "): " + String(blocked[blockedCount]));
                blockedCount++;
                result = "";
                track = false;
              }
              else
              {
                result += buff[i];
              }
            }
            else if (buff[i] == '{')
            {
              countBracket++;
            }
            else if (countBracket >= 3 && buff[i] == ':')
            {
              track = true;
            }
          }

          if (len > 0)
            len -= c;
        }
        delay(1);
      }
    }
    http.end();
  }
  else
  {
    errorMessage = "Connection for Pi-Hole data failed: " + String(apiGetData);
    Serial.println(errorMessage); // error message if no client connect
    Serial.println();
    return;
  }

  Serial.println("High Value: " + String(blockedHigh));
  Serial.println("Count: " + String(blockedCount));
  Serial.println();
}

void PiHoleClient::resetClientsBlocked()
{
  for (int inx = 0; inx < 3; inx++)
  {
    blockedClients[inx].clientAddress = "";
    blockedClients[inx].blockedCount = 0;
  }
}

void PiHoleClient::resetBlockedGraphData()
{
  for (int inx = 0; inx < 144; inx++)
  {
    blocked[inx] = 0;
  }
  blockedCount = 0;
  blockedHigh = 0;
}

String PiHoleClient::getDomainsBeingBlocked()
{
  return piHoleData.domains_being_blocked;
}

String PiHoleClient::getDnsQueriesToday()
{
  return piHoleData.dns_queries_today;
}

String PiHoleClient::getAdsBlockedToday()
{
  return piHoleData.ads_blocked_today;
}

String PiHoleClient::getAdsPercentageToday()
{
  return piHoleData.ads_percentage_today;
}

String PiHoleClient::getUniqueClients()
{
  return piHoleData.unique_clients;
}

String PiHoleClient::getClientsEverSeen()
{
  return piHoleData.clients_ever_seen;
}

/* //Need to add the following
  String getUniqueDomains();
  String getQueriesForwarded();
  String getQueriesCached();
  String getDnsQueriesAllTypes();
  String getReplyNODATA();
  String getReplyNXDOMAIN();
  String getReplyCNAME();
  String getReplyIP();
  String getPrivacyLevel();
 */

String PiHoleClient::getPiHoleStatus()
{
  return piHoleData.piHoleStatus;
}

String PiHoleClient::getError()
{
  return errorMessage;
}

int *PiHoleClient::getBlockedAds()
{
  return blocked;
}

int PiHoleClient::getBlockedCount()
{
  return blockedCount;
}

int PiHoleClient::getBlockedHigh()
{
  return blockedHigh;
}

String PiHoleClient::getTopClientBlocked(int index)
{
  return blockedClients[index].clientAddress;
}

int PiHoleClient::getTopClientBlockedCount(int index)
{
  return blockedClients[index].blockedCount;
}
