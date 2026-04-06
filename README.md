[Readme API Wind direction, wind spe.txt](https://github.com/user-attachments/files/26225668/Readme.API.Wind.direction.wind.spe.txt)
[Readme API Wind direction, wind spe.txt](https://github.com/user-attachments/files/26225668/Readme.API.Wind.direction.wind.spe.txt)
Readme API Wind direction, wind speed and Temperature project
An API is an Application Programming Interface that uses a set of rules and protocals to allow the exchange of information. Open weather map 
has a free service that provides your local weather data with one simple call from their website. The Arduino NANO is an ESP 32 microcontroller that has a WiFi module on board that allows you to connect to the internet via WiFi and request specific weather data. I wanted to get outside wind speed , wind direction and outside air temperature without having to put up a mast and sensors that eventually fail. Two 1.28 inch round TFT displays using the GC9A01 driver display the data. Because I'm using multiple displays I chose the Arduino GFX driver to run the displays because it allows multiple displays on the ARDUINO ESP32 SPI bus. You will need to provide two separate CS and RST lines, one set for  for each display. See my schematic.  Getting the Arduino Nano ESP32 to work was not as easy as I expected. It was critically important to configure the Arduino IDE to work with the Arduino Nano ESP32(See second paragraph) . It took several days to figure out how to properly configure the IDE as some of the settings are not intuitive. I don't suggest trying to use TFT_eSPI graphics driver as it's really only good for one display. After downloading the sketch you'll need to do three things 1, set up an Open API Weather map account and get a free apiKey.  See : https://openweathermap.org/api  . Version 2.5 is free. Higher versions will require you to pay. 
Put that apiKey in the sketch where I have "apiKey" . It's on line 24. The second thing you need to do is determine your lattitude and longitude. Put those on the LAT and LON data types. they are line 22 & 23. The 3rd thing you'll need to do is configure your WIFI SSID and WIFI password where I have "your SSID" and the next line where I have "yourPassword" . When you load the sketch there are some trouble shooting comments that will print in the monitor to tell you how far you got in the sketch if it locks up. 
The TFT displays will update once every 5 min. To go much faster you need a paid openweathermap account. 
 

Arduino IDE (IDE Setup Stuff I needed to do to get the ESP32 Nano working)
I used Arduino IDE 2.3.7
You will need to go into the board manager and load esp32 by Espressif Ver 3.3.6
(Do not load the Arduino ESP32 Boards package. I couldn't get it to work correctly.)
In The Arduino IDE -Tools, Board Manager - choose esp32 , then choose Arduino Nano esp32 from the list  (Its down near the bottom) .
Then in tools go to Pin Numbering: Then choose By GPIO number (legacy). This is important for the Arduino GFX pin numbering according to Bodmer.
When connecting to the Arduino Nano ESP32 look for the COM port that expresses when you plug it in. You may need device manager to find which port shows up when you plug it in.
When first plugging in the Arduino NANO ESP32 you'll probably need to press the reset button twice, quite fast. The green LED should be blinking slowly or you may not be in boot mode. This took a while to figure out and do correctly. ie one quick push then a longer second push . Also make sure the Arduino IDE monitor is not open or you can't talk to the ESP 32 and it won't go into boot mode. 

These are the libraries you will need to load into your IDE library for this sketch to run

#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP32Time.h>
#include <Arduino_GFX_Library.h>
#include <SPI.h>
The youtube video for this is at     https://youtu.be/NmUVwrHxLis  
