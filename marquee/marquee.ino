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

/**********************************************
  Edit Settings.h for personalization
***********************************************/

#include "Settings.h"

#define CONFIG "/conf.txt"

// declairing prototypes
void configModeCallback(WiFiManager *myWiFiManager);
int8_t getWifiQuality();

// Display Strings
String VERSION = "3.03-TR";
String HOSTNAME = "SAAT-";
String message = "SELAM";
String wifiQualityS = "SINYAL GUCU: %";
String webDisableS = "Web Arayuzu Devre Disi Birakildi";

// LED Settings
int refresh = 0;
int spacer = 1;         // dots between letters
int width = 5 + spacer; // The font width is 5 pixels + spacer
Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);
String Wide_Clock_Style = "2"; // 1="hh:mm Temp", 2="hh:mm:ss", 3="hh:mm"

// TimeDB
TimeDB TimeDBClient("");
String lastMinute = "xx";
int displayRefreshCount = 1;
long lastEpoch = 0;
long firstEpoch = 0;
long displayOffEpoch = 0;
boolean displayOn = true;

TimeDB timezoneClient("");

// Weather Client
OpenWeatherMapClient weatherClient(APIKEY, IS_METRIC, CityName, WeatherLanguage);
OpenWeatherMapClient worldWeatherClient(APIKEY, IS_METRIC, CityName, WeatherLanguage);

// Prayers Time Client
PrayersClient prayersClient(prayersMethod);

// Currency Client
CurrencyConverterClient currencyClient(BaseCurrency1, BaseCurrency2, TargetCurrency);

// OctoPrint Client
OctoPrintClient printerClient(OctoPrintApiKey, OctoPrintServer, OctoPrintPort, OctoAuthUser, OctoAuthPass);
int printerCount = 0;

// Pi-hole Client
PiHoleClient piholeClient;

ESP8266WebServer server(WEBSERVER_PORT);
ESP8266HTTPUpdateServer serverUpdater;

Adafruit_BMP085 bmp180;

static const char WEB_ACTIONS1[] PROGMEM = "<a class='w3-bar-item w3-button' href='/'><i class='fas fa-home'></i> Anasayfa</a>"
                                           "<a class='w3-bar-item w3-button' href='/displayworldclock'><i class='fas fa-clock'></i> Dünya Saatleri Verileri</a>"
                                           "<a class='w3-bar-item w3-button' href='/displayworldclockweather'><i class='fas fa-clock'></i> Dünya Şehirleri Hava Durumu Verileri</a>"
                                           "<a class='w3-bar-item w3-button' href='/displayprayerstime'><i class='fas fa-mosque'></i> Namaz Vakitleri Verileri</a>"
                                           "<a class='w3-bar-item w3-button' href='/displaycurrency'><i class='fas fa-money-bill'></i> Döviz Kurları Verileri</a>"
                                           "<a class='w3-bar-item w3-button' href='/configure'><i class='fas fa-cog'></i> Ayarlar</a>"
                                           "<a class='w3-bar-item w3-button' href='/configurematrix'><i class='fas fa-cog'></i> Ekran Ayarları</a>"
                                           "<a class='w3-bar-item w3-button' href='/configureprayers'><i class='fas fa-mosque'></i> Namaz Vakitleri Ayarları</a>"
                                           "<a class='w3-bar-item w3-button' href='/configureworldclock'><i class='fas fa-clock'></i> Dünya Saatleri Ayarları</a>"
                                           "<a class='w3-bar-item w3-button' href='/configurecurrency'><i class='fas fa-money-bill'></i> Döviz Kurları Ayarları</a>";

static const char WEB_ACTIONS2[] PROGMEM = "<a class='w3-bar-item w3-button' href='/configureoctoprint'><i class='fas fa-cube'></i> OctoPrint Ayarları</a>"
                                           "<a class='w3-bar-item w3-button' href='/configurepihole'><i class='fas fa-network-wired'></i> Pi-hole Ayarları</a>"
                                           "<a class='w3-bar-item w3-button' href='/pull'><i class='fas fa-cloud-download-alt'></i> Verileri Yenile</a>"
                                           "<a class='w3-bar-item w3-button' href='/display'>";

static const char WEB_ACTION3[] PROGMEM = "</a><a class='w3-bar-item w3-button' href='/systemreset' onclick='return confirm(\"Varsayılan hava durumu ayarlarına sıfırlamak istiyor musunuz?\")'><i class='fas fa-undo'></i> Ayarları Sıfırla</a>"
                                          "<a class='w3-bar-item w3-button' href='/forgetwifi' onclick='return confirm(\"WiFi bağlantısını unutmak mı istiyorsunuz?\")'><i class='fas fa-wifi'></i> WiFi Ağını Unut</a>"
                                          "<a class='w3-bar-item w3-button' href='/update'><i class='fas fa-wrench'></i> Yazılım Güncellemesi</a>"
                                          "<a class='w3-bar-item w3-button' href='https://github.com/idezentas/marquee-scroller-tr' target='_blank'><i class='fas fa-question-circle'></i> Hakkında</a>";

static const char CHANGE_FORM1[] PROGMEM = "<form class='w3-container' action='/locations' method='get'><h2>Ayarlar:</h2>"
                                           "<label>TimeZone DB API Anahtarı (<a href='https://timezonedb.com/register' target='_BLANK'>buradan</a> alabilirsiniz)</label>"
                                           "<input class='w3-input w3-border w3-margin-bottom' type='text' name='TimeZoneDB' value='%TIMEDBKEY%' maxlength='60'>"
                                           "<label>OpenWeatherMap API Anahtarı (<a href='https://openweathermap.org/' target='_BLANK'>buradan</a> alabilirsiniz)</label>"
                                           "<input class='w3-input w3-border w3-margin-bottom' type='text' name='openWeatherMapApiKey' value='%WEATHERKEY%' maxlength='70'>"
                                           "<p><label>Şehir Adı Giriniz</label><input class='w3-input w3-border w3-margin-bottom' type='text' name='namecity' value='%NAMECITY%' maxlength='60'></p>";

static const char CHANGE_FORM2[] PROGMEM = "<p><input name='flashseconds' class='w3-check w3-margin-top' type='checkbox' %FLASHSECONDS%> Saati gösterirken : işaretini yanıp söndür</p>"
                                           "<p><label>Mesaj (60 karaktere kadar)</label><input class='w3-input w3-border w3-margin-bottom' type='text' name='marqueeMsg' value='%MSG%' maxlength='60'></p>"
                                           "<p><label>Başlangıç Zamanı </label><input name='startTime' type='time' value='%STARTTIME%'></p>"
                                           "<p><label>Bitiş Zamanı </label><input name='endTime' type='time' value='%ENDTIME%'></p>";

static const char CHANGE_FORM3[] PROGMEM = "<p>Ekran Parlaklığı <input class='w3-border w3-margin-bottom' name='ledintensity' type='number' min='0' max='15' value='%INTENSITYOPTIONS%'></p>"
                                           "<p>Ekran Kaydırma Hızı <select class='w3-option w3-padding' name='scrollspeed'>%SCROLLOPTIONS%</select></p>"
                                           "<p>Veri Yenileme (Dakika Cinsinden) <select class='w3-option w3-padding' name='refresh'>%OPTIONS%</select></p>"
                                           "<p>Kayan Yazı Süresi (Dakika Cinsinden) <input class='w3-border w3-margin-bottom' name='refreshDisplay' type='number' min='1' max='10' value='%REFRESH_DISPLAY%'></p>"
                                           "<p>Tema Rengini Seçiniz <select class='w3-option w3-padding' name='theme' id='theme'></select></p>"
                                           "<script>var s1='%THEME_OPTIONS%';var tt1='';var xmlhttp=new XMLHttpRequest();xmlhttp.open('GET','https://raw.githubusercontent.com/idezentas/marquee-scroller-tr/master/themes.json',true);xmlhttp.onreadystatechange=function(){if(xmlhttp.readyState==4){if(xmlhttp.status==200){var obj=JSON.parse(xmlhttp.responseText);obj.sources.forEach(updateOptions);}}};"
                                           "xmlhttp.send();"
                                           "function updateOptions(it){if(it!=null){var se=(s1==it.id)?' selected':'';tt1+='<option value=\"'+it.id+'\"'+se+'>'+it.name+'</option>';document.getElementById('theme').innerHTML=tt1;}}</script>"
                                           "<hr><p><input name='isBasicAuth' class='w3-check w3-margin-top' type='checkbox' %IS_BASICAUTH_CHECKED%> Ayarlar Değişiklikleri için Kullanıcı Adı ve Şifre Kullan</p>"
                                           "<p><label>Kullanıcı Adı (Web Sunucu İçin)</label><input class='w3-input w3-border w3-margin-bottom' type='text' name='userid' value='%USERID%' maxlength='20'></p>"
                                           "<p><label>Şifre (Web Sunucu İçin)</label><input class='w3-input w3-border w3-margin-bottom' type='password' name='stationpassword' value='%STATIONPASSWORD%'></p>"
                                           "<p><button class='w3-button w3-block w3-green w3-section w3-padding' type='submit'>Kaydet</button></p></form>"
                                           "<script>function isNumberKey(e){var h=e.which?e.which:event.keyCode;return!(h>31&&(h<48||h>57))}</script>";

static const char WIDECLOCK_FORM[] PROGMEM = "<form class='w3-container' action='/savewideclock' method='get'><h2>Geniş Saat Ayarları:</h2>"
                                             "<p>Geniş Saat Gösterme Formatı <select class='w3-option w3-padding' name='wideclockformat'>%WIDECLOCKOPTIONS%</select></p>"
                                             "<button class='w3-button w3-block w3-grey w3-section w3-padding' type='submit'>Kaydet</button></form>";

static const char PIHOLE_FORM[] PROGMEM = "<form class='w3-container' action='/savepihole' method='get'><h2>Pi-hole Ayarları:</h2>"
                                          "<p><input name='displaypihole' class='w3-check w3-margin-top' type='checkbox' %PIHOLECHECKED%> Pi-hole İstatistikleri Göster</p>"
                                          "<label>Pi-hole Adresi (http:// olmadan)</label><input class='w3-input w3-border w3-margin-bottom' type='text' name='piholeAddress' id='piholeAddress' value='%PIHOLEADDRESS%' maxlength='60'>"
                                          "<label>Pi-hole Portu</label><input class='w3-input w3-border w3-margin-bottom' type='text' name='piholePort' id='piholePort' value='%PIHOLEPORT%' maxlength='5'  onkeypress='return isNumberKey(event)'>"
                                          "<label>Pi-hole API Token (Pi-hole &rarr; Ayarlar &rarr; API/Web Arayüzü sayfasından)</label>"
                                          "<input class='w3-input w3-border w3-margin-bottom' type='text' name='piApiToken' id='piApiToken' value='%PIAPITOKEN%' maxlength='65'>"
                                          "<input type='button' value='Test Bağlantısı ve JSON Yanıtı' onclick='testPiHole()'><p id='PiHoleTest'></p>"
                                          "<button class='w3-button w3-block w3-green w3-section w3-padding' type='submit'>Kaydet</button></form>"
                                          "<script>function isNumberKey(e){var h=e.which?e.which:event.keyCode;return!(h>31&&(h<48||h>57))}</script>";

static const char PIHOLE_TEST[] PROGMEM = "<script>function testPiHole(){var e=document.getElementById(\"PiHoleTest\"),t=document.getElementById(\"piholeAddress\").value,"
                                          "n=document.getElementById(\"piholePort\").value,api=document.getElementById(\"piApiToken\").value;;"
                                          "if(e.innerHTML=\"\",\"\"==t||\"\"==n)return e.innerHTML=\"* Address and Port are required\","
                                          "void(e.style.background=\"\");var r=\"http://\"+t+\":\"+n;r+=\"/admin/api.php?summary=3&auth=\"+api,window.open(r,\"_blank\").focus()}</script>";

static const char OCTO_FORM[] PROGMEM = "<form class='w3-container' action='/saveoctoprint' method='get'><h2>OctoPrint Ayarları:</h2>"
                                        "<p><input name='displayoctoprint' class='w3-check w3-margin-top' type='checkbox' %OCTOCHECKED%> OctoPrint Durumunu Göster</p>"
                                        "<p><input name='octoprintprogress' class='w3-check w3-margin-top' type='checkbox' %OCTOPROGRESSCHECKED%> OctoPrint İlerlemesini Saatle Göster</p>"
                                        "<label>OctoPrint API Anahtarı (Sunucunuzdan Alın)</label><input class='w3-input w3-border w3-margin-bottom' type='text' name='octoPrintApiKey' value='%OCTOKEY%' maxlength='60'>"
                                        "<label>OctoPrint Adresi (http:// olmadan)</label><input class='w3-input w3-border w3-margin-bottom' type='text' name='octoPrintAddress' value='%OCTOADDRESS%' maxlength='60'>"
                                        "<label>OctoPrint Portu</label><input class='w3-input w3-border w3-margin-bottom' type='text' name='octoPrintPort' value='%OCTOPORT%' maxlength='5'  onkeypress='return isNumberKey(event)'>"
                                        "<label>OctoPrint Kullanıcı Adı (Yalnızca Haproxy Veya Basic Auth Açıksa Gereklidir)</label><input class='w3-input w3-border w3-margin-bottom' type='text' name='octoUser' value='%OCTOUSER%' maxlength='30'>"
                                        "<label>OctoPrint Şifresi </label><input class='w3-input w3-border w3-margin-bottom' type='password' name='octoPass' value='%OCTOPASS%'>"
                                        "<button class='w3-button w3-block w3-green w3-section w3-padding' type='submit'>Kaydet</button></form>"
                                        "<script>function isNumberKey(e){var h=e.which?e.which:event.keyCode;return!(h>31&&(h<48||h>57))}</script>";

static const char WORLD_CLOCK_FORM[] PROGMEM = "<form class='w3-container' action='/saveworldclock' method='get'><h2>Dünya Saatleri Ayarları:</h2>"
                                               "<p><input name='displayworldclock' class='w3-check w3-margin-top' type='checkbox' %WORLDCLOCKCHECKED%> Dünya Saatlerini Göster</p>"
                                               "<p><label>1.Şehrin Adı</label><input class='w3-input w3-border w3-margin-bottom' type='text' name='worldcityname1' value='%WORLDCITYNAME1%' maxlength='60'></p>"
                                               "<p><label>2.Şehrin Adı</label><input class='w3-input w3-border w3-margin-bottom' type='text' name='worldcityname2' value='%WORLDCITYNAME2%' maxlength='60'></p>"
                                               "<p><label>3.Şehrin Adı</label><input class='w3-input w3-border w3-margin-bottom' type='text' name='worldcityname3' value='%WORLDCITYNAME3%' maxlength='60'></p>"
                                               "<button class='w3-button w3-block w3-green w3-section w3-padding' type='submit'>Kaydet</button></form>"
                                               "<script>function isNumberKey(e){var h=e.which?e.which:event.keyCode;return!(h>31&&(h<48||h>57))}</script>";

static const char PRAYERS_FORM[] PROGMEM = "<form class='w3-container' action='/saveprayers' method='get'><h2>Namaz Vakitleri Ayarları:</h2>"
                                           "<p><input name='displayprayers' class='w3-check w3-margin-top' type='checkbox' %PRAYERSCHECKED%> Namaz Vakitlerini Göster</p>"
                                           "<button class='w3-button w3-block w3-green w3-section w3-padding' type='submit'>Kaydet</button></form>"
                                           "<script>function isNumberKey(e){var h=e.which?e.which:event.keyCode;return!(h>31&&(h<48||h>57))}</script>";

