# Marquee Scroller (Clock, Weather, News, and More) Turkish Version

## Changes Betwwen the Original Project
--- In Turkish ---
* ArduinoJson kütüphanesi versiyon 6'ya güncellendi.
* Wemos D1 Mini yerine NodeMCU kullanılmaktadır. 
* Web sayfası ve dot matrix yazıları Türkçeleştirilmiştir.
* Yerel namaz vakitleri eklenmiştir.
* Dünya saatleri özelliği (OpenWeatherMap ve TimeZoneDB kullanılarak) eklenmiştir.
* CityID yerine şehir ismi ile hava durumu bilgisi elde edilmektedir.
* Hava kirliliği özelliği eklenmiştir.
* Hissedilen sıcaklık eklenmiştir.
* Güneş doğuşu ve batışı eklenmiştir. (<a href="https://github.com/andrewcliffoutlook/marquee-scroller">andrewcliffoutlook</a>'a teşekkürler.)
* Bulutlanma eklenmiştir. 
* cleanText'e Türkçe harfler eklenmiştir.
* Şehir adı, ülke adı, zaman dilimi vb özellikler TimeZoneDB kullanılarak eklenmiştir.
* Zaman dilimleri arasındaki farkı tespit etme özelliği eklenmiştir.
* Haberler özelliği silinmiştir.
* Hava Durumu için dil seçeneği eklenmiştir.
* Kayan yazıyı kapatma özelliği eklenmiştir.
* Kayan yazıda gösterilecek bilgileri Ekran Ayarları sayfasında taşınmıştır.
* PlatformIO ile derleme desteği eklenmiştir.
* SPIFFS yerine LittleFS kullanılmaktadır.
* Ayın doğuşu ve batışı eklenmiştir. (<a href="https://github.com/ThingPulse/esp8266-weather-station">ThingPulse</a>'a teşekkürler.)

--- In English ---
* ArduinoJson library updated to version 6.
* NodeMCU is used instead of Wemos D1 Mini.
* Web interface and display texts have been translated into Turkish.
* Local prayers times have been added.
* World clocks feature (using OpenWeatherMap and TimeZoneDB) has been added.
* Weather information is obtained with the city name instead of CityID.
* Air pollution feature has been added.
* Apparent temperature has been added.
* Sunrise and sunset have been added. (Thanks to <a href="https://github.com/andrewcliffoutlook/marquee-scroller">andrewcliffoutlook</a>)
* Cloudiness has been added.
* Turkish letters have been added to cleanText.
* City name, country name, time zone etc. features have been added using TimeZoneDB.
* The ability to detect the difference between time zones has been added.
* News feature has been deleted.
* Language option for weather has been added.
* The feature to turn off the marquee has been added.
* The information to be displayed in the marquee has been moved to the Display Settings page.
* PlatformIO build feature has been added.
* LittleFS is used instead of SPIFFS.
* Moonrise and moonset have been added. (Thanks to <a href="https://github.com/ThingPulse/esp8266-weather-station">ThingPulse</a>)

## NOTICE
The latest version of Marquee Scroller 3.01 works with **ESP8266 Core 3.0.2** -- if you are upgrading from Marquee Scroller 2.X version this may require you to enter in all your API Keys and settings.  Always meake sure you have coppied all your API keys somewhere before updating.  The ESP8266 Core 3.0.2 uses the newer FS (file system) that may require a fresh start on the configuration.
Make sure you update to the latest version of WifiManager library (link below).
* Removed Bitcoin features in 3.0

## Features include:
* Accurate Clock refresh off Internet Time Servers
* Local Weather and conditions (refreshed every 10 - 30 minutes)
* News Headlines from all the major sources
* Configured through Web Interface
* Display 3D print progress from your OctoPrint Server
* Option to display Pi-hole status and graph (each pixel accross is 10 minutes)
* Basic Authorization around Configuration web interface
* Support for OTA (loading firmware over WiFi)
* Update firmware through web interface
* Configurable scroll speed
* Configurable scrolling frequency
* Configurable number of LED panels
* Options of different types of Clock Displays (display seconds or temperature) -- only on 8 or more display panels
* Video: https://youtu.be/DsThufRpoiQ
* Build Video by Chris Riley: https://youtu.be/KqBiqJT9_lE

## Required Parts:
* Wemos D1 Mini: https://amzn.to/3tMl81U
* Dot Matrix Module: https://amzn.to/2HtnQlD  

Note: Using the links provided here help to support these types of projects. Thank you for the support.  

## Wiring for the NodeMCU to the Dot Matrix Display
CLK -> D5 (SCK)  
CS  -> D8  
DIN -> D7 (MOSI)  
VCC -> 5V+  
GND -> GND- 

## Wiring for the Wemos D1 Mini to the Dot Matrix Display
CLK -> D5 (SCK)  
CS  -> D6  
DIN -> D7 (MOSI)  
VCC -> 5V+  
GND -> GND-  

![Marquee Scroller Wiring](/images/marquee_scroller_pins.png)  

## 3D Printed Case by David Payne:  
Original Single Panel version: https://www.thingiverse.com/thing:2867294  
Double Wide LED version: https://www.thingiverse.com/thing:2989552  

## Compiling and Loading to Wemos D1
It is recommended to use Arduino IDE.  You will need to configure Arduino IDE to work with the Wemos board and USB port and installed the required USB drivers etc.  
* USB CH340G drivers:  https://sparks.gogo.co.nz/ch340.html
* Enter http://arduino.esp8266.com/stable/package_esp8266com_index.json into Additional Board Manager URLs field. You can add multiple URLs, separating them with commas.  This will add support for the Wemos D1 Mini to Arduino IDE.
* Open Boards Manager from Tools > Board menu and install esp8266 Core platform version Latest **3.0.2**
* Select Board:  "ESP8266 Boards (3.0.2)" --> "LOLIN(WEMOS) D1 R2 & mini"
* Set Flash Size: 4MB (FS:1MB OTA:~1019KB) -- **this project requires FS for saving and reading configuration settings.**
* Select the **Port** from the tools menu.  

## Loading Supporting Library Files in Arduino
Use the Arduino guide for details on how to installing and manage libraries https://www.arduino.cc/en/Guide/Libraries  
**Packages** -- the following packages and libraries are used (download and install):  
<WiFiManager.h> --> https://github.com/tzapu/WiFiManager (latest)  
<TimeLib.h> --> https://github.com/PaulStoffregen/Time  
<Adafruit_GFX.h> --> https://github.com/adafruit/Adafruit-GFX-Library  
<Max72xxPanel.h> --> https://github.com/markruys/arduino-Max72xxPanel  
<JsonStreamingParser.h> --> https://github.com/squix78/json-streaming-parser  
<ArduinoJson.h> --> https://github.com/bblanchon/ArduinoJson

## Initial Configuration
Editing the **Settings.h** file is totally optional and not required.  All API Keys are now managed in the Web Interface. It is not required to edit the Settings.h file before loading and running the code.  
* Open Weather Map free API key: http://openweathermap.org/  -- this is used to get weather data and the latitude and longitude for the current time zone. Weather API key is required for correct time.
* TimeZoneDB free registration for API key: https://timezonedb.com/register -- this is used for setting the time and getting the correct time zone as well as managing time changes due to Day Light Savings time by regions.  This key is set and managed only through the web interface and added in version 2.10 of Marquee Scroller. TimeZoneDB key is required for correct time display.
* News API key (free): https://newsapi.org/ -- Optional if you want to get current news headlines.
* Your OctoPrint API Key -- optional if you use the OctoPrint status.
* Version 2.0 supports Chained 4x1 LED displays -- configure up to 16x1 in the Settings.h file.  

NOTE: The settings in the Settings.h are the default settings for the first loading. After loading you will manage changes to the settings via the Web Interface. If you want to change settings again in the settings.h, you will need to erase the file system on the Wemos or use the “Reset Settings” option in the Web Interface.  

## Web Interface
The Marquee Scroller uses the **WiFiManager** so when it can't find the last network it was connected to 
it will become a **AP Hotspot** -- connect to it with your phone and you can then enter your WiFi connection information.

After connected to your WiFi network it will display the IP addressed assigned to it and that can be 
used to open a browser to the Web Interface.  You will be able to manage your API Keys through the web interface.  
The default user / password for the configuration page is: admin / password  

The Clock will display the time of the City selected for the weather.  

<p align="center">
  <img src="/images/2018-04-19%2006.58.05.png" width="200"/>
  <img src="/images/2018-04-19%2006.58.15.png" width="200"/>
  <img src="/images/2018-04-19%2006.58.32.png" width="200"/>
  <img src="/images/2018-04-19%2006.58.58.png" width="200"/>
</p>

## Donation or Tip
Please do not feel obligated, but donations and tips are warmly welcomed.  I have added the donation button at the request of a few people that wanted to contribute and show appreciation.  Thank you, and enjoy the application and project.  

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=A82AT6FLN2MPY)

Or -- you can buy me something from my Amazon Wishlist: https://www.amazon.com/hz/wishlist/ls/GINC2PHRNEY3  

## Contributors
David Payne  
Nathan Glaus  
Daniel Eichhorn -- Author of the TimeClient class (in older versions)  
yanvigdev  
nashiko-s  
magnum129  

Contributing to this software is warmly welcomed. You can do this basically by forking from master, committing modifications and then making a pulling requests against the latest DEV branch to be reviewed (follow the links above for operating guide). Detailed comments are encouraged. Adding change log and your contact into file header is encouraged. Thanks for your contribution.

When considering making a code contribution, please keep in mind the following goals for the project:
* User should not be required to edit the Settings.h file to compile and run.  This means the feature should be simple enough to manage through the web interface.
* Changes should always support the recommended hardware (links above).

![Marquee Scroller](/images/5d7f02ccbf01125cabbf246f97f2ead1_preview_featured.jpg)  
![Marquee Parts](/images/1ffa0c835554d280258c13be5513c4fe_preview_featured.jpg)