static const char CURRENCY_FORM[] PROGMEM = "<form class='w3-container' action='/savecurrency' method='get'><h2>Döviz Kurları Ayarları:</h2>"
                                            "<p><input name='displaycurrencies' class='w3-check w3-margin-top' type='checkbox' %CURRENCYCHECKED%> Döviz Kurlarını Göster</p>"
                                            "<p>1.Temel Para Birimini Seçiniz <select class='w3-option w3-padding' name='basecurrency1' id='basecurrency1'></select></p>"
                                            "<script>var s1='%BASECURRENCY1%';var tt1='';var xmlhttp1=new XMLHttpRequest();xmlhttp1.open('GET','https://raw.githubusercontent.com/idezentas/marquee-scroller-tr/master/currencies.json',true);xmlhttp1.onreadystatechange=function(){if(xmlhttp1.readyState==4){if(xmlhttp1.status==200){var obj=JSON.parse(xmlhttp1.responseText);obj.sources.forEach(updateOptions1);}}};"
                                            "xmlhttp1.send();"
                                            "function updateOptions1(it){if(it!=null){var se=(s1==it.id)?' selected':'';tt1+='<option value=\"'+it.id+'\"'+se+'>'+it.name+'</option>';document.getElementById('basecurrency1').innerHTML=tt1;}}</script>"
                                            "<p>2.Temel Para Birimini Seçiniz <select class='w3-option w3-padding' name='basecurrency2' id='basecurrency2'></select></p>"
                                            "<script>var s2='%BASECURRENCY2%';var tt2='';var xmlhttp2=new XMLHttpRequest();xmlhttp2.open('GET','https://raw.githubusercontent.com/idezentas/marquee-scroller-tr/master/currencies.json',true);xmlhttp2.onreadystatechange=function(){if(xmlhttp2.readyState==4){if(xmlhttp2.status==200){var obj=JSON.parse(xmlhttp2.responseText);obj.sources.forEach(updateOptions2);}}};"
                                            "xmlhttp2.send();"
                                            "function updateOptions2(it){if(it!=null){var se=(s2==it.id)?' selected':'';tt2+='<option value=\"'+it.id+'\"'+se+'>'+it.name+'</option>';document.getElementById('basecurrency2').innerHTML=tt2;}}</script>"
                                            "<p>Dönüştürülecek Para Birimini Seçiniz <select class='w3-option w3-padding' name='targetcurrency' id='targetcurrency'></select></p>"
                                            "<script>var s='%TARGETCURRENCY%';var tt='';var xmlhttp=new XMLHttpRequest();xmlhttp.open('GET','https://raw.githubusercontent.com/idezentas/marquee-scroller-tr/master/currencies.json',true);xmlhttp.onreadystatechange=function(){if(xmlhttp.readyState==4){if(xmlhttp.status==200){var obj=JSON.parse(xmlhttp.responseText);obj.sources.forEach(updateOptions);}}};"
                                            "xmlhttp.send();"
                                            "function updateOptions(it){if(it!=null){var se=(s==it.id)?' selected':'';tt+='<option value=\"'+it.id+'\"'+se+'>'+it.name+'</option>';document.getElementById('targetcurrency').innerHTML=tt;}}</script>"
                                            "<button class='w3-button w3-block w3-green w3-section w3-padding' type='submit'>Kaydet</button></form>"
                                            "<script>function isNumberKey(e){var h=e.which?e.which:event.keyCode;return!(h>31&&(h<48||h>57))}</script>";

static const char MATRIX_FORM[] PROGMEM = "<form class='w3-container' action='/savematrix' method='get'><h2>Ekran Ayarları:</h2>"
                                          "<p><input name='enablescrolling' class='w3-check w3-margin-top' type='checkbox' %SCROLL_CHECKED%> Kayan Yazıyı Göster</p>"
                                          "<p><input name='showdate' class='w3-check w3-margin-top' type='checkbox' %DATE_CHECKED%> Tarihi Göster</p>"
                                          "<p><input name='showtimezone' class='w3-check w3-margin-top' type='checkbox' %TIMEZONE_CHECKED%> Zaman Dilimini Göster</p>"
                                          "<p><input name='showriseset' class='w3-check w3-margin-top' type='checkbox' %RISE_SET_CHECKED%> Güneşin Doğuş ve Batış Saatlerini Göster</p>"
                                          "<p><input name='showcity' class='w3-check w3-margin-top' type='checkbox' %CITY_CHECKED%> Şehir Adını Göster</p>"
                                          "<p><input name='showtemp' class='w3-check w3-margin-top' type='checkbox' %TEMP_CHECKED%> Sıcaklığı Göster</p>"
                                          "<p><input name='showfeeltemp' class='w3-check w3-margin-top' type='checkbox' %FEEL_TEMP_CHECKED%> Hissedilen Sıcaklığı Göster</p>"
                                          "<p><input name='showhighlow' class='w3-check w3-margin-top' type='checkbox' %HIGHLOW_CHECKED%> Mevcut En Yüksek ve En Düşük Sıcaklıkları Göster</p>"
                                          "<p><input name='showcondition' class='w3-check w3-margin-top' type='checkbox' %CONDITION_CHECKED%> Hava Durumunu Göster</p>"
                                          "<p><input name='showhumidity' class='w3-check w3-margin-top' type='checkbox' %HUMIDITY_CHECKED%> Nemi Göster</p>";

static const char MATRIX_FORM_2[] PROGMEM = "<p><input name='showwind' class='w3-check w3-margin-top' type='checkbox' %WIND_CHECKED%> Rüzgarı Göster</p>"
                                            "<p><input name='showairpollution' class='w3-check w3-margin-top' type='checkbox' %AIR_POLLUTION_CHECKED%> Hava Kirliğini Göster</p>"
                                            "<p><input name='showpressure' class='w3-check w3-margin-top' type='checkbox' %PRESSURE_CHECKED%> Barometrik Basıncı Göster</p>"
                                            "<button class='w3-button w3-block w3-green w3-section w3-padding' type='submit'>Kaydet</button></form>";

const int TIMEOUT = 500; // 500 = 1/2 second
int timeoutCount = 0;

// Change the externalLight to the pin you wish to use if other than the Built-in LED
int externalLight = LED_BUILTIN; // LED_BUILTIN is is the built in LED on the Wemos

time_t serverLastUpdate;
String serverLastUpdateString;

void setup()
{
  Serial.begin(115200);
  LittleFS.begin();
  bmp180.begin();
  // LittleFS.remove(CONFIG);
  delay(10);

  // Initialize digital pin for LED
  pinMode(externalLight, OUTPUT);

  Serial.println();

  Serial.println("Getting Data From LittleFS");

  // New Line to clear from start garbage
  Serial.println();

  readCityIds();

  Serial.println("------------------------------");

  File file2 = LittleFS.open(CONFIG, "r");

  if (!file2)
  {
    Serial.println("LittleFS File Reading Failed!");
    return;
  }

  Serial.println("LittleFS File Content:");

  while (file2.available())
  {

    Serial.write(file2.read());
  }

  file2.close();

  Serial.println("------------------------------");
  Serial.println();

  Serial.println("Number of LED Displays: " + String(numberOfHorizontalDisplays));
  // initialize display
  matrix.setIntensity(0); // Use a value between 0 and 15 for brightness

  int maxPos = numberOfHorizontalDisplays * numberOfVerticalDisplays;
  for (int i = 0; i < maxPos; i++)
  {
    matrix.setRotation(i, ledRotation);
    matrix.setPosition(i, maxPos - i - 1, 0);
  }

  Serial.println("matrix created");
  matrix.fillScreen(LOW); // show black
  message = CleanText(message);
  centerPrint(message);

  for (int inx = 0; inx <= 15; inx++)
  {
    matrix.setIntensity(inx);
    delay(100);
  }
  for (int inx = 15; inx >= 0; inx--)
  {
    matrix.setIntensity(inx);
    delay(60);
  }
  delay(1000);
  matrix.setIntensity(displayIntensity);
  // noTone(BUZZER_PIN);

  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  // Uncomment for testing wifi manager
  // wifiManager.resetSettings();
  wifiManager.setAPCallback(configModeCallback);

  // Custom Station (client) Static IP Configuration - Set custom IP for your Network (IP, Gateway, Subnet mask)
  // wifiManager.setSTAStaticIPConfig(IPAddress(192,168,0,99), IPAddress(192,168,0,1), IPAddress(255,255,255,0));

  String hostname(HOSTNAME);
  hostname += String(ESP.getChipId(), HEX);
  if (!wifiManager.autoConnect((const char *)hostname.c_str()))
  { // new addition
    delay(3000);
    WiFi.disconnect(true);
    ESP.reset();
    delay(5000);
  }

  // print the received signal strength:
  Serial.print("Signal Strength (RSSI): ");
  Serial.print("%");
  Serial.println(getWifiQuality());

  if (ENABLE_OTA)
  {
    ArduinoOTA.onStart([]()
                       { Serial.println("Start"); });
    ArduinoOTA.onEnd([]()
                     { Serial.println("\nEnd"); });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                          { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
    ArduinoOTA.onError([](ota_error_t error)
                       {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed"); });
    ArduinoOTA.setHostname((const char *)hostname.c_str());
    if (OTA_Password != "")
    {
      ArduinoOTA.setPassword(((const char *)OTA_Password.c_str()));
    }
    ArduinoOTA.begin();
  }

  if (WEBSERVER_ENABLED)
  {
    server.on("/", displayWeatherData);
    server.on("/pull", handlePull);
    server.on("/locations", handleLocations);
    server.on("/savewideclock", handleSaveWideClock);
    server.on("/saveoctoprint", handleSaveOctoprint);
    server.on("/savepihole", handleSavePihole);
    server.on("/saveworldclock", handleSaveWorldClock);
    server.on("/saveprayers", handleSavePrayers);
    server.on("/savecurrency", handleSaveCurrency);
    server.on("/savematrix", handleSaveMatrix);
    server.on("/systemreset", handleSystemReset);
    server.on("/forgetwifi", handleForgetWifi);
    server.on("/configure", handleConfigure);
    server.on("/configurewideclock", handleWideClockConfigure);
    server.on("/configureoctoprint", handleOctoprintConfigure);
    server.on("/configurepihole", handlePiholeConfigure);
    server.on("/configurematrix", handleMatrixConfigure);
    server.on("/configureworldclock", handleWorldClockConfigure);
    server.on("/configureprayers", handlePrayersConfigure);
    server.on("/configurecurrency", handleCurrencyConfigure);
    server.on("/display", handleDisplay);
    server.on("/displayworldclock", displayWorldClockData);
    server.on("/displayworldclockweather", displayWorldClockWeatherData);
    server.on("/displayprayerstime", displayPrayersTimeData);
    server.on("/displaycurrency", displayCurrencyData);
    server.onNotFound(redirectHome);
    serverUpdater.setup(&server, "/update", www_username, www_password);
    // Start the server
    server.begin();
    Serial.println("Server started");
    // Print the WIFI
    Serial.print("WIFI: ");
    Serial.println(WiFi.SSID());
    // Print the IP address
    String webAddress = "http://" + WiFi.localIP().toString() + ":" + String(WEBSERVER_PORT) + "/";
    Serial.println("Use this URL : " + webAddress);
    scrollMessage(" v" + String(VERSION) + "  WIFI: " + WiFi.SSID() + "  IP: " + WiFi.localIP().toString() + "  " + wifiQualityS + getWifiQuality() + "  ");
  }
  else
  {
    Serial.println("Web Interface is Disabled");
    scrollMessage(webDisableS);
  }

  flashLED(1, 500);
}

//************************************************************
// Main Looop
//************************************************************
void loop()
{

  // Get some Weather Data to serve
  if ((getMinutesFromLastRefresh() >= minutesBetweenDataRefresh) || lastEpoch == 0)
  {
    getWeatherData();
    if (displayOn)
    {
      serverLastUpdate = now();
      serverLastUpdateString = getTimestamp2Date(serverLastUpdate);
    }
  }
  checkDisplay(); // this will see if we need to turn it on or off for night mode.

  if (lastMinute != TimeDBClient.zeroPad(minute()))
  {
    lastMinute = TimeDBClient.zeroPad(minute());

    if ((weatherClient.getError(0) != "") && ENABLE_SCROLL)
    {
      scrollMessage(weatherClient.getError(0));
      return;
    }

    if (displayOn)
    {
      matrix.shutdown(false);
    }
    matrix.fillScreen(LOW); // show black

    if (OCTOPRINT_ENABLED)
    {
      if (displayOn && ((printerClient.isOperational() || printerClient.isPrinting()) || printerCount == 0))
      {
        // This should only get called if the printer is actually running or if it has been 2 minutes since last check
        printerClient.getPrinterJobResults();
      }
      printerCount += 1;
      if (printerCount > 2)
      {
        printerCount = 0;
      }
    }

    displayRefreshCount--;
    // Check to see if we need to Scroll some Data
    if (displayRefreshCount <= 0)
    {
      displayRefreshCount = minutesBetweenScrolling;
      if (ENABLE_SCROLL)
      {
        String temperature = weatherClient.getTempRounded(0);
        String feelR = weatherClient.getFeelRounded(0);
        String highR = weatherClient.getHighRounded(0);
        String lowR = weatherClient.getLowRounded(0);

        String description = weatherClient.getDescription(0);
        String descriptionClear = CleanText(description);
        descriptionClear.toUpperCase();

        String airQuality = weatherClient.getAqi(0);
        String airQualityClear = CleanText(airQuality);
        airQualityClear.toUpperCase();

        String dayNameU = TimeDBClient.getDayName();
        String dayNameUClear = CleanText(dayNameU);
        dayNameUClear.toUpperCase();

        String monthNameU = TimeDBClient.getMonthName();
        String monthNameUClear = CleanText(monthNameU);
        monthNameUClear.toUpperCase();

        // String cityNameU = weatherClient.getCity(0);
        String cityNameU = TimeDBClient.getCityName(0);
        String cityNameUClear = CleanText(cityNameU);
        cityNameUClear.toUpperCase();

        String regionNameU = TimeDBClient.getRegionName(0);
        String regionNameUClear = CleanText(regionNameU);
        regionNameUClear.toUpperCase();

        String directionTextU = weatherClient.getDirectionText(0);
        String directionTextUClear = CleanText(directionTextU);
        directionTextUClear.toUpperCase();

        String msg;
        msg += " ";

        if (SHOW_TIMEZONE)
        {
          msg += "ZAMAN DILIMI: " + TimeDBClient.getZoneName(0) + "  ";
          msg += "(" + TimeDBClient.getAbbreviation(0) + ")  ";
        }

        if (SHOW_DATE)
        {
          msg += "TARIH= ";
          msg += dayNameUClear + "  ";
          msg += zeroPad(day()) + " " + monthNameUClear + " " + year() + " ";
          msg += "SUANKI SAAT= ";
          msg += TimeDBClient.zeroPad(hour()) + ":" + TimeDBClient.zeroPad(minute()) + ":" + TimeDBClient.zeroPad(second()) + " ";
        }

        if (SHOW_CITY)
        {
          // msg += cityNameU + "/" + weatherClient.getCountry(0) + "  ";
          msg += cityNameUClear + "," + regionNameUClear + "," + TimeDBClient.getCountryCode(0) + "  ";
        }

        if (SHOW_TEMP)
        {
          msg += "SICAKLIK: " + temperature + getTempSymbol() + "  ";
        }

        if (SHOW_FEEL_TEMP)
        {
          msg += "HISSEDILEN SICAKLIK: " + feelR + getTempSymbol() + "  ";
        }

        if (SHOW_HIGHLOW)
        {
          msg += "EN YUKSEK SICAKLIK: " + highR + getTempSymbol() + " ";
          msg += "EN DUSUK SICAKLIK: " + lowR + getTempSymbol() + "  ";
        }

        if (SHOW_CONDITION)
        {
          msg += "HAVA DURUMU: " + descriptionClear + "  ";
          msg += "BULUTLANMA: %" + weatherClient.getCloudcover(0) + "  ";
        }
        if (SHOW_AIR_POLLUTION)
        {
          msg += "HAVA KIRLILIGI: " + airQualityClear + "  ";
        }
        if (SHOW_HUMIDITY)
        {
          msg += "NEM: %" + weatherClient.getHumidityRounded(0) + "  ";
        }
        if (SHOW_WIND)
        {
          msg += "RUZGAR: " + directionTextUClear + " @ " + weatherClient.getWindRounded(0) + " " + getSpeedSymbol() + "  ";
        }

        if (SHOW_PRESSURE)
        {
          msg += "BASINC: " + weatherClient.getPressure(0) + getPressureSymbol() + "  ";
        }

        if (SHOW_RISE_SET)
        {
          msg += "GUN DOGUMU= " + weatherClient.getSunrise(0) + "  ";
          msg += "GUN BATIMI= " + weatherClient.getSunset(0) + "  ";
        }

        msg += marqueeMessage + " ";

        if (OCTOPRINT_ENABLED && printerClient.isPrinting())
        {
          msg += "  " + printerClient.getFileName() + " ";
          msg += "(" + printerClient.getProgressCompletion() + "%)  ";
        }

        if (USE_PIHOLE)
        {
          piholeClient.getPiHoleData(PiHoleServer, PiHolePort, PiHoleApiKey);
          piholeClient.getGraphData(PiHoleServer, PiHolePort, PiHoleApiKey);
          if (piholeClient.getPiHoleStatus() != "")
          {
            msg += "    Pi-hole (" + piholeClient.getPiHoleStatus() + "): " + piholeClient.getAdsPercentageToday() + "% ";
          }
        }

        scrollMessage(msg);
        drawPiholeGraph();
      }
    }
  }

  String currentTime = hourMinutes(false);

  if (numberOfHorizontalDisplays >= 8)
  {
    if (Wide_Clock_Style == "1")
    {
      // On Wide Display -- show the current temperature as well
      String currentTemp = weatherClient.getTempRounded(0);
      currentTime += " " + currentTemp + getTempSymbol();
    }
    if (Wide_Clock_Style == "2")
    {
      currentTime = currentTime + secondsIndicator(false) + TimeDBClient.zeroPad(second());
      matrix.fillScreen(LOW); // show black
    }
    if (Wide_Clock_Style == "3")
    {
      // No change this is normal clock display
    }
  }
  matrix.fillScreen(LOW);
  centerPrint(currentTime, true);

  if (WEBSERVER_ENABLED)
  {
    server.handleClient();
  }
  if (ENABLE_OTA)
  {
    ArduinoOTA.handle();
  }
}

String zeroPad(int value)
{
  String rtnValue = String(value);
  if (value < 10)
  {
    rtnValue = "0" + rtnValue;
  }
  return rtnValue;
}

String hourMinutes(boolean isRefresh)
{
  if (IS_24HOUR)
  {
    return String(hour()) + secondsIndicator(isRefresh) + TimeDBClient.zeroPad(minute());
  }
  else
  {
    return String(hourFormat12()) + secondsIndicator(isRefresh) + TimeDBClient.zeroPad(minute());
  }
}

String secondsIndicator(boolean isRefresh)
{
  String rtnValue = ":";
  if (isRefresh == false && (flashOnSeconds && (second() % 2) == 0))
  {
    rtnValue = " ";
  }
  return rtnValue;
}

boolean athentication()
{
  if (IS_BASIC_AUTH)
  {
    return server.authenticate(www_username, www_password);
  }
  return true; // Authentication not required
}

void handlePull()
{
  getWeatherData(); // this will force a data pull for new weather
  displayWeatherData();
}

void handleSaveWideClock()
{
  if (!athentication())
  {
    return server.requestAuthentication();
  }
  if (numberOfHorizontalDisplays >= 8)
  {
    Wide_Clock_Style = server.arg("wideclockformat");
    writeCityIds();
    matrix.fillScreen(LOW); // show black
  }
  redirectHome();
}

void handleSaveOctoprint()
{
  if (!athentication())
  {
    return server.requestAuthentication();
  }
  OCTOPRINT_ENABLED = server.hasArg("displayoctoprint");
  OCTOPRINT_PROGRESS = server.hasArg("octoprintprogress");
  OctoPrintApiKey = server.arg("octoPrintApiKey");
  OctoPrintServer = server.arg("octoPrintAddress");
  OctoPrintPort = server.arg("octoPrintPort").toInt();
  OctoAuthUser = server.arg("octoUser");
  OctoAuthPass = server.arg("octoPass");
  matrix.fillScreen(LOW); // show black
  writeCityIds();
  if (OCTOPRINT_ENABLED)
  {
    printerClient.getPrinterJobResults();
  }
  redirectHome();
}

void handleSavePihole()
{
  if (!athentication())
  {
    return server.requestAuthentication();
  }
  USE_PIHOLE = server.hasArg("displaypihole");
  PiHoleServer = server.arg("piholeAddress");
  PiHolePort = server.arg("piholePort").toInt();
  PiHoleApiKey = server.arg("piApiToken");
  Serial.println("PiHoleApiKey from save: " + PiHoleApiKey);
  writeCityIds();
  if (USE_PIHOLE)
  {
    piholeClient.getPiHoleData(PiHoleServer, PiHolePort, PiHoleApiKey);
    piholeClient.getGraphData(PiHoleServer, PiHolePort, PiHoleApiKey);
  }
  redirectHome();
}

void handleSaveWorldClock()
{
  if (!athentication())
  {
    return server.requestAuthentication();
  }
  WORLD_CLOCK_ENABLED = server.hasArg("displayworldclock");
  WorldCityName1 = server.arg("worldcityname1");
  WorldCityName2 = server.arg("worldcityname2");
  WorldCityName3 = server.arg("worldcityname3");
  matrix.fillScreen(LOW); // show black
  writeCityIds();
  worldWeatherClient.updateWeatherName(WorldCityName1, 1);
  delay(1000);
  timezoneClient.getCityTime(TIMEDBKEY, worldWeatherClient.getLat(1), worldWeatherClient.getLon(1), 1);
  delay(1000);
  timezoneClient.convertTimezone(TIMEDBKEY, TimeDBClient.getZoneName(0), timezoneClient.getZoneName(1), 1);
  delay(1000);
  worldWeatherClient.updateCityAirPollution(worldWeatherClient.getLat(1), worldWeatherClient.getLon(1), 1);
  delay(1000);
  worldWeatherClient.updateSunMoonTime(worldWeatherClient.getCityTimeStamp(1), worldWeatherClient.getLat(1), worldWeatherClient.getLon(1), 1);
  delay(1000);
  worldWeatherClient.updateWeatherName(WorldCityName2, 2);
  delay(1000);
  timezoneClient.getCityTime(TIMEDBKEY, worldWeatherClient.getLat(2), worldWeatherClient.getLon(2), 2);
  delay(1000);
  timezoneClient.convertTimezone(TIMEDBKEY, TimeDBClient.getZoneName(0), timezoneClient.getZoneName(2), 2);
  delay(1000);
  worldWeatherClient.updateCityAirPollution(worldWeatherClient.getLat(2), worldWeatherClient.getLon(2), 2);
  delay(1000);
  worldWeatherClient.updateSunMoonTime(worldWeatherClient.getCityTimeStamp(2), worldWeatherClient.getLat(2), worldWeatherClient.getLon(2), 2);
  delay(1000);
  worldWeatherClient.updateWeatherName(WorldCityName3, 3);
  delay(1000);
  timezoneClient.getCityTime(TIMEDBKEY, worldWeatherClient.getLat(3), worldWeatherClient.getLon(3), 3);
  delay(1000);
  timezoneClient.convertTimezone(TIMEDBKEY, TimeDBClient.getZoneName(0), timezoneClient.getZoneName(3), 3);
  delay(1000);
  worldWeatherClient.updateCityAirPollution(worldWeatherClient.getLat(3), worldWeatherClient.getLon(3), 3);
  delay(1000);
  worldWeatherClient.updateSunMoonTime(worldWeatherClient.getCityTimeStamp(3), worldWeatherClient.getLat(3), worldWeatherClient.getLon(3), 3);
  delay(1000);
  redirectHome();
}

void handleSavePrayers()
{
  if (!athentication())
  {
    return server.requestAuthentication();
  }
  PRAYERS_ENABLED = server.hasArg("displayprayers");
  matrix.fillScreen(LOW); // show black
  writeCityIds();
  String cityEncoded = encodeHtmlString(CityName);
  prayersClient.updatePrayerTimesAddress(cityEncoded, 0);
  delay(1000);
  String cityEncoded1 = encodeHtmlString(WorldCityName1);
  prayersClient.updatePrayerTimesAddress(cityEncoded1, 1);
  delay(1000);
  String cityEncoded2 = encodeHtmlString(WorldCityName2);
  prayersClient.updatePrayerTimesAddress(cityEncoded2, 2);
  delay(1000);
  String cityEncoded3 = encodeHtmlString(WorldCityName3);
  prayersClient.updatePrayerTimesAddress(cityEncoded3, 3);
  delay(1000);
  redirectHome();
}

void handleSaveCurrency()
{
  if (!athentication())
  {
    return server.requestAuthentication();
  }
  CURRENCY_ENABLED = server.hasArg("displaycurrencies");
  BaseCurrency1 = server.arg("basecurrency1");
  BaseCurrency2 = server.arg("basecurrency2");
  TargetCurrency = server.arg("targetcurrency");
  matrix.fillScreen(LOW); // show black
  writeCityIds();
  currencyClient.updateCurrency(BaseCurrency1, TargetCurrency, 0);
  delay(1000);
  currencyClient.updateCurrency(BaseCurrency2, TargetCurrency, 1);
  delay(1000);
  redirectHome();
}

void handleSaveMatrix()
{
  if (!athentication())
  {
    return server.requestAuthentication();
  }
  ENABLE_SCROLL = server.hasArg("enablescrolling");
  SHOW_DATE = server.hasArg("showdate");
  SHOW_TIMEZONE = server.hasArg("showtimezone");
  SHOW_RISE_SET = server.hasArg("showriseset");
  SHOW_FEEL_TEMP = server.hasArg("showfeeltemp");
  SHOW_TEMP = server.hasArg("showtemp");
  SHOW_CITY = server.hasArg("showcity");
  SHOW_CONDITION = server.hasArg("showcondition");
  SHOW_HUMIDITY = server.hasArg("showhumidity");
  SHOW_WIND = server.hasArg("showwind");
  SHOW_AIR_POLLUTION = server.hasArg("showairpollution");
  SHOW_PRESSURE = server.hasArg("showpressure");
  SHOW_HIGHLOW = server.hasArg("showhighlow");
  writeCityIds();
  matrix.fillScreen(LOW); // show black
  redirectHome();
}

void handleLocations()
{
  if (!athentication())
  {
    return server.requestAuthentication();
  }
  TIMEDBKEY = server.arg("TimeZoneDB");
  APIKEY = server.arg("openWeatherMapApiKey");
  flashOnSeconds = server.hasArg("flashseconds");
  CityName = server.arg("namecity");
  marqueeMessage = decodeHtmlString(server.arg("marqueeMsg"));
  timeDisplayTurnsOn = decodeHtmlString(server.arg("startTime"));
  timeDisplayTurnsOff = decodeHtmlString(server.arg("endTime"));
  displayIntensity = server.arg("ledintensity").toInt();
  minutesBetweenDataRefresh = server.arg("refresh").toInt();
  themeColor = server.arg("theme");
  minutesBetweenScrolling = server.arg("refreshDisplay").toInt();
  displayScrollSpeed = server.arg("scrollspeed").toInt();
  IS_BASIC_AUTH = server.hasArg("isBasicAuth");
  String temp = server.arg("userid");
  temp.toCharArray(www_username, sizeof(temp));
  temp = server.arg("stationpassword");
  temp.toCharArray(www_password, sizeof(temp));
  weatherClient.setMetric(IS_METRIC);
  matrix.fillScreen(LOW); // show black
  writeCityIds();
  getWeatherData(); // this will force a data pull for new weather
  redirectHome();
}

void handleSystemReset()
{
  if (!athentication())
  {
    return server.requestAuthentication();
  }
  Serial.println("Reset System Configuration");
  if (LittleFS.remove(CONFIG))
  {
    redirectHome();
    ESP.restart();
  }
}

void handleForgetWifi()
{
  if (!athentication())
  {
    return server.requestAuthentication();
  }
  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it around
  redirectHome();
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  ESP.restart();
}

void handleWideClockConfigure()
{
  if (!athentication())
  {
    return server.requestAuthentication();
  }
  digitalWrite(externalLight, LOW);

  server.sendHeader("Cache-Control", "no-cache, no-store");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");

  sendHeader();

  if (numberOfHorizontalDisplays >= 8)
  {
    // Wide display options
    String form = FPSTR(WIDECLOCK_FORM);
    String clockOptions = "<option value='1'>HH:MM Sıcaklık</option><option value='2'>HH:MM:SS</option><option value='3'>HH:MM</option>";
    clockOptions.replace(Wide_Clock_Style + "'", Wide_Clock_Style + "' selected");
    form.replace("%WIDECLOCKOPTIONS%", clockOptions);
    server.sendContent(form);
  }

  sendFooter();

  server.sendContent("");
  server.client().stop();
  digitalWrite(externalLight, HIGH);
}

void handleOctoprintConfigure()
{
  if (!athentication())
  {
    return server.requestAuthentication();
  }
  digitalWrite(externalLight, LOW);

  server.sendHeader("Cache-Control", "no-cache, no-store");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");

  sendHeader();

  String form = FPSTR(OCTO_FORM);
  String isOctoPrintDisplayedChecked = "";
  if (OCTOPRINT_ENABLED)
  {
    isOctoPrintDisplayedChecked = "checked='checked'";
  }
  form.replace("%OCTOCHECKED%", isOctoPrintDisplayedChecked);
  String isOctoPrintProgressChecked = "";
  if (OCTOPRINT_PROGRESS)
  {
    isOctoPrintProgressChecked = "checked='checked'";
  }
  form.replace("%OCTOPROGRESSCHECKED%", isOctoPrintProgressChecked);
  form.replace("%OCTOKEY%", OctoPrintApiKey);
  form.replace("%OCTOADDRESS%", OctoPrintServer);
  form.replace("%OCTOPORT%", String(OctoPrintPort));
  form.replace("%OCTOUSER%", OctoAuthUser);
  form.replace("%OCTOPASS%", OctoAuthPass);
  server.sendContent(form);

  sendFooter();

  server.sendContent("");
  server.client().stop();
  digitalWrite(externalLight, HIGH);
}

void handlePiholeConfigure()
{
  if (!athentication())
  {
    return server.requestAuthentication();
  }
  digitalWrite(externalLight, LOW);

  server.sendHeader("Cache-Control", "no-cache, no-store");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");

  sendHeader();

  server.sendContent(FPSTR(PIHOLE_TEST));

  String form = FPSTR(PIHOLE_FORM);
  String isPiholeDisplayedChecked = "";
  if (USE_PIHOLE)
  {
    isPiholeDisplayedChecked = "checked='checked'";
  }
  form.replace("%PIHOLECHECKED%", isPiholeDisplayedChecked);
  form.replace("%PIHOLEADDRESS%", PiHoleServer);
  form.replace("%PIHOLEPORT%", String(PiHolePort));
  form.replace("%PIAPITOKEN%", PiHoleApiKey);

  server.sendContent(form);
  form = "";

  sendFooter();

  server.sendContent("");
  server.client().stop();
  digitalWrite(externalLight, HIGH);
}

void handleWorldClockConfigure()
{
  if (!athentication())
  {
    return server.requestAuthentication();
  }
  digitalWrite(externalLight, LOW);

  server.sendHeader("Cache-Control", "no-cache, no-store");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");

  sendHeader();

  String form = FPSTR(WORLD_CLOCK_FORM);
  String isWorldClockDisplayedChecked = "";
  if (WORLD_CLOCK_ENABLED)
  {
    isWorldClockDisplayedChecked = "checked='checked'";
  }
  form.replace("%WORLDCLOCKCHECKED%", isWorldClockDisplayedChecked);
  form.replace("%WORLDCITYNAME1%", WorldCityName1);
  form.replace("%WORLDCITYNAME2%", WorldCityName2);
  form.replace("%WORLDCITYNAME3%", WorldCityName3);
  server.sendContent(form);

  sendFooter();

  server.sendContent("");
  server.client().stop();
  digitalWrite(externalLight, HIGH);
}

void handlePrayersConfigure()
{
  if (!athentication())
  {
    return server.requestAuthentication();
  }
  digitalWrite(externalLight, LOW);

  server.sendHeader("Cache-Control", "no-cache, no-store");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");

  sendHeader();

  String form = FPSTR(PRAYERS_FORM);
  String isPrayersDisplayedChecked = "";
  if (PRAYERS_ENABLED)
  {
    isPrayersDisplayedChecked = "checked='checked'";
  }
  form.replace("%PRAYERSCHECKED%", isPrayersDisplayedChecked);
  server.sendContent(form);

  sendFooter();

  server.sendContent("");
  server.client().stop();
  digitalWrite(externalLight, HIGH);
}

void handleCurrencyConfigure()
{
  if (!athentication())
  {
    return server.requestAuthentication();
  }
  digitalWrite(externalLight, LOW);

  server.sendHeader("Cache-Control", "no-cache, no-store");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");

  sendHeader();

  String form = FPSTR(CURRENCY_FORM);
  String isCurrencyDisplayedChecked = "";
  if (CURRENCY_ENABLED)
  {
    isCurrencyDisplayedChecked = "checked='checked'";
  }
  form.replace("%CURRENCYCHECKED%", isCurrencyDisplayedChecked);
  form.replace("%BASECURRENCY1%", BaseCurrency1);
  form.replace("%BASECURRENCY2%", BaseCurrency2);
  form.replace("%TARGETCURRENCY%", TargetCurrency);
  server.sendContent(form);

  sendFooter();

  server.sendContent("");
  server.client().stop();
  digitalWrite(externalLight, HIGH);
}

void handleMatrixConfigure()
{
  if (!athentication())
  {
    return server.requestAuthentication();
  }
  digitalWrite(externalLight, LOW);

  server.sendHeader("Cache-Control", "no-cache, no-store");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");

  sendHeader();

  String form = FPSTR(MATRIX_FORM);
  String isScrollDisplayedChecked = "";
  if (ENABLE_SCROLL)
  {
    isScrollDisplayedChecked = "checked='checked'";
  }
  form.replace("%SCROLL_CHECKED%", isScrollDisplayedChecked);

  String isDateChecked = "";
  if (SHOW_DATE)
  {
    isDateChecked = "checked='checked'";
  }
  form.replace("%DATE_CHECKED%", isDateChecked);

  String isTimezoneChecked = "";
  if (SHOW_TIMEZONE)
  {
    isTimezoneChecked = "checked='checked'";
  }
  form.replace("%TIMEZONE_CHECKED%", isTimezoneChecked);

  String isSunRiseSetChecked = "";
  if (SHOW_RISE_SET)
  {
    isSunRiseSetChecked = "checked='checked'";
  }
  form.replace("%RISE_SET_CHECKED%", isSunRiseSetChecked);

  String isCityChecked = "";
  if (SHOW_CITY)
  {
    isCityChecked = "checked='checked'";
  }
  form.replace("%CITY_CHECKED%", isCityChecked);

  String isTempChecked = "";
  if (SHOW_TEMP)
  {
    isTempChecked = "checked='checked'";
  }
  form.replace("%TEMP_CHECKED%", isTempChecked);

  String isFeelTempChecked = "";
  if (SHOW_FEEL_TEMP)
  {
    isFeelTempChecked = "checked='checked'";
  }
  form.replace("%FEEL_TEMP_CHECKED%", isFeelTempChecked);

  String isHighlowChecked = "";
  if (SHOW_HIGHLOW)
  {
    isHighlowChecked = "checked='checked'";
  }
  form.replace("%HIGHLOW_CHECKED%", isHighlowChecked);

  String isConditionChecked = "";
  if (SHOW_CONDITION)
  {
    isConditionChecked = "checked='checked'";
  }
  form.replace("%CONDITION_CHECKED%", isConditionChecked);

  String isHumidityChecked = "";
  if (SHOW_HUMIDITY)
  {
    isHumidityChecked = "checked='checked'";
  }
  form.replace("%HUMIDITY_CHECKED%", isHumidityChecked);
  server.sendContent(form);

  form = FPSTR(MATRIX_FORM_2);
  String isWindChecked = "";
  if (SHOW_WIND)
  {
    isWindChecked = "checked='checked'";
  }
  form.replace("%WIND_CHECKED%", isWindChecked);

  String isAirPollutionChecked = "";
  if (SHOW_AIR_POLLUTION)
  {
    isAirPollutionChecked = "checked='checked'";
  }
  form.replace("%AIR_POLLUTION_CHECKED%", isAirPollutionChecked);

  String isPressureChecked = "";
  if (SHOW_PRESSURE)
  {
    isPressureChecked = "checked='checked'";
  }
  form.replace("%PRESSURE_CHECKED%", isPressureChecked);
  server.sendContent(form);

  sendFooter();

  server.sendContent("");
  server.client().stop();
  digitalWrite(externalLight, HIGH);
}

void handleConfigure()
{
  if (!athentication())
  {
    return server.requestAuthentication();
  }
  digitalWrite(externalLight, LOW);
  String html = "";

  server.sendHeader("Cache-Control", "no-cache, no-store");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");

  sendHeader();

  String form = FPSTR(CHANGE_FORM1);
  form.replace("%TIMEDBKEY%", TIMEDBKEY);
  form.replace("%WEATHERKEY%", APIKEY);

  String cityName = "";
  if (weatherClient.getCity(0) != "")
  {
    cityName = weatherClient.getCity(0) + ", " + weatherClient.getCountry(0);
  }
  form.replace("%CITYNAME1%", cityName);
  form.replace("%NAMECITY%", CityName);

  server.sendContent(form);

  form = FPSTR(CHANGE_FORM2);

  String isFlashSecondsChecked = "";
  if (flashOnSeconds)
  {
    isFlashSecondsChecked = "checked='checked'";
  }
  form.replace("%FLASHSECONDS%", isFlashSecondsChecked);

  form.replace("%MSG%", marqueeMessage);
  form.replace("%STARTTIME%", timeDisplayTurnsOn);
  form.replace("%ENDTIME%", timeDisplayTurnsOff);
  server.sendContent(form); // Send another chunk of the form

  form = FPSTR(CHANGE_FORM3);
  form.replace("%INTENSITYOPTIONS%", String(displayIntensity));

  String dSpeed = String(displayScrollSpeed);
  String scrollOptions = "<option value='35'>Yavaş</option><option value='25'>Normal</option><option value='15'>Hızlı</option><option value='10'>Çok Hızlı</option>";
  scrollOptions.replace(dSpeed + "'", dSpeed + "' selected");
  form.replace("%SCROLLOPTIONS%", scrollOptions);

  String minutes = String(minutesBetweenDataRefresh);
  String options = "<option>15</option><option>20</option><option>30</option><option>60</option>";
  options.replace(">" + minutes + "<", " selected>" + minutes + "<");
  form.replace("%OPTIONS%", options);

  form.replace("%REFRESH_DISPLAY%", String(minutesBetweenScrolling));
  form.replace("%THEME_OPTIONS%", themeColor);

  String isUseSecurityChecked = "";
  if (IS_BASIC_AUTH)
  {
    isUseSecurityChecked = "checked='checked'";
  }
  form.replace("%IS_BASICAUTH_CHECKED%", isUseSecurityChecked);

  form.replace("%USERID%", String(www_username));
  form.replace("%STATIONPASSWORD%", String(www_password));

  server.sendContent(form); // Send the second chunk of Data

  sendFooter();

  server.sendContent("");
  server.client().stop();
  digitalWrite(externalLight, HIGH);
}

void handleDisplay()
{
  if (!athentication())
  {
    return server.requestAuthentication();
  }
  enableDisplay(!displayOn);
  String state = "KAPALI";
  if (displayOn)
  {
    state = "ACIK";
  }
  displayMessage("Ekran Durumu " + state);
}

//***********************************************************************
void getWeatherData() // client function to send/receive GET request data..
{
  digitalWrite(externalLight, LOW);
  matrix.fillScreen(LOW); // show black
  Serial.println();

  if (displayOn)
  {
    // only pull the weather data if display is on
    if (firstEpoch != 0)
    {
      centerPrint(hourMinutes(true), true);
    }
    else
    {
      centerPrint("...");
    }
    matrix.drawPixel(0, 7, HIGH);
    matrix.drawPixel(1, 7, HIGH);
    matrix.drawPixel(2, 7, HIGH);
    matrix.write();
    weatherClient.updateWeatherName(CityName, 0);
    delay(1000);
    weatherClient.updateCityAirPollution(weatherClient.getLat(0), weatherClient.getLon(0), 0);
    delay(1000);
    weatherClient.updateSunMoonTime(weatherClient.getCityTimeStamp(0), weatherClient.getLat(0), weatherClient.getLon(0), 0);
    delay(1000);
    if ((weatherClient.getError(0) != "") && ENABLE_SCROLL)
    {
      scrollMessage(weatherClient.getError(0));
    }
  }

  Serial.println("Updating Time...");
  // Update the Time
  matrix.drawPixel(4, 7, HIGH);
  matrix.drawPixel(5, 7, HIGH);
  matrix.drawPixel(6, 7, HIGH);
  Serial.println("matrix Width:" + matrix.width());
  matrix.write();
  TimeDBClient.updateConfig(TIMEDBKEY, weatherClient.getLat(0), weatherClient.getLon(0));
  time_t currentTime = TimeDBClient.getTime();
  if (currentTime > 5000 || firstEpoch == 0)
  {
    setTime(currentTime);
  }
  else
  {
    Serial.println("Time update unsuccessful!");
  }
  lastEpoch = now();
  if (firstEpoch == 0)
  {
    firstEpoch = now();
    Serial.println("firstEpoch: " + String(firstEpoch));
  }

  if (PRAYERS_ENABLED && displayOn)
  {
    delay(1000);
    matrix.drawPixel(8, 7, HIGH);
    matrix.drawPixel(9, 7, HIGH);
    matrix.drawPixel(10, 7, HIGH);
    matrix.write();
    Serial.println("Getting Prayers Time Data...");
    String cityEncoded = encodeHtmlString(CityName);
    prayersClient.updatePrayerTimesAddress(cityEncoded, 0);
    delay(1000);
    String cityEncoded1 = encodeHtmlString(WorldCityName1);
    prayersClient.updatePrayerTimesAddress(cityEncoded1, 1);
    delay(1000);
    String cityEncoded2 = encodeHtmlString(WorldCityName2);
    prayersClient.updatePrayerTimesAddress(cityEncoded2, 2);
    delay(1000);
    String cityEncoded3 = encodeHtmlString(WorldCityName3);
    prayersClient.updatePrayerTimesAddress(cityEncoded3, 3);
    delay(1000);
  }

  if (WORLD_CLOCK_ENABLED && displayOn)
  {
    matrix.drawPixel(12, 7, HIGH);
    matrix.drawPixel(13, 7, HIGH);
    matrix.drawPixel(14, 7, HIGH);
    matrix.write();
    Serial.println("Getting World Time Data...");
    worldWeatherClient.updateWeatherName(WorldCityName1, 1);
    delay(1000);
    timezoneClient.getCityTime(TIMEDBKEY, worldWeatherClient.getLat(1), worldWeatherClient.getLon(1), 1);
    delay(1000);
    timezoneClient.convertTimezone(TIMEDBKEY, TimeDBClient.getZoneName(0), timezoneClient.getZoneName(1), 1);
    delay(1000);
    worldWeatherClient.updateCityAirPollution(worldWeatherClient.getLat(1), worldWeatherClient.getLon(1), 1);
    delay(1000);
    worldWeatherClient.updateSunMoonTime(worldWeatherClient.getCityTimeStamp(1), worldWeatherClient.getLat(1), worldWeatherClient.getLon(1), 1);
    delay(1000);
    worldWeatherClient.updateWeatherName(WorldCityName2, 2);
    delay(1000);
    timezoneClient.getCityTime(TIMEDBKEY, worldWeatherClient.getLat(2), worldWeatherClient.getLon(2), 2);
    delay(1000);
    timezoneClient.convertTimezone(TIMEDBKEY, TimeDBClient.getZoneName(0), timezoneClient.getZoneName(2), 2);
    delay(1000);
    worldWeatherClient.updateCityAirPollution(worldWeatherClient.getLat(2), worldWeatherClient.getLon(2), 2);
    delay(1000);
    worldWeatherClient.updateSunMoonTime(worldWeatherClient.getCityTimeStamp(2), worldWeatherClient.getLat(2), worldWeatherClient.getLon(2), 2);
    delay(1000);
    worldWeatherClient.updateWeatherName(WorldCityName3, 3);
    delay(1000);
    timezoneClient.getCityTime(TIMEDBKEY, worldWeatherClient.getLat(3), worldWeatherClient.getLon(3), 3);
    delay(1000);
    timezoneClient.convertTimezone(TIMEDBKEY, TimeDBClient.getZoneName(0), timezoneClient.getZoneName(3), 3);
    delay(1000);
    worldWeatherClient.updateCityAirPollution(worldWeatherClient.getLat(3), worldWeatherClient.getLon(3), 3);
    delay(1000);
    worldWeatherClient.updateSunMoonTime(worldWeatherClient.getCityTimeStamp(3), worldWeatherClient.getLat(3), worldWeatherClient.getLon(3), 3);
    delay(1000);
  }

  if (CURRENCY_ENABLED && displayOn)
  {
    matrix.drawPixel(16, 7, HIGH);
    matrix.drawPixel(17, 7, HIGH);
    matrix.drawPixel(18, 7, HIGH);
    matrix.write();
    Serial.println("Getting Currency Data...");
    currencyClient.updateCurrency(BaseCurrency1, TargetCurrency, 0);
    delay(1000);
    currencyClient.updateCurrency(BaseCurrency2, TargetCurrency, 1);
    delay(1000);
  }

  Serial.println("Version: " + String(VERSION));
  Serial.println();
  digitalWrite(externalLight, HIGH);
}

void displayMessage(String message)
{
  digitalWrite(externalLight, LOW);

  server.sendHeader("Cache-Control", "no-cache, no-store");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  sendHeader();
  server.sendContent(message);
  sendFooter();
  server.sendContent("");
  server.client().stop();

  digitalWrite(externalLight, HIGH);
}

void redirectHome()
{
  // Send them back to the Root Directory
  server.sendHeader("Location", String("/"), true);
  server.sendHeader("Cache-Control", "no-cache, no-store");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(302, "text/plain", "");
  server.client().stop();
  delay(1000);
}

void sendHeader()
{
  String html = "<!DOCTYPE HTML>";
  html += "<html><head><title>Akıllı Saat</title><link rel='icon' href='data:;base64,='>";
  html += "<meta http-equiv='Content-Type' content='text/html; charset=UTF-8' />";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<link rel='stylesheet' href='https://www.w3schools.com/w3css/4/w3.css'>";
  html += "<link rel='stylesheet' href='https://www.w3schools.com/lib/w3-theme-" + themeColor + ".css'>";
  html += "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.8.1/css/all.min.css'>";
  html += "</head><body>";
  server.sendContent(html);
  html = "<nav class='w3-sidebar w3-bar-block w3-card' style='margin-top:88px' id='mySidebar'>";
  html += "<div class='w3-container w3-theme-d2'>";
  html += "<span onclick='closeSidebar()' class='w3-button w3-display-topright w3-large'><i class='fas fa-times'></i></span>";
  html += "<div class='w3-left'><img src='http://openweathermap.org/img/w/" + weatherClient.getIcon(0) + ".png' alt='" + weatherClient.getDescription(0) + "'></div>";
  html += "<div class='w3-padding'>Menü</div></div>";
  server.sendContent(html);

  server.sendContent(FPSTR(WEB_ACTIONS1));
  Serial.println("Displays: " + String(numberOfHorizontalDisplays));
  if (numberOfHorizontalDisplays >= 8)
  {
    server.sendContent("<a class='w3-bar-item w3-button' href='/configurewideclock'><i class='far fa-clock'></i> Geniş Saat</a>");
  }
  server.sendContent(FPSTR(WEB_ACTIONS2));
  if (displayOn)
  {
    server.sendContent("<i class='fas fa-eye-slash'></i> Ekranı Kapat");
  }
  else
  {
    server.sendContent("<i class='fas fa-eye'></i> Ekranı Aç");
  }
  server.sendContent(FPSTR(WEB_ACTION3));

  html = "</nav>";
  html += "<header class='w3-top w3-bar w3-theme'><button class='w3-bar-item w3-button w3-xxxlarge w3-hover-theme' onclick='openSidebar()'><i class='fas fa-bars'></i></button><h2 class='w3-bar-item'>Akıllı Saat Anasayfa</h2></header>";
  html += "<script>";
  html += "function openSidebar(){document.getElementById('mySidebar').style.display='block'}function closeSidebar(){document.getElementById('mySidebar').style.display='none'}closeSidebar();";
  html += "</script>";
  html += "<br><div class='w3-container w3-large' style='margin-top:88px'>";
  server.sendContent(html);
}

void sendFooter()
{
  Serial.print("WIFI: ");
  Serial.println(WiFi.SSID());
  int8_t rssi = getWifiQuality();
  Serial.print("Signal Strength (RSSI): ");
  Serial.print("%");
  Serial.println(rssi);
  Serial.print("Next Update: ");
  Serial.println(getTimeTillUpdate());
  String html = "<br><br><br>";
  html += "</div>";
  html += "<footer class='w3-container w3-bottom w3-theme w3-margin-top'>";
  html += "<i class='far fa-paper-plane'></i> Versiyon: " + String(VERSION) + "<br>";
  html += "<i class='far fa-clock'></i> Sonraki Veri Güncellemesi: " + getTimeTillUpdate() + "<br>";
  html += "<i class='fas fa-wifi'></i> WIFI: " + WiFi.SSID() + "<br>";
  html += "<i class='fas fa-rss'></i> Sinyal Gücü: %" + String(rssi);
  html += "</footer>";
  html += "</body></html>";
  server.sendContent(html);
}

void displayWeatherData()
{
  digitalWrite(externalLight, LOW);
  String html = "";

  server.sendHeader("Cache-Control", "no-cache, no-store");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  sendHeader();

  String temperature = weatherClient.getTemp(0);

  if ((temperature.indexOf(".") != -1) && (temperature.length() >= (temperature.indexOf(".") + 2)))
  {
    temperature.remove(temperature.indexOf(".") + 2);
  }

  String time = TimeDBClient.getDayName() + ", " + day() + " " + TimeDBClient.getMonthName() + " " + year() + ", " + hour() + ":" + TimeDBClient.zeroPad(minute()) + ":" + TimeDBClient.zeroPad(second());
  String timeClearH = CleanText(time);

  String descriptionH = weatherClient.getDescription(0);
  String descriptionClearH = CleanText(descriptionH);
  String airQualityH = weatherClient.getAqi(0);
  String airQualityClearH = CleanText(airQualityH);

  Serial.println(weatherClient.getCity(0));
  Serial.println(weatherClient.getCountry(0));
  Serial.println(TimeDBClient.getCityName(0));
  Serial.println(TimeDBClient.getRegionName(0));
  Serial.println(TimeDBClient.getCountryCode(0));
  Serial.println(TimeDBClient.getZoneName(0));
  Serial.println(weatherClient.getCondition(0));
  Serial.println(descriptionClearH);
  Serial.println(airQualityClearH);
  Serial.println(temperature);
  Serial.println(timeClearH);

  if (TIMEDBKEY == "")
  {
    html += "<p>Lütfen <a href='/configure'>TimeZoneDB</a>'yi API Anahtarı ile Yapılandırın.</p>";
  }

  if (weatherClient.getCity(0) == "")
  {
    html += "<p>Lütfen <a href='/configure'>Hava Durumunu</a>API Anahtarı ile Yapılandırın.</p>";
    if (weatherClient.getError(0) != "")
    {
      html += "<p>Hava Durumu Hatası: <strong>" + weatherClient.getError(0) + "</strong></p>";
    }
  }
  else
  {
    html += "<div class='w3-cell-row' style='width:100%'><h2><img src='https://flagsapi.com/" + weatherClient.getCountry(0) + "/flat/48.png' alt='" + weatherClient.getCountry(0) + "'>" + "&ensp;" + weatherClient.getCity(0) + ", " + weatherClient.getCountry(0) + " (" + TimeDBClient.getCityName(0) + ", " + TimeDBClient.getRegionName(0) + ", " + TimeDBClient.getCountryCode(0) + ")</h2></div><div class='w3-cell-row'>";
    html += "<div class='w3-cell w3-left w3-medium' style='width:180px'>";
    html += "<img src='http://openweathermap.org/img/w/" + weatherClient.getIcon(0) + ".png' alt='" + weatherClient.getDescription(0) + "'><br>";
    html += "Nem: %" + weatherClient.getHumidity(0) + "<br>";
    html += "Rüzgar: " + weatherClient.getWind(0) + " " + getSpeedSymbol() + "<br>";
    html += "Rüzgar Yönü: " + weatherClient.getDirectionText(0) + "<br>";
    html += "Rüzgar Açısı: " + weatherClient.getDirection(0) + "° <br>";
    html += "Kuvvetli Rüzgar: " + weatherClient.getGust(0) + " " + getSpeedSymbol() + "<br>";
    html += "Basınç: " + weatherClient.getPressure(0) + getPressureSymbol() + "<br>";
    html += "SO2: " + weatherClient.getSO2(0) + " μg/m3<br>";
    html += "NO2: " + weatherClient.getNO2(0) + " μg/m3<br>";
    html += "PM10: " + weatherClient.getPM10(0) + " μg/m3<br>";
    html += "PM2.5: " + weatherClient.getPM2_5(0) + " μg/m3<br>";
    html += "O3: " + weatherClient.getO3(0) + " μg/m3<br>";
    html += "CO: " + weatherClient.getCO(0) + " μg/m3<br>";
    html += "NH3: " + weatherClient.getNH3(0) + "<br>";
    html += "NO: " + weatherClient.getNO(0) + "<br>";
    html += "</div>";
    html += "<div class='w3-cell w3-container' style='width:100%'><p>";
    html += "Hava Durumu: " + weatherClient.getDescription(0) + "<br>";
    html += "Bulutlanma: %" + weatherClient.getCloudcover(0) + "<br>";
    html += "Görüş Uzaklığı: " + weatherClient.getVisibility(0) + " m (" + weatherClient.getVisibilityOtherUnit(0) + " km)<br>";
    html += "Hava Kirliliği: " + weatherClient.getAqi(0) + "<br>";
    html += "Oda Sıcaklığı: " + (String)bmp180.readTemperature() + " °C<br>";
    html += "Sıcaklık: " + temperature + " " + getTempSymbol(true) + " (Hissedilen: " + weatherClient.getFeel(0) + " " + getTempSymbol(true) + ")" + "<br>";
    html += "En Yüksek/Düşük Sıcaklık: " + weatherClient.getHigh(0) + " " + getTempSymbol(true) + " / " + weatherClient.getLow(0) + " " + getTempSymbol(true) + "<br>";
    html += "Gün Doğumu/Batımı: " + weatherClient.getSunrise(0) + " / " + weatherClient.getSunset(0) + " (Aradaki Fark: " + weatherClient.getSunDifference(0) + ")" + "<br>";
    html += "Ay Doğumu/Batımı: " + weatherClient.getMoonRise(0) + " / " + weatherClient.getMoonSet(0) + " (Aradaki Fark: " + weatherClient.getMoonDifference(0) + ")" + "<br>";
    html += "Ayın Görünüşü: " + weatherClient.getMoonPhase(0) + " (Aydınlığı: %" + weatherClient.getMoonIllumination(0) + ")<br>";
    html += "Zaman Dilimi: " + TimeDBClient.getZoneName(0) + "   " + TimeDBClient.getGmtOffsetString(0) + "   (" + TimeDBClient.getAbbreviation(0) + ")" + "<br>";
    html += "Yaz Saati: " + TimeDBClient.useDST(0) + "<br>";
    html += "Yaz Saati Başlangıcı/Bitişi: " + TimeDBClient.getZoneStart(0) + " / " + TimeDBClient.getZoneEnd(0) + "<br>";
    html += "Bir Sonraki Zaman Dilimi Kısaltması: " + TimeDBClient.getNextAbbreviation(0) + "<br>";
    html += "Şuanki Tarih ve Saat: " + time + "<br>";
    html += "Son Güncelleme: " + serverLastUpdateString + "<br>";
    html += "<a href='https://www.google.com/maps/@" + weatherClient.getLat(0) + "," + weatherClient.getLon(0) + ",10000m/data=!3m1!1e3' target='_BLANK'><i class='fas fa-map-marker' style='color:red'></i> Haritala!</a><br>";
    html += "</p></div></div><hr>";
  }

  server.sendContent(String(html)); // spit out what we got
  html = "";                        // fresh start

  if (OCTOPRINT_ENABLED)
  {
    html = "<div class='w3-cell-row'><b>OctoPrint İlerlemesi:</b> ";
    if (printerClient.isPrinting())
    {
      int val = printerClient.getProgressPrintTimeLeft().toInt();
      int hours = numberOfHours(val);
      int minutes = numberOfMinutes(val);
      int seconds = numberOfSeconds(val);
      html += "Çevrimiçi ve Basılıyor</br>Tahmini Kalan Baskı Süresi: " + zeroPad(hours) + ":" + zeroPad(minutes) + ":" + zeroPad(seconds) + "<br>";

      val = printerClient.getProgressPrintTime().toInt();
      hours = numberOfHours(val);
      minutes = numberOfMinutes(val);
      seconds = numberOfSeconds(val);
      html += "Baskı Süresi: " + zeroPad(hours) + ":" + zeroPad(minutes) + ":" + zeroPad(seconds) + "<br>";
      html += printerClient.getState() + " " + printerClient.getFileName() + "</br>";
      html += "<style>#myProgress {width: 100%;background-color: #ddd;}#myBar {width: " + printerClient.getProgressCompletion() + "%;height: 30px;background-color: #4CAF50;}</style>";
      html += "<div id=\"myProgress\"><div id=\"myBar\" class=\"w3-medium w3-center\">" + printerClient.getProgressCompletion() + "%</div></div>";
    }
    else if (printerClient.isOperational())
    {
      html += printerClient.getState();
    }
    else if (printerClient.getError() != "")
    {
      html += printerClient.getError();
    }
    else
    {
      html += "Bağlı Değil";
    }
    html += "</div><br><hr>";
    server.sendContent(String(html));
    html = "";
  }

  if (USE_PIHOLE)
  {
    if (piholeClient.getError() == "")
    {
      html = "<div class='w3-cell-row'><b>Pi-hole</b><br>";
      html += "Toplam Sorgu Sayısı (" + piholeClient.getUniqueClients() + " istemciler): <b>" + piholeClient.getDnsQueriesToday() + "</b><br>";
      html += "Engellenen Sorgular: <b>" + piholeClient.getAdsBlockedToday() + "</b><br>";
      html += "Engellenen (Yüzde): <b>" + piholeClient.getAdsPercentageToday() + "%</b><br>";
      html += "Blok Listesindeki Alanlar: <b>" + piholeClient.getDomainsBeingBlocked() + "</b><br>";
      html += "Durumu: <b>" + piholeClient.getPiHoleStatus() + "</b><br>";
      html += "</div><br><hr>";
    }
    else
    {
      html = "<div class='w3-cell-row'>Pi-hole Hatası";
      html += "Lütfen <a href='/configurepihole' title='Configure'>Pi-hole Ayarlarını</a> Yapınız <a href='/configurepihole' title='Configure'><i class='fas fa-cog'></i></a><br>";
      html += "Durum: Veri Alınırken Hata Oluştu<br>";
      html += "Sebebi: " + piholeClient.getError() + "<br></div><br><hr>";
    }
    server.sendContent(html);
    html = "";
  }

  sendFooter();
  server.sendContent("");
  server.client().stop();
  digitalWrite(externalLight, HIGH);
}

void displayWorldClockData()
{
  digitalWrite(externalLight, LOW);
  String html = "";

  server.sendHeader("Cache-Control", "no-cache, no-store");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  sendHeader();

  if (WORLD_CLOCK_ENABLED)
  {
    if (worldWeatherClient.getError(1) == "")
    {
      html += "<div class='w3-cell-row' style='width:100%'><h2><img src='https://flagsapi.com/" + worldWeatherClient.getCountry(1) + "/flat/48.png' alt='" + worldWeatherClient.getCountry(1) + "'>" + "&ensp;" + worldWeatherClient.getCity(1) + ", " + worldWeatherClient.getCountry(1) + " (" + timezoneClient.getCityName(1) + ", " + timezoneClient.getRegionName(1) + ", " + timezoneClient.getCountryCode(1) + ")</h2></div><div class='w3-cell-row'>";
      html += "Tarih ve Saat: " + timezoneClient.getTimestamp2Date(1) + "<br>";
      html += "Zaman Dilimi: " + timezoneClient.getZoneName(1) + "   " + timezoneClient.getGmtOffsetString(1) + "   (" + timezoneClient.getToAbbreviation(1) + ")" + "<br>";
      html += TimeDBClient.getZoneName(0) + " İle Arasındaki Zaman Farkı: " + timezoneClient.getOffsetDifferenceString(1) + "<br>";
      html += "Yaz Saati: " + timezoneClient.useDST(1) + "<br>";
      html += "Yaz Saati Başlangıcı/Bitişi: " + timezoneClient.getZoneStart(1) + " / " + timezoneClient.getZoneEnd(1) + "<br>";
      html += "Bir Sonraki Zaman Dilimi Kısaltması: " + timezoneClient.getNextAbbreviation(1) + "<br>";
      html += "<a href='https://www.google.com/maps/@" + worldWeatherClient.getLat(1) + "," + worldWeatherClient.getLon(1) + ",10000m/data=!3m1!1e3' target='_BLANK'><i class='fas fa-map-marker' style='color:red'></i> Haritala!</a><br>";
      html += "</p></div><hr>";
    }
    else
    {
      html = "<div class='w3-cell-row'>Dünya Saatleri Hatası";
      html += "<p>Lütfen <a href='/configureworldclock' Dünya Saatleri Ayarlarını</a> Yapınız</p><br>";
      html += "Sebebi: <strong>" + worldWeatherClient.getError(1) + "</strong><br></div><br>";
    }
    if (worldWeatherClient.getError(2) == "")
    {
      html += "<div class='w3-cell-row' style='width:100%'><h2><img src='https://flagsapi.com/" + worldWeatherClient.getCountry(2) + "/flat/48.png' alt='" + worldWeatherClient.getCountry(2) + "'>" + "&ensp;" + worldWeatherClient.getCity(2) + ", " + worldWeatherClient.getCountry(2) + " (" + timezoneClient.getCityName(2) + ", " + timezoneClient.getRegionName(2) + ", " + timezoneClient.getCountryCode(2) + ")</h2></div><div class='w3-cell-row'>";
      html += "Tarih ve Saat: " + timezoneClient.getTimestamp2Date(2) + "<br>";
      html += "Zaman Dilimi: " + timezoneClient.getZoneName(2) + "   " + timezoneClient.getGmtOffsetString(2) + "   (" + timezoneClient.getToAbbreviation(2) + ")" + "<br>";
      html += TimeDBClient.getZoneName(0) + " İle Arasındaki Zaman Farkı: " + timezoneClient.getOffsetDifferenceString(2) + "<br>";
      html += "Yaz Saati: " + timezoneClient.useDST(2) + "<br>";
      html += "Yaz Saati Başlangıcı/Bitişi: " + timezoneClient.getZoneStart(2) + " / " + timezoneClient.getZoneEnd(2) + "<br>";
      html += "Bir Sonraki Zaman Dilimi Kısaltması: " + timezoneClient.getNextAbbreviation(2) + "<br>";
      html += "<a href='https://www.google.com/maps/@" + worldWeatherClient.getLat(2) + "," + worldWeatherClient.getLon(2) + ",10000m/data=!3m1!1e3' target='_BLANK'><i class='fas fa-map-marker' style='color:red'></i> Haritala!</a><br>";
      html += "</p></div><hr>";
    }
    else
    {
      html = "<div class='w3-cell-row'>Dünya Saatleri Hatası";
      html += "<p>Lütfen <a href='/configureworldclock' Dünya Saatleri Ayarlarını</a> Yapınız</p><br>";
      html += "Sebebi: <strong>" + worldWeatherClient.getError(2) + "</strong><br></div><br>";
    }
    if (worldWeatherClient.getError(3) == "")
    {
      html += "<div class='w3-cell-row' style='width:100%'><h2><img src='https://flagsapi.com/" + worldWeatherClient.getCountry(3) + "/flat/48.png' alt='" + worldWeatherClient.getCountry(3) + "'>" + "&ensp;" + worldWeatherClient.getCity(3) + ", " + worldWeatherClient.getCountry(3) + " (" + timezoneClient.getCityName(3) + ", " + timezoneClient.getRegionName(3) + ", " + timezoneClient.getCountryCode(3) + ")</h2></div><div class='w3-cell-row'>";
      html += "Tarih ve Saat: " + timezoneClient.getTimestamp2Date(3) + "<br>";
      html += "Zaman Dilimi: " + timezoneClient.getZoneName(3) + "   " + timezoneClient.getGmtOffsetString(3) + "   (" + timezoneClient.getToAbbreviation(3) + ")" + "<br>";
      html += TimeDBClient.getZoneName(0) + " İle Arasındaki Zaman Farkı: " + timezoneClient.getOffsetDifferenceString(3) + "<br>";
      html += "Yaz Saati: " + timezoneClient.useDST(3) + "<br>";
      html += "Yaz Saati Başlangıcı/Bitişi: " + timezoneClient.getZoneStart(3) + " / " + timezoneClient.getZoneEnd(3) + "<br>";
      html += "Bir Sonraki Zaman Dilimi Kısaltması: " + timezoneClient.getNextAbbreviation(3) + "<br>";
      html += "<a href='https://www.google.com/maps/@" + worldWeatherClient.getLat(3) + "," + worldWeatherClient.getLon(3) + ",10000m/data=!3m1!1e3' target='_BLANK'><i class='fas fa-map-marker' style='color:red'></i> Haritala!</a><br>";
      html += "</p></div><hr>";
    }
    else
    {
      html = "<div class='w3-cell-row'>Dünya Saatleri Hatası";
      html += "<p>Lütfen <a href='/configureworldclock' Dünya Saatleri Ayarlarını</a> Yapınız</p><br>";
      html += "Sebebi: <strong>" + worldWeatherClient.getError(3) + "</strong><br></div><br>";
    }
    server.sendContent(String(html));
    html = "";
  }
  else
  {
    html += "Dünya Saatleri Özelliği Devre Dışı";
    server.sendContent(String(html));
    html = "";
  }

  sendFooter();
  server.sendContent("");
  server.client().stop();
  digitalWrite(externalLight, HIGH);
}

void displayWorldClockWeatherData()
{
  digitalWrite(externalLight, LOW);
  String html = "";

  server.sendHeader("Cache-Control", "no-cache, no-store");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  sendHeader();

  if (WORLD_CLOCK_ENABLED)
  {
    if (worldWeatherClient.getError(1) == "")
    {
      html += "<div class='w3-cell-row' style='width:100%'><h2><img src='https://flagsapi.com/" + worldWeatherClient.getCountry(1) + "/flat/48.png' alt='" + worldWeatherClient.getCountry(1) + "'>" + "&ensp;" + worldWeatherClient.getCity(1) + ", " + worldWeatherClient.getCountry(1) + " (" + timezoneClient.getCityName(1) + ", " + timezoneClient.getRegionName(1) + ", " + timezoneClient.getCountryCode(1) + ")</h2></div><div class='w3-cell-row'>";
      html += "Hava Durumu: " + worldWeatherClient.getDescription(1) + " " + "<img src='http://openweathermap.org/img/w/" + worldWeatherClient.getIcon(1) + ".png' alt='" + worldWeatherClient.getDescription(1) + "'>" + " | " + "Bulutlanma: %" + worldWeatherClient.getCloudcover(1) + " | " + "Nem: %" + worldWeatherClient.getHumidity(1) + " | " + "Rüzgar: " + worldWeatherClient.getWind(1) + " " + getSpeedSymbol() + " " + worldWeatherClient.getDirectionText(1) + " " + worldWeatherClient.getDirection(1) + "°" + " | " + "Kuvvetli Rüzgar: " + worldWeatherClient.getGust(1) + " " + getSpeedSymbol() + " | " + "Hava Kirliliği: " + worldWeatherClient.getAqi(1) + " | " + "Basınç: " + worldWeatherClient.getPressure(1) + getPressureSymbol() + "<br>";
      html += "Görüş Uzaklığı: " + worldWeatherClient.getVisibility(1) + " m (" + worldWeatherClient.getVisibilityOtherUnit(1) + " km)<br>";
      html += "Sıcaklık: " + worldWeatherClient.getTemp(1) + " " + getTempSymbol(true) + " (Hissedilen: " + worldWeatherClient.getFeel(1) + " " + getTempSymbol(true) + ")" + " | " + "En Yüksek/Düşük Sıcaklık: " + worldWeatherClient.getHigh(1) + " " + getTempSymbol(true) + " / " + worldWeatherClient.getLow(1) + " " + getTempSymbol(true) + "<br>";
      html += "Gün Doğumu/Batımı: " + worldWeatherClient.getSunrise(1) + " / " + worldWeatherClient.getSunset(1) + " (Aradaki Fark: " + worldWeatherClient.getSunDifference(1) + ")" + "<br>";
      html += "Ay Doğumu/Batımı: " + worldWeatherClient.getMoonRise(1) + " / " + worldWeatherClient.getMoonSet(1) + " (Aradaki Fark: " + worldWeatherClient.getMoonDifference(1) + ")" + "<br>";
      html += "Ayın Görünüşü: " + worldWeatherClient.getMoonPhase(1) + " (Aydınlığı: %" + worldWeatherClient.getMoonIllumination(1) + ")<br>";
      html += "<a href='https://www.google.com/maps/@" + worldWeatherClient.getLat(1) + "," + worldWeatherClient.getLon(1) + ",10000m/data=!3m1!1e3' target='_BLANK'><i class='fas fa-map-marker' style='color:red'></i> Haritala!</a><br>";
      html += "</p></div><hr>";
    }
    else
    {
      html = "<div class='w3-cell-row'>Dünya Saatleri Hatası";
      html += "<p>Lütfen <a href='/configureworldclock' Dünya Saatleri Ayarlarını</a> Yapınız</p><br>";
      html += "Sebebi: <strong>" + worldWeatherClient.getError(1) + "</strong><br></div><br>";
    }
    if (worldWeatherClient.getError(2) == "")
    {
      html += "<div class='w3-cell-row' style='width:100%'><h2><img src='https://flagsapi.com/" + worldWeatherClient.getCountry(2) + "/flat/48.png' alt='" + worldWeatherClient.getCountry(2) + "'>" + "&ensp;" + worldWeatherClient.getCity(2) + ", " + worldWeatherClient.getCountry(2) + " (" + timezoneClient.getCityName(2) + ", " + timezoneClient.getRegionName(2) + ", " + timezoneClient.getCountryCode(2) + ")</h2></div><div class='w3-cell-row'>";
      html += "Hava Durumu: " + worldWeatherClient.getDescription(2) + " " + "<img src='http://openweathermap.org/img/w/" + worldWeatherClient.getIcon(2) + ".png' alt='" + worldWeatherClient.getDescription(2) + "'>" + " | " + "Bulutlanma: %" + worldWeatherClient.getCloudcover(2) + " | " + "Nem: %" + worldWeatherClient.getHumidity(2) + " | " + "Rüzgar: " + worldWeatherClient.getWind(2) + " " + getSpeedSymbol() + " " + worldWeatherClient.getDirectionText(2) + " " + worldWeatherClient.getDirection(2) + "°" + " | " + "Kuvvetli Rüzgar: " + worldWeatherClient.getGust(2) + " " + getSpeedSymbol() + " | " + "Hava Kirliliği: " + worldWeatherClient.getAqi(2) + " | " + "Basınç: " + worldWeatherClient.getPressure(2) + getPressureSymbol() + "<br>";
      html += "Görüş Uzaklığı: " + worldWeatherClient.getVisibility(2) + " m (" + worldWeatherClient.getVisibilityOtherUnit(2) + " km)<br>";
      html += "Sıcaklık: " + worldWeatherClient.getTemp(2) + " " + getTempSymbol(true) + " (Hissedilen: " + worldWeatherClient.getFeel(2) + " " + getTempSymbol(true) + ")" + " | " + "En Yüksek/Düşük Sıcaklık: " + worldWeatherClient.getHigh(2) + " " + getTempSymbol(true) + " / " + worldWeatherClient.getLow(2) + " " + getTempSymbol(true) + "<br>";
      html += "Gün Doğumu/Batımı: " + worldWeatherClient.getSunrise(2) + " / " + worldWeatherClient.getSunset(2) + " (Aradaki Fark: " + worldWeatherClient.getSunDifference(2) + ")" + "<br>";
      html += "Ay Doğumu/Batımı: " + worldWeatherClient.getMoonRise(2) + " / " + worldWeatherClient.getMoonSet(2) + " (Aradaki Fark: " + worldWeatherClient.getMoonDifference(2) + ")" + "<br>";
      html += "Ayın Görünüşü: " + worldWeatherClient.getMoonPhase(2) + " (Aydınlığı: %" + worldWeatherClient.getMoonIllumination(2) + ")<br>";
      html += "<a href='https://www.google.com/maps/@" + worldWeatherClient.getLat(2) + "," + worldWeatherClient.getLon(2) + ",10000m/data=!3m1!1e3' target='_BLANK'><i class='fas fa-map-marker' style='color:red'></i> Haritala!</a><br>";
      html += "</p></div><hr>";
    }
    else
    {
      html = "<div class='w3-cell-row'>Dünya Saatleri Hatası";
      html += "<p>Lütfen <a href='/configureworldclock' Dünya Saatleri Ayarlarını</a> Yapınız</p><br>";
      html += "Sebebi: <strong>" + worldWeatherClient.getError(2) + "</strong><br></div><br>";
    }
    if (worldWeatherClient.getError(3) == "")
    {
      html += "<div class='w3-cell-row' style='width:100%'><h2><img src='https://flagsapi.com/" + worldWeatherClient.getCountry(3) + "/flat/48.png' alt='" + worldWeatherClient.getCountry(3) + "'>" + "&ensp;" + worldWeatherClient.getCity(3) + ", " + worldWeatherClient.getCountry(3) + " (" + timezoneClient.getCityName(3) + ", " + timezoneClient.getRegionName(3) + ", " + timezoneClient.getCountryCode(3) + ")</h2></div><div class='w3-cell-row'>";
      html += "Hava Durumu: " + worldWeatherClient.getDescription(3) + " " + "<img src='http://openweathermap.org/img/w/" + worldWeatherClient.getIcon(3) + ".png' alt='" + worldWeatherClient.getDescription(3) + "'>" + " | " + "Bulutlanma: %" + worldWeatherClient.getCloudcover(3) + " | " + "Nem: %" + worldWeatherClient.getHumidity(3) + " | " + "Rüzgar: " + worldWeatherClient.getWind(3) + " " + getSpeedSymbol() + " " + worldWeatherClient.getDirectionText(3) + " " + worldWeatherClient.getDirection(3) + "°" + " | " + "Kuvvetli Rüzgar: " + worldWeatherClient.getGust(3) + " " + getSpeedSymbol() + " | " + "Hava Kirliliği: " + worldWeatherClient.getAqi(3) + " | " + "Basınç: " + worldWeatherClient.getPressure(3) + getPressureSymbol() + "<br>";
      html += "Görüş Uzaklığı: " + worldWeatherClient.getVisibility(3) + " m (" + worldWeatherClient.getVisibilityOtherUnit(3) + " km)<br>";
      html += "Sıcaklık: " + worldWeatherClient.getTemp(3) + " " + getTempSymbol(true) + " (Hissedilen: " + worldWeatherClient.getFeel(3) + " " + getTempSymbol(true) + ")" + " | " + "En Yüksek/Düşük Sıcaklık: " + worldWeatherClient.getHigh(3) + " " + getTempSymbol(true) + " / " + worldWeatherClient.getLow(3) + " " + getTempSymbol(true) + "<br>";
      html += "Gün Doğumu/Batımı: " + worldWeatherClient.getSunrise(3) + " / " + worldWeatherClient.getSunset(3) + " (Aradaki Fark: " + worldWeatherClient.getSunDifference(3) + ")" + "<br>";
      html += "Ay Doğumu/Batımı: " + worldWeatherClient.getMoonRise(3) + " / " + worldWeatherClient.getMoonSet(3) + " (Aradaki Fark: " + worldWeatherClient.getMoonDifference(3) + ")" + "<br>";
      html += "Ayın Görünüşü: " + worldWeatherClient.getMoonPhase(3) + " (Aydınlığı: %" + worldWeatherClient.getMoonIllumination(3) + ")<br>";
      html += "<a href='https://www.google.com/maps/@" + worldWeatherClient.getLat(3) + "," + worldWeatherClient.getLon(3) + ",10000m/data=!3m1!1e3' target='_BLANK'><i class='fas fa-map-marker' style='color:red'></i> Haritala!</a><br>";
      html += "</p></div><hr>";
    }
    else
    {
      html = "<div class='w3-cell-row'>Dünya Saatleri Hatası";
      html += "<p>Lütfen <a href='/configureworldclock' Dünya Saatleri Ayarlarını</a> Yapınız</p><br>";
      html += "Sebebi: <strong>" + worldWeatherClient.getError(3) + "</strong><br></div><br>";
    }
    server.sendContent(String(html));
    html = "";
  }
  else
  {
    html += "<strong>Dünya Saatleri Özelliği Devre Dışı</strong><br>";
    server.sendContent(String(html));
    html = "";
  }

  sendFooter();
  server.sendContent("");
  server.client().stop();
  digitalWrite(externalLight, HIGH);
}

void displayPrayersTimeData()
{
  digitalWrite(externalLight, LOW);
  String html = "";

  server.sendHeader("Cache-Control", "no-cache, no-store");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  sendHeader();

  if (PRAYERS_ENABLED)
  {
    if (prayersClient.getError(0) == "")
    {
      String praySource = prayersClient.getMethodName(0);
      String praySourceClear = prayersClient.cleanText(praySource);

      html += "<div class='w3-cell-row' style='width:100%'><h2><img src='https://flagsapi.com/" + weatherClient.getCountry(0) + "/flat/48.png' alt='" + weatherClient.getCountry(0) + "'>" + "&ensp;" + "Namaz Vakitleri (" + TimeDBClient.getCityName(0) + ", " + TimeDBClient.getRegionName(0) + ", " + TimeDBClient.getCountryCode(0) + ")</h2></div><div class='w3-cell-row'><p>";
      html += "Kaynak: " + praySourceClear + " / ID: " + prayersMethod + "<br>";
      html += "Hicri/Miladi Takvime Göre Tarih: " + prayersClient.getHijriDate(0) + " / " + prayersClient.getGregorianDate(0) + "<br>";
      html += "İmsak: " + prayersClient.getFajr(0) + "<br>";
      html += "Güneş: " + prayersClient.getSunrise(0) + "<br>";
      html += "Öğle Namazı: " + prayersClient.getDhuhr(0) + "<br>";
      html += "İkindi Namazı: " + prayersClient.getAsr(0) + "<br>";
      html += "Akşam Namazı: " + prayersClient.getMaghrib(0) + "<br>";
      html += "Yatsı Namazı: " + prayersClient.getIsha(0) + "<br>";
      html += "</p></div><hr>";
    }
    else
    {
      html = "<div class='w3-cell-row'>Namaz Vakitleri Hatası";
      html += "<p>Lütfen <a href='/configureprayers'>Namaz Vakitleri Ayarlarını</a> Yapınız<p><br>";
      html += "Sebebi: <strong>" + prayersClient.getError(0) + "</strong><br></div><br><hr>";
    }
    if (prayersClient.getError(1) == "")
    {
      String praySource = prayersClient.getMethodName(1);
      String praySourceClear = prayersClient.cleanText(praySource);

      html += "<div class='w3-cell-row' style='width:100%'><h2><img src='https://flagsapi.com/" + worldWeatherClient.getCountry(1) + "/flat/48.png' alt='" + worldWeatherClient.getCountry(1) + "'>" + "&ensp;" + "Namaz Vakitleri (" + timezoneClient.getCityName(1) + ", " + timezoneClient.getRegionName(1) + ", " + timezoneClient.getCountryCode(1) + ")</h2></div><div class='w3-cell-row'><p>";
      html += "Kaynak: " + praySourceClear + " / ID: " + prayersMethod + "<br>";
      html += "Hicri/Miladi Takvime Göre Tarih: " + prayersClient.getHijriDate(1) + " / " + prayersClient.getGregorianDate(1) + "<br>";
      html += "İmsak: " + prayersClient.getFajr(1) + "<br>";
      html += "Güneş: " + prayersClient.getSunrise(1) + "<br>";
      html += "Öğle Namazı: " + prayersClient.getDhuhr(1) + "<br>";
      html += "İkindi Namazı: " + prayersClient.getAsr(1) + "<br>";
      html += "Akşam Namazı: " + prayersClient.getMaghrib(1) + "<br>";
      html += "Yatsı Namazı: " + prayersClient.getIsha(1) + "<br>";
      html += "</p></div><hr>";
    }
    else
    {
      html = "<div class='w3-cell-row'>Namaz Vakitleri Hatası";
      html += "<p>Lütfen <a href='/configureprayers'>Namaz Vakitleri Ayarlarını</a> Yapınız<p><br>";
      html += "Sebebi: <strong>" + prayersClient.getError(1) + "</strong><br></div><br><hr>";
    }
    if (prayersClient.getError(2) == "")
    {
      String praySource = prayersClient.getMethodName(2);
      String praySourceClear = prayersClient.cleanText(praySource);

      html += "<div class='w3-cell-row' style='width:100%'><h2><img src='https://flagsapi.com/" + worldWeatherClient.getCountry(2) + "/flat/48.png' alt='" + worldWeatherClient.getCountry(2) + "'>" + "&ensp;" + "Namaz Vakitleri (" + timezoneClient.getCityName(2) + ", " + timezoneClient.getRegionName(2) + ", " + timezoneClient.getCountryCode(2) + ")</h2></div><div class='w3-cell-row'><p>";
      html += "Kaynak: " + praySourceClear + " / ID: " + prayersMethod + "<br>";
      html += "Hicri/Miladi Takvime Göre Tarih: " + prayersClient.getHijriDate(2) + " / " + prayersClient.getGregorianDate(2) + "<br>";
      html += "İmsak: " + prayersClient.getFajr(2) + "<br>";
      html += "Güneş: " + prayersClient.getSunrise(2) + "<br>";
      html += "Öğle Namazı: " + prayersClient.getDhuhr(2) + "<br>";
      html += "İkindi Namazı: " + prayersClient.getAsr(2) + "<br>";
      html += "Akşam Namazı: " + prayersClient.getMaghrib(2) + "<br>";
      html += "Yatsı Namazı: " + prayersClient.getIsha(2) + "<br>";
      html += "</p></div><hr>";
    }
    else
    {
      html = "<div class='w3-cell-row'>Namaz Vakitleri Hatası";
      html += "<p>Lütfen <a href='/configureprayers'>Namaz Vakitleri Ayarlarını</a> Yapınız<p><br>";
      html += "Sebebi: <strong>" + prayersClient.getError(2) + "</strong><br></div><br><hr>";
    }
    if (prayersClient.getError(3) == "")
    {
      String praySource = prayersClient.getMethodName(3);
      String praySourceClear = prayersClient.cleanText(praySource);

      html += "<div class='w3-cell-row' style='width:100%'><h2><img src='https://flagsapi.com/" + worldWeatherClient.getCountry(3) + "/flat/48.png' alt='" + worldWeatherClient.getCountry(3) + "'>" + "&ensp;" + "Namaz Vakitleri (" + timezoneClient.getCityName(3) + ", " + timezoneClient.getRegionName(3) + ", " + timezoneClient.getCountryCode(3) + ")</h2></div><div class='w3-cell-row'><p>";
      html += "Kaynak: " + praySourceClear + " / ID: " + prayersMethod + "<br>";
      html += "Hicri/Miladi Takvime Göre Tarih: " + prayersClient.getHijriDate(3) + " / " + prayersClient.getGregorianDate(3) + "<br>";
      html += "İmsak: " + prayersClient.getFajr(3) + "<br>";
      html += "Güneş: " + prayersClient.getSunrise(3) + "<br>";
      html += "Öğle Namazı: " + prayersClient.getDhuhr(3) + "<br>";
      html += "İkindi Namazı: " + prayersClient.getAsr(3) + "<br>";
      html += "Akşam Namazı: " + prayersClient.getMaghrib(3) + "<br>";
      html += "Yatsı Namazı: " + prayersClient.getIsha(3) + "<br>";
      html += "</p></div><hr>";
    }
    else
    {
      html = "<div class='w3-cell-row'>Namaz Vakitleri Hatası";
      html += "<p>Lütfen <a href='/configureprayers'>Namaz Vakitleri Ayarlarını</a> Yapınız<p><br>";
      html += "Sebebi: <strong>" + prayersClient.getError(3) + "</strong><br></div><br><hr>";
    }
    server.sendContent(String(html));
    html = "";
  }
  else
  {
    html += "<strong>Namaz Vakitleri Özelliği Devre Dışı</strong><br>";
    server.sendContent(String(html));
    html = "";
  }
  sendFooter();
  server.sendContent("");
  server.client().stop();
  digitalWrite(externalLight, HIGH);
}

void displayCurrencyData()
{
  digitalWrite(externalLight, LOW);
  String html = "";

  server.sendHeader("Cache-Control", "no-cache, no-store");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  sendHeader();

  if (CURRENCY_ENABLED)
  {
    if (currencyClient.getError(0) == "")
    {
      html += "1 " + currencyClient.getBaseCurrencyName(0) + " = " + currencyClient.getTargetCurrencyFormatted(0) + " " + currencyClient.getTargetCurrencyName(0) + " (Son Güncelleme: " + currencyClient.getRequestTime(0) + ")" + "<br>";
      html += "1 " + currencyClient.getTargetCurrencyName(0) + " = " + currencyClient.getInverseCurrencyFormatted(0) + " " + currencyClient.getBaseCurrencyName(0) + " (Son Güncelleme: " + currencyClient.getRequestTime(0) + ")" + "<br>";
      html += "<hr>";
    }
    else
    {
      html = "<div class='w3-cell-row'>Döviz Kurları Hatası";
      html += "<p>Lütfen <a href='/configurecurrency' Döviz Kurları Ayarlarını</a> Yapınız</p><br>";
      html += "Sebebi: <strong>" + currencyClient.getError(0) + "</strong><br></div><br>";
    }
    if (currencyClient.getError(1) == "")
    {
      html += "1 " + currencyClient.getBaseCurrencyName(1) + " = " + currencyClient.getTargetCurrencyFormatted(1) + " " + currencyClient.getTargetCurrencyName(1) + " (Son Güncelleme: " + currencyClient.getRequestTime(1) + ")" + "<br>";
      html += "1 " + currencyClient.getTargetCurrencyName(1) + " = " + currencyClient.getInverseCurrencyFormatted(1) + " " + currencyClient.getBaseCurrencyName(1) + " (Son Güncelleme: " + currencyClient.getRequestTime(1) + ")" + "<br>";
      html += "<hr>";
    }
    else
    {
      html = "<div class='w3-cell-row'>Döviz Kurları Hatası";
      html += "<p>Lütfen <a href='/configurecurrency' Döviz Kurları Ayarlarını</a> Yapınız</p><br>";
      html += "Sebebi: <strong>" + currencyClient.getError(1) + "</strong><br></div><br>";
    }
    server.sendContent(String(html));
    html = "";
  }
  else
  {
    html += "Döviz Kurları Özelliği Devre Dışı";
    server.sendContent(String(html));
    html = "";
  }

  sendFooter();
  server.sendContent("");
  server.client().stop();
  digitalWrite(externalLight, HIGH);
}

void configModeCallback(WiFiManager *myWiFiManager)
{
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println("Wifi Manager");
  Serial.println("Please connect to AP");
  Serial.println(myWiFiManager->getConfigPortalSSID());
  Serial.println("To setup Wifi Configuration");
  scrollMessage("Lutfen AP'ye Baglanin: " + String(myWiFiManager->getConfigPortalSSID()));
  centerPrint("wifi");
}

void flashLED(int number, int delayTime)
{
  for (int inx = 0; inx < number; inx++)
  {
    delay(delayTime);
    digitalWrite(externalLight, LOW);
    delay(delayTime);
    digitalWrite(externalLight, HIGH);
    delay(delayTime);
  }
}

String getTempSymbol()
{
  return getTempSymbol(false);
}

String getTempSymbol(bool forWeb)
{
  String rtnValue = "F";
  if (IS_METRIC)
  {
    rtnValue = "C";
  }
  if (forWeb)
  {
    rtnValue = "°" + rtnValue;
  }
  else
  {
    rtnValue = char(247) + rtnValue;
  }
  return rtnValue;
}

String getSpeedSymbol()
{
  String rtnValue = "mph";
  if (IS_METRIC)
  {
    rtnValue = "km/h";
  }
  return rtnValue;
}

String getPressureSymbol()
{
  String rtnValue = "";
  if (IS_METRIC)
  {
    rtnValue = "mb";
  }
  return rtnValue;
}

// converts the dBm to a range between 0 and 100%
int8_t getWifiQuality()
{
  int32_t dbm = WiFi.RSSI();
  if (dbm <= -100)
  {
    return 0;
  }
  else if (dbm >= -50)
  {
    return 100;
  }
  else
  {
    return 2 * (dbm + 100);
  }
}

String getTimeTillUpdate()
{
  String rtnValue = "";

  long timeToUpdate = (((minutesBetweenDataRefresh * 60) + lastEpoch) - now());

  int hours = numberOfHours(timeToUpdate);
  int minutes = numberOfMinutes(timeToUpdate);
  int seconds = numberOfSeconds(timeToUpdate);

  rtnValue += String(hours) + ":";
  if (minutes < 10)
  {
    rtnValue += "0";
  }
  rtnValue += String(minutes) + ":";
  if (seconds < 10)
  {
    rtnValue += "0";
  }
  rtnValue += String(seconds);

  return rtnValue;
}

int getMinutesFromLastRefresh()
{
  int minutes = (now() - lastEpoch) / 60;
  return minutes;
}

int getMinutesFromLastDisplay()
{
  int minutes = (now() - displayOffEpoch) / 60;
  return minutes;
}

void enableDisplay(boolean enable)
{
  displayOn = enable;
  if (enable)
  {
    if (getMinutesFromLastDisplay() >= minutesBetweenDataRefresh)
    {
      // The display has been off longer than the minutes between refresh -- need to get fresh data
      lastEpoch = 0;       // this should force a data pull of the weather
      displayOffEpoch = 0; // reset
    }
    matrix.shutdown(false);
    matrix.fillScreen(LOW); // clear screen
    Serial.println("Display was turned ON: " + now());
  }
  else
  {
    matrix.shutdown(true);
    Serial.println("Display was turned OFF: " + now());
    displayOffEpoch = lastEpoch;
  }
}

// Toggle on and off the display if user defined times
void checkDisplay()
{
  if (timeDisplayTurnsOn == "" || timeDisplayTurnsOff == "")
  {
    return; // nothing to do
  }
  String currentTime = TimeDBClient.zeroPad(hour()) + ":" + TimeDBClient.zeroPad(minute());

  if (currentTime == timeDisplayTurnsOn && !displayOn)
  {
    Serial.println("Time to turn display on: " + currentTime);
    flashLED(1, 500);
    enableDisplay(true);
  }

  if (currentTime == timeDisplayTurnsOff && displayOn)
  {
    Serial.println("Time to turn display off: " + currentTime);
    flashLED(2, 500);
    enableDisplay(false);
  }
}

String writeCityIds()
{
  // Save decoded message to LittleFS file for playback on power up.
  File f = LittleFS.open(CONFIG, "w");
  if (!f)
  {
    Serial.println("File open failed!");
  }
  else
  {
    Serial.println("Saving settings now...");
    f.println("TIMEDBKEY=" + TIMEDBKEY);
    f.println("APIKEY=" + APIKEY);
    f.println("WeatherLanguage=" + WeatherLanguage);
    f.println("marqueeMessage=" + marqueeMessage);
    f.println("timeDisplayTurnsOn=" + timeDisplayTurnsOn);
    f.println("timeDisplayTurnsOff=" + timeDisplayTurnsOff);
    f.println("ledIntensity=" + String(displayIntensity));
    f.println("scrollSpeed=" + String(displayScrollSpeed));
    f.println("isFlash=" + String(flashOnSeconds));
    f.println("is24hour=" + String(IS_24HOUR));
    f.println("isPM=" + String(IS_PM));
    f.println("wideclockformat=" + Wide_Clock_Style);
    f.println("isMetric=" + String(IS_METRIC));
    f.println("refreshRate=" + String(minutesBetweenDataRefresh));
    f.println("minutesBetweenScrolling=" + String(minutesBetweenScrolling));
    f.println("isOctoPrint=" + String(OCTOPRINT_ENABLED));
    f.println("isOctoProgress=" + String(OCTOPRINT_PROGRESS));
    f.println("octoKey=" + OctoPrintApiKey);
    f.println("octoServer=" + OctoPrintServer);
    f.println("octoPort=" + String(OctoPrintPort));
    f.println("octoUser=" + OctoAuthUser);
    f.println("octoPass=" + OctoAuthPass);
    f.println("www_username=" + String(www_username));
    f.println("www_password=" + String(www_password));
    f.println("IS_BASIC_AUTH=" + String(IS_BASIC_AUTH));
    f.println("SHOW_CITY=" + String(SHOW_CITY));
    f.println("SHOW_CONDITION=" + String(SHOW_CONDITION));
    f.println("SHOW_HUMIDITY=" + String(SHOW_HUMIDITY));
    f.println("SHOW_WIND=" + String(SHOW_WIND));
    f.println("SHOW_PRESSURE=" + String(SHOW_PRESSURE));
    f.println("SHOW_HIGHLOW=" + String(SHOW_HIGHLOW));
    f.println("SHOW_DATE=" + String(SHOW_DATE));
    f.println("SHOW_TEMP=" + String(SHOW_TEMP));
    f.println("SHOW_FEEL_TEMP=" + String(SHOW_FEEL_TEMP));
    f.println("SHOW_RISE_SET=" + String(SHOW_RISE_SET));
    f.println("SHOW_TIMEZONE=" + String(SHOW_TIMEZONE));
    f.println("USE_PIHOLE=" + String(USE_PIHOLE));
    f.println("PiHoleServer=" + PiHoleServer);
    f.println("PiHolePort=" + String(PiHolePort));
    f.println("PiHoleApiKey=" + String(PiHoleApiKey));
    f.println("themeColor=" + themeColor);
    f.println("SHOW_AIR_POLLUTION=" + String(SHOW_AIR_POLLUTION));
    f.println("isWorldClock=" + String(WORLD_CLOCK_ENABLED));
    f.println("WorldCityName1=" + WorldCityName1);
    f.println("CityName=" + CityName);
    f.println("ENABLE_SCROLL=" + String(ENABLE_SCROLL));
    f.println("WorldCityName2=" + WorldCityName2);
    f.println("isPrayer=" + String(PRAYERS_ENABLED));
    f.println("prayersMethod=" + prayersMethod);
    f.println("isCurrency=" + String(CURRENCY_ENABLED));
    f.println("BaseCurrency1=" + BaseCurrency1);
    f.println("BaseCurrency2=" + BaseCurrency2);
    f.println("TargetCurrency=" + TargetCurrency);
    f.println("WorldCityName3=" + WorldCityName3);
  }
  f.close();
  readCityIds();
  weatherClient.updateCityName(CityName);
  String cityNames = weatherClient.getMyCityName();
  return cityNames;
}

void readCityIds()
{
  if (LittleFS.exists(CONFIG) == false)
  {
    Serial.println("Settings File does not yet exists.");
    writeCityIds();
    return;
  }
  File fr = LittleFS.open(CONFIG, "r");
  String line;
  while (fr.available())
  {
    line = fr.readStringUntil('\n');
    if (line.indexOf("TIMEDBKEY=") >= 0)
    {
      TIMEDBKEY = line.substring(line.lastIndexOf("TIMEDBKEY=") + 10);
      TIMEDBKEY.trim();
      Serial.println("TIMEDBKEY: " + TIMEDBKEY);
    }
    if (line.indexOf("APIKEY=") >= 0)
    {
      APIKEY = line.substring(line.lastIndexOf("APIKEY=") + 7);
      APIKEY.trim();
      Serial.println("APIKEY: " + APIKEY);
    }
    if (line.indexOf("WeatherLanguage=") >= 0)
    {
      WeatherLanguage = line.substring(line.lastIndexOf("WeatherLanguage=") + 16);
      WeatherLanguage.trim();
      Serial.println("WeatherLanguage: " + WeatherLanguage);
    }
    if (line.indexOf("isFlash=") >= 0)
    {
      flashOnSeconds = line.substring(line.lastIndexOf("isFlash=") + 8).toInt();
      Serial.println("flashOnSeconds= " + String(flashOnSeconds));
    }
    if (line.indexOf("is24hour=") >= 0)
    {
      IS_24HOUR = line.substring(line.lastIndexOf("is24hour=") + 9).toInt();
      Serial.println("IS_24HOUR= " + String(IS_24HOUR));
    }
    if (line.indexOf("isPM=") >= 0)
    {
      IS_PM = line.substring(line.lastIndexOf("isPM=") + 5).toInt();
      Serial.println("IS_PM= " + String(IS_PM));
    }
    if (line.indexOf("wideclockformat=") >= 0)
    {
      Wide_Clock_Style = line.substring(line.lastIndexOf("wideclockformat=") + 16);
      Wide_Clock_Style.trim();
      Serial.println("Wide_Clock_Style= " + Wide_Clock_Style);
    }
    if (line.indexOf("isMetric=") >= 0)
    {
      IS_METRIC = line.substring(line.lastIndexOf("isMetric=") + 9).toInt();
      Serial.println("IS_METRIC= " + String(IS_METRIC));
    }
    if (line.indexOf("refreshRate=") >= 0)
    {
      minutesBetweenDataRefresh = line.substring(line.lastIndexOf("refreshRate=") + 12).toInt();
      if (minutesBetweenDataRefresh == 0)
      {
        minutesBetweenDataRefresh = 15; // sıfır olamaz
      }
      Serial.println("minutesBetweenDataRefresh= " + String(minutesBetweenDataRefresh));
    }
    if (line.indexOf("minutesBetweenScrolling=") >= 0)
    {
      displayRefreshCount = 1;
      minutesBetweenScrolling = line.substring(line.lastIndexOf("minutesBetweenScrolling=") + 24).toInt();
      Serial.println("minutesBetweenScrolling= " + String(minutesBetweenScrolling));
    }
    if (line.indexOf("marqueeMessage=") >= 0)
    {
      marqueeMessage = line.substring(line.lastIndexOf("marqueeMessage=") + 15);
      marqueeMessage.trim();
      Serial.println("marqueeMessage= " + marqueeMessage);
    }
    if (line.indexOf("timeDisplayTurnsOn=") >= 0)
    {
      timeDisplayTurnsOn = line.substring(line.lastIndexOf("timeDisplayTurnsOn=") + 19);
      timeDisplayTurnsOn.trim();
      Serial.println("timeDisplayTurnsOn= " + timeDisplayTurnsOn);
    }
    if (line.indexOf("timeDisplayTurnsOff=") >= 0)
    {
      timeDisplayTurnsOff = line.substring(line.lastIndexOf("timeDisplayTurnsOff=") + 20);
      timeDisplayTurnsOff.trim();
      Serial.println("timeDisplayTurnsOff= " + timeDisplayTurnsOff);
    }
    if (line.indexOf("ledIntensity=") >= 0)
    {
      displayIntensity = line.substring(line.lastIndexOf("ledIntensity=") + 13).toInt();
      Serial.println("displayIntensity=" + String(displayIntensity));
    }
    if (line.indexOf("scrollSpeed=") >= 0)
    {
      displayScrollSpeed = line.substring(line.lastIndexOf("scrollSpeed=") + 12).toInt();
      Serial.println("displayScrollSpeed= " + String(displayScrollSpeed));
    }
    if (line.indexOf("isOctoPrint=") >= 0)
    {
      OCTOPRINT_ENABLED = line.substring(line.lastIndexOf("isOctoPrint=") + 12).toInt();
      Serial.println("OCTOPRINT_ENABLED= " + String(OCTOPRINT_ENABLED));
    }
    if (line.indexOf("isOctoProgress=") >= 0)
    {
      OCTOPRINT_PROGRESS = line.substring(line.lastIndexOf("isOctoProgress=") + 15).toInt();
      Serial.println("OCTOPRINT_PROGRESS= " + String(OCTOPRINT_PROGRESS));
    }
    if (line.indexOf("octoKey=") >= 0)
    {
      OctoPrintApiKey = line.substring(line.lastIndexOf("octoKey=") + 8);
      OctoPrintApiKey.trim();
      Serial.println("OctoPrintApiKey= " + OctoPrintApiKey);
    }
    if (line.indexOf("octoServer=") >= 0)
    {
      OctoPrintServer = line.substring(line.lastIndexOf("octoServer=") + 11);
      OctoPrintServer.trim();
      Serial.println("OctoPrintServer= " + OctoPrintServer);
    }
    if (line.indexOf("octoPort=") >= 0)
    {
      OctoPrintPort = line.substring(line.lastIndexOf("octoPort=") + 9).toInt();
      Serial.println("OctoPrintPort= " + String(OctoPrintPort));
    }
    if (line.indexOf("octoUser=") >= 0)
    {
      OctoAuthUser = line.substring(line.lastIndexOf("octoUser=") + 9);
      OctoAuthUser.trim();
      Serial.println("OctoAuthUser= " + OctoAuthUser);
    }
    if (line.indexOf("octoPass=") >= 0)
    {
      OctoAuthPass = line.substring(line.lastIndexOf("octoPass=") + 9);
      OctoAuthPass.trim();
      Serial.println("OctoAuthPass= " + OctoAuthPass);
    }
    if (line.indexOf("www_username=") >= 0)
    {
      String temp = line.substring(line.lastIndexOf("www_username=") + 13);
      temp.trim();
      temp.toCharArray(www_username, sizeof(temp));
      Serial.println("www_username= " + String(www_username));
    }
    if (line.indexOf("www_password=") >= 0)
    {
      String temp = line.substring(line.lastIndexOf("www_password=") + 13);
      temp.trim();
      temp.toCharArray(www_password, sizeof(temp));
      Serial.println("www_password= " + String(www_password));
    }
    if (line.indexOf("IS_BASIC_AUTH=") >= 0)
    {
      IS_BASIC_AUTH = line.substring(line.lastIndexOf("IS_BASIC_AUTH=") + 14).toInt();
      Serial.println("IS_BASIC_AUTH= " + String(IS_BASIC_AUTH));
    }
    if (line.indexOf("SHOW_CITY=") >= 0)
    {
      SHOW_CITY = line.substring(line.lastIndexOf("SHOW_CITY=") + 10).toInt();
      Serial.println("SHOW_CITY= " + String(SHOW_CITY));
    }
    if (line.indexOf("SHOW_CONDITION=") >= 0)
    {
      SHOW_CONDITION = line.substring(line.lastIndexOf("SHOW_CONDITION=") + 15).toInt();
      Serial.println("SHOW_CONDITION= " + String(SHOW_CONDITION));
    }
    if (line.indexOf("SHOW_HUMIDITY=") >= 0)
    {
      SHOW_HUMIDITY = line.substring(line.lastIndexOf("SHOW_HUMIDITY=") + 14).toInt();
      Serial.println("SHOW_HUMIDITY= " + String(SHOW_HUMIDITY));
    }
    if (line.indexOf("SHOW_WIND=") >= 0)
    {
      SHOW_WIND = line.substring(line.lastIndexOf("SHOW_WIND=") + 10).toInt();
      Serial.println("SHOW_WIND= " + String(SHOW_WIND));
    }
    if (line.indexOf("SHOW_PRESSURE=") >= 0)
    {
      SHOW_PRESSURE = line.substring(line.lastIndexOf("SHOW_PRESSURE=") + 14).toInt();
      Serial.println("SHOW_PRESSURE= " + String(SHOW_PRESSURE));
    }
    if (line.indexOf("SHOW_HIGHLOW=") >= 0)
    {
      SHOW_HIGHLOW = line.substring(line.lastIndexOf("SHOW_HIGHLOW=") + 13).toInt();
      Serial.println("SHOW_HIGHLOW= " + String(SHOW_HIGHLOW));
    }
    if (line.indexOf("SHOW_DATE=") >= 0)
    {
      SHOW_DATE = line.substring(line.lastIndexOf("SHOW_DATE=") + 10).toInt();
      Serial.println("SHOW_DATE= " + String(SHOW_DATE));
    }
    if (line.indexOf("SHOW_TIMEZONE=") >= 0)
    {
      SHOW_TIMEZONE = line.substring(line.lastIndexOf("SHOW_TIMEZONE=") + 14).toInt();
      Serial.println("SHOW_TIMEZONE= " + String(SHOW_TIMEZONE));
    }
    if (line.indexOf("SHOW_RISE_SET=") >= 0)
    {
      SHOW_RISE_SET = line.substring(line.lastIndexOf("SHOW_RISE_SET=") + 14).toInt();
      Serial.println("SHOW_RISE_SET= " + String(SHOW_RISE_SET));
    }
    if (line.indexOf("SHOW_FEEL_TEMP=") >= 0)
    {
      SHOW_FEEL_TEMP = line.substring(line.lastIndexOf("SHOW_FEEL_TEMP=") + 15).toInt();
      Serial.println("SHOW_FEEL_TEMP= " + String(SHOW_FEEL_TEMP));
    }
    if (line.indexOf("SHOW_TEMP=") >= 0)
    {
      SHOW_TEMP = line.substring(line.lastIndexOf("SHOW_TEMP=") + 10).toInt();
      Serial.println("SHOW_TEMP= " + String(SHOW_TEMP));
    }
    if (line.indexOf("CityName=") >= 0)
    {
      CityName = line.substring(line.lastIndexOf("CityName=") + 9);
      CityName.trim();
      Serial.println("CityName: " + CityName);
    }
    if (line.indexOf("USE_PIHOLE=") >= 0)
    {
      USE_PIHOLE = line.substring(line.lastIndexOf("USE_PIHOLE=") + 11).toInt();
      Serial.println("USE_PIHOLE= " + String(USE_PIHOLE));
    }
    if (line.indexOf("PiHoleServer=") >= 0)
    {
      PiHoleServer = line.substring(line.lastIndexOf("PiHoleServer=") + 13);
      PiHoleServer.trim();
      Serial.println("PiHoleServer=" + String(PiHoleServer));
    }
    if (line.indexOf("PiHolePort=") >= 0)
    {
      PiHolePort = line.substring(line.lastIndexOf("PiHolePort=") + 11).toInt();
      Serial.println("PiHolePort= " + String(PiHolePort));
    }
    if (line.indexOf("PiHoleApiKey=") >= 0)
    {
      PiHoleApiKey = line.substring(line.lastIndexOf("PiHoleApiKey=") + 13);
      PiHoleApiKey.trim();
      Serial.println("PiHoleApiKey= " + String(PiHoleApiKey));
    }
    if (line.indexOf("themeColor=") >= 0)
    {
      themeColor = line.substring(line.lastIndexOf("themeColor=") + 11);
      themeColor.trim();
      Serial.println("themeColor= " + themeColor);
    }
    if (line.indexOf("SHOW_AIR_POLLUTION=") >= 0)
    {
      SHOW_AIR_POLLUTION = line.substring(line.lastIndexOf("SHOW_AIR_POLLUTION=") + 19).toInt();
      Serial.println("SHOW_AIR_POLLUTION= " + String(SHOW_AIR_POLLUTION));
    }
    if (line.indexOf("isWorldClock=") >= 0)
    {
      WORLD_CLOCK_ENABLED = line.substring(line.lastIndexOf("isWorldClock=") + 13).toInt();
      Serial.println("WORLD_CLOCK_ENABLED= " + String(WORLD_CLOCK_ENABLED));
    }
    if (line.indexOf("WorldCityName1=") >= 0)
    {
      WorldCityName1 = line.substring(line.lastIndexOf("WorldCityName1=") + 15);
      WorldCityName1.trim();
      Serial.println("WorldCityName1= " + WorldCityName1);
    }
    if (line.indexOf("ENABLE_SCROLL=") >= 0)
    {
      ENABLE_SCROLL = line.substring(line.lastIndexOf("ENABLE_SCROLL=") + 14).toInt();
      Serial.println("ENABLE_SCROLL= " + String(ENABLE_SCROLL));
    }
    if (line.indexOf("WorldCityName2=") >= 0)
    {
      WorldCityName2 = line.substring(line.lastIndexOf("WorldCityName2=") + 15);
      WorldCityName2.trim();
      Serial.println("WorldCityName2= " + WorldCityName2);
    }
    if (line.indexOf("isPrayer=") >= 0)
    {
      PRAYERS_ENABLED = line.substring(line.lastIndexOf("isPrayer=") + 9).toInt();
      Serial.println("PRAYERS_ENABLED= " + String(PRAYERS_ENABLED));
    }
    if (line.indexOf("prayersMethod=") >= 0)
    {
      prayersMethod = line.substring(line.lastIndexOf("prayersMethod=") + 14);
      prayersMethod.trim();
      Serial.println("prayersMethod= " + prayersMethod);
    }
    if (line.indexOf("isCurrency=") >= 0)
    {
      CURRENCY_ENABLED = line.substring(line.lastIndexOf("isCurrency=") + 11).toInt();
      Serial.println("CURRENCY_ENABLED= " + String(CURRENCY_ENABLED));
    }
    if (line.indexOf("BaseCurrency1=") >= 0)
    {
      BaseCurrency1 = line.substring(line.lastIndexOf("BaseCurrency1=") + 14);
      BaseCurrency1.trim();
      Serial.println("BaseCurrency1= " + BaseCurrency1);
    }
    if (line.indexOf("BaseCurrency2=") >= 0)
    {
      BaseCurrency2 = line.substring(line.lastIndexOf("BaseCurrency2=") + 14);
      BaseCurrency2.trim();
      Serial.println("BaseCurrency2= " + BaseCurrency2);
    }
    if (line.indexOf("TargetCurrency=") >= 0)
    {
      TargetCurrency = line.substring(line.lastIndexOf("TargetCurrency=") + 15);
      TargetCurrency.trim();
      Serial.println("TargetCurrency= " + TargetCurrency);
    }
    if (line.indexOf("WorldCityName3=") >= 0)
    {
      WorldCityName3 = line.substring(line.lastIndexOf("WorldCityName3=") + 15);
      WorldCityName3.trim();
      Serial.println("WorldCityName3= " + WorldCityName3);
    }
  }
  fr.close();
  matrix.setIntensity(displayIntensity);
  prayersClient.updateMethodID(prayersMethod);
  currencyClient.updateBaseCurrency1(BaseCurrency1);
  currencyClient.updateBaseCurrency2(BaseCurrency2);
  currencyClient.updateTargetCurrency(TargetCurrency);
  worldWeatherClient.updateWorldCityName1(WorldCityName1);
  worldWeatherClient.updateWorldCityName2(WorldCityName2);
  worldWeatherClient.updateWorldCityName3(WorldCityName3);
  weatherClient.updateWeatherApiKey(APIKEY);
  weatherClient.updateLanguage(WeatherLanguage);
  weatherClient.setMetric(IS_METRIC);
  weatherClient.updateCityName(CityName);
  printerClient.updateOctoPrintClient(OctoPrintApiKey, OctoPrintServer, OctoPrintPort, OctoAuthUser, OctoAuthPass);
}

void scrollMessage(String msg)
{
  msg += " "; // add a space at the end
  for (int i = 0; i < width * msg.length() + matrix.width() - 1 - spacer; i++)
  {
    if (WEBSERVER_ENABLED)
    {
      server.handleClient();
    }
    if (ENABLE_OTA)
    {
      ArduinoOTA.handle();
    }
    if (refresh == 1)
      i = 0;
    refresh = 0;
    matrix.fillScreen(LOW);

    int letter = i / width;
    int x = (matrix.width() - 1) - i % width;
    int y = (matrix.height() - 8) / 2; // center the text vertically

    while (x + width - spacer >= 0 && letter >= 0)
    {
      if (letter < msg.length())
      {
        matrix.drawChar(x, y, msg[letter], HIGH, LOW, 1);
      }

      letter--;
      x -= width;
    }

    matrix.write(); // Send bitmap to display
    delay(displayScrollSpeed);
  }
  matrix.setCursor(0, 0);
}

void drawPiholeGraph()
{
  if (!USE_PIHOLE || piholeClient.getBlockedCount() == 0)
  {
    return;
  }
  int count = piholeClient.getBlockedCount();
  int high = 0;
  int row = matrix.width() - 1;
  int yval = 0;

  int totalRows = count - matrix.width();

  if (totalRows < 0)
  {
    totalRows = 0;
  }

  // get the high value for the sample that will be on the screen
  for (int inx = count; inx >= totalRows; inx--)
  {
    if (piholeClient.getBlockedAds()[inx] > high)
    {
      high = (int)piholeClient.getBlockedAds()[inx];
    }
  }

  int currentVal = 0;
  for (int inx = (count - 1); inx >= totalRows; inx--)
  {
    currentVal = (int)piholeClient.getBlockedAds()[inx];
    yval = map(currentVal, 0, high, 7, 0);
    // Serial.println("Value: " + String(currentVal));
    // Serial.println("x: " + String(row) + " y:" + String(yval) + " h:" + String(8-yval));
    matrix.drawFastVLine(row, yval, 8 - yval, HIGH);
    if (row == 0)
    {
      break;
    }
    row--;
  }
  matrix.write();
  for (int wait = 0; wait < 500; wait++)
  {
    if (WEBSERVER_ENABLED)
    {
      server.handleClient();
    }
    if (ENABLE_OTA)
    {
      ArduinoOTA.handle();
    }
    delay(20);
  }
}

void centerPrint(String msg)
{
  centerPrint(msg, false);
}

void centerPrint(String msg, boolean extraStuff)
{
  int x = (matrix.width() - (msg.length() * width)) / 2;

  // Print the static portions of the display before the main Message
  if (extraStuff)
  {
    if (!IS_24HOUR && IS_PM && isPM())
    {
      matrix.drawPixel(matrix.width() - 1, 6, HIGH);
    }
    if (OCTOPRINT_ENABLED && OCTOPRINT_PROGRESS && printerClient.isPrinting())
    {
      int numberOfLightPixels = (printerClient.getProgressCompletion().toFloat() / float(100)) * (matrix.width() - 1);
      matrix.drawFastHLine(0, 7, numberOfLightPixels, HIGH);
    }
  }

  matrix.setCursor(x, 0);
  matrix.print(msg);

  matrix.write();
}

String CleanText(String text)
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

String decodeHtmlString(String msg)
{
  String decodedMsg = msg;
  // Restore special characters that are misformed to %char by the client browser
  decodedMsg.replace("+", " ");
  decodedMsg.replace("%21", "!");
  decodedMsg.replace("%22", "");
  decodedMsg.replace("%23", "#");
  decodedMsg.replace("%24", "$");
  decodedMsg.replace("%25", "%");
  decodedMsg.replace("%26", "&");
  decodedMsg.replace("%27", "'");
  decodedMsg.replace("%28", "(");
  decodedMsg.replace("%29", ")");
  decodedMsg.replace("%2A", "*");
  decodedMsg.replace("%2B", "+");
  decodedMsg.replace("%2C", ",");
  decodedMsg.replace("%2F", "/");
  decodedMsg.replace("%3A", ":");
  decodedMsg.replace("%3B", ";");
  decodedMsg.replace("%3C", "<");
  decodedMsg.replace("%3D", "=");
  decodedMsg.replace("%3E", ">");
  decodedMsg.replace("%3F", "?");
  decodedMsg.replace("%40", "@");
  decodedMsg.toUpperCase();
  decodedMsg.trim();
  return decodedMsg;
}

String encodeHtmlString(String msg)
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

String getTimestamp2Date(time_t timeStamp)
{
  String rtnValue = "";
  String monthValue = "";
  String weekdayValue = "";

  if (timeStamp == 0)
  {
    return rtnValue;
  }

  long epoc = timeStamp;
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