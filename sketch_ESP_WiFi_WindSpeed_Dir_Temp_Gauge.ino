/****************************************************
   Nano ESP32 Dual Gauge
   Wind Dir/Speed + Temperature
   GC9A01 + Arduino_GFX
   Updates once every 5 min

    2/9/26 Released 
*****************************************************/

#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP32Time.h>
#include <Arduino_GFX_Library.h>
#include <SPI.h>

/************* WIFI BACKUP ******************/
const char* WIFI_SSID = "yourSSID"; //User to modify
const char* WIFI_PASS = "yourPassword ";//User to modify 

/************* OPENWEATHER ******************/
// You will need to figure out your decimal lattitude and longitude and enter them here 
const float LAT = 45.7469;
const float LON = -75.7589;
const char* apiKey = "apiKey";//User to modify, make a free account to get your own key and enter it here

/************* COLORS ***********************/
#define TFT_BLACK    0x0000
#define TFT_WHITE    0xFFFF
#define TFT_RED      0xF800
#define TFT_ORANGE   0xFD20
#define TFT_YELLOW   0xFFE0
#define TFT_CYAN     0x07FF
#define TFT_DARKGREY 0x7BEF
#define TFT_GREEN    0x07E0

/************* GC9A01 PINS ******************/
// Shared SPI
#define TFT_MOSI 38
#define TFT_SCLK 48
#define TFT_DC   9

// Wind Display
#define WIND_CS   10
#define WIND_RST  8

// Temp Display
#define TEMP_CS   7
#define TEMP_RST  6

/************* DISPLAY **********************/

// Wind Bus
Arduino_DataBus *windBus = new Arduino_ESP32SPI(
  TFT_DC,
  WIND_CS,
  TFT_SCLK,
  TFT_MOSI,
  -1,
  FSPI,
  true
);

// Temp Bus
Arduino_DataBus *tempBus = new Arduino_ESP32SPI(
  TFT_DC,
  TEMP_CS,
  TFT_SCLK,
  TFT_MOSI,
  -1,
  FSPI,
  true
);

// Displays
Arduino_GFX *windTFT = new Arduino_GC9A01(windBus, WIND_RST, 0, true);
Arduino_GFX *tempTFT = new Arduino_GC9A01(tempBus, TEMP_RST, 0, true);

/************* OBJECTS **********************/
ESP32Time rtc;

/************* DATA *************************/
float windSpeed = 0;
int   windDir = 0;
float temperatureF = 0;

unsigned long lastUpdate = 0;
const unsigned long updateTime = 300000; // 5 min - How often your data updates 

// Wind Gauge
int cx, cy, radius;

// Temp Gauge
int tcx, tcy, tradius;

/************************************************
   CENTER TEXT
************************************************/
void drawCenterText(Arduino_GFX *d, const char *txt, int x, int y, uint8_t s)
{
  int16_t x1, y1;
  uint16_t w, h;

  d->setTextSize(s);
  d->getTextBounds(txt, 0, 0, &x1, &y1, &w, &h);

  d->setCursor(x - w / 2, y - h / 2);
  d->print(txt);
}

/************************************************
   SETUP
************************************************/
void setup()
{
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n--- BOOT START ---");

  // Init Displays
  windTFT->begin();
  delay(100);
  tempTFT->begin();

  windTFT->setRotation(0);
  tempTFT->setRotation(2);

  windTFT->fillScreen(TFT_BLACK);
  tempTFT->fillScreen(TFT_BLACK);

  // Wind coords
  cx = windTFT->width() / 2;
  cy = windTFT->height() / 2;
  radius = cx - 8;

  // Temp coords
  tcx = tempTFT->width() / 2;
  tcy = tempTFT->height() / 2;
  tradius = tcx - 8;

  setupWiFi();

  configTime(-5 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  getWeather();
  drawAll();

  Serial.println("--- BOOT DONE ---");
}

/************************************************
   LOOP
************************************************/
void loop()
{
  if (millis() - lastUpdate > updateTime)
  {
    getWeather();
    drawAll();
    lastUpdate = millis();
  }
}

/************************************************
   WIFI
************************************************/
void setupWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("Connecting");

  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - start < 10000)
  {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\nWiFi Connected!");
    Serial.println(WiFi.localIP());
  }
  else
  {
    WiFiManager wm;

    if (!wm.autoConnect("DualGauge_AP"))
      ESP.restart();
  }
}

/************************************************
   WEATHER
************************************************/
void getWeather()
{
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;

  String url =
    "https://api.openweathermap.org/data/2.5/weather?lat=45.7469&lon=-75.7589"
    "&appid=" + String(apiKey) +
    "&units=imperial";

  http.begin(url);

  int code = http.GET();

  if (code == 200)
  {
    StaticJsonDocument<2048> doc;

    if (!deserializeJson(doc, http.getString()))
    {
      windSpeed    = doc["wind"]["speed"];
      windDir      = doc["wind"]["deg"];
      temperatureF = doc["main"]["temp"];

      windDir = (windDir + 180) % 360;

      Serial.println("Updated:");
      Serial.print("Wind: "); Serial.println(windSpeed);
      Serial.print("Dir : "); Serial.println(windDir);
      Serial.print("Temp: "); Serial.println(temperatureF);
    }
  }

  http.end();
}

/************************************************
   MAIN DRAW
************************************************/
void drawAll()
{
  drawWindGauge();
  drawTempGauge();
}

/************************************************
   WIND GAUGE
************************************************/
void drawWindGauge()
{
  windTFT->fillScreen(TFT_BLACK);

  // Dial
  windTFT->drawCircle(cx, cy, radius, TFT_WHITE);
  windTFT->drawCircle(cx, cy, radius - 2, TFT_WHITE);

  for (int i = 0; i < 360; i += 10)
  {
    float a = (i - 90) * DEG_TO_RAD;

    int x1 = cx + cos(a) * (radius - 5);
    int y1 = cy + sin(a) * (radius - 5);

    int x2 = cx + cos(a) * (radius - 18);
    int y2 = cy + sin(a) * (radius - 18);

    uint16_t col = (i % 90 == 0) ? TFT_YELLOW : TFT_DARKGREY;

    windTFT->drawLine(x1, y1, x2, y2, col);
  }

  // Labels
  windTFT->setTextColor(TFT_WHITE, TFT_BLACK);

  drawCenterText(windTFT, "N", cx, cy - radius + 15, 2);
  drawCenterText(windTFT, "S", cx, cy + radius - 35, 2);
  drawCenterText(windTFT, "E", cx + radius - 20, cy - 8, 2);
  drawCenterText(windTFT, "W", cx - radius + 20, cy - 8, 2);

  // Needle
  float a = ((windDir + 180) - 90) * DEG_TO_RAD;

  int tx = cx + cos(a) * (radius - 25);
  int ty = cy + sin(a) * (radius - 25);

  windTFT->drawLine(cx, cy, tx, ty, TFT_RED);
  windTFT->drawLine(cx + 1, cy, tx, ty, TFT_ORANGE);
  windTFT->drawLine(cx - 1, cy, tx, ty, TFT_ORANGE);

  windTFT->fillCircle(cx, cy, 6, TFT_WHITE);
  windTFT->fillCircle(cx, cy, 3, TFT_BLACK);

  // Speed
  windTFT->setTextColor(TFT_CYAN, TFT_BLACK);
  windTFT->setTextSize(2);

  char buf[20];
  sprintf(buf, "%.1f MPH", windSpeed);

  drawCenterText(windTFT, buf, cx, cy + 45, 2);
}

/************************************************
   TEMP GAUGE
************************************************/
void drawTempGauge()

{
  tempTFT->fillScreen(TFT_BLACK);

  const float ROT = -90 * DEG_TO_RAD; // 90° CCW rotation

  // Outer dial
  tempTFT->drawCircle(tcx, tcy, tradius, TFT_WHITE);
  tempTFT->drawCircle(tcx, tcy, tradius - 2, TFT_WHITE);

  tempTFT->setTextColor(TFT_WHITE, TFT_BLACK);
  tempTFT->setTextSize(1);

  // Ticks + Numbers
  for (int t = -20; t <= 120; t += 10)
  {
    // Base angle
    float a = map(t, -20, 120, -140, 140) * DEG_TO_RAD;

    // Rotate CCW 90°
    a += ROT;

    // Tick marks
    int x1 = tcx + cos(a) * (tradius - 5);
    int y1 = tcy + sin(a) * (tradius - 5);

    int x2 = tcx + cos(a) * (tradius - 18);
    int y2 = tcy + sin(a) * (tradius - 18);

    uint16_t col = (t == 0 || t == 100) ? TFT_YELLOW : TFT_DARKGREY;

    tempTFT->drawLine(x1, y1, x2, y2, col);

    // Number position
    int tx = tcx + cos(a) * (tradius - 32);
    int ty = tcy + sin(a) * (tradius - 32);

    char buf[6];
    sprintf(buf, "%d", t);

    // Upright centered numbers
    drawCenterText(tempTFT, buf, tx, ty, 2);// controls radial temp font size 2 is kinda too big
  }

  // ---------- Needle ----------
  float na = map(temperatureF * 10,
                 -200, 1200,
                 -1400, 1400) / 10.0 * DEG_TO_RAD;

  // Rotate needle too
  na += ROT;

  int nx = tcx + cos(na) * (tradius - 25);
  int ny = tcy + sin(na) * (tradius - 25);

  tempTFT->drawLine(tcx, tcy, nx, ny, TFT_GREEN);
  tempTFT->drawLine(tcx + 1, tcy, nx, ny, TFT_GREEN);

  tempTFT->fillCircle(tcx, tcy, 6, TFT_WHITE);

  // ---------- Digital Value (UNCHANGED) ----------
  tempTFT->setTextColor(TFT_CYAN, TFT_BLACK);
  tempTFT->setTextSize(2);

  char vbuf[16];
  sprintf(vbuf, "%.1f F", temperatureF);

  drawCenterText(tempTFT, vbuf, tcx, tcy + 45, 2); //controls digital temp font size 3 is too big 
}





/*{
  tempTFT->fillScreen(TFT_BLACK);

  // Dial
  tempTFT->drawCircle(tcx, tcy, tradius, TFT_WHITE);
  tempTFT->drawCircle(tcx, tcy, tradius - 2, TFT_WHITE);

  for (int i = -20; i <= 120; i += 10)
  {
    float a = map(i, -20, 120, -140, 140) * DEG_TO_RAD;

    int x1 = tcx + cos(a) * (tradius - 5);
    int y1 = tcy + sin(a) * (tradius - 5);

    int x2 = tcx + cos(a) * (tradius - 18);
    int y2 = tcy + sin(a) * (tradius - 18);

    uint16_t col = (i == 0 || i == 100) ? TFT_YELLOW : TFT_DARKGREY;

    tempTFT->drawLine(x1, y1, x2, y2, col);
  }

  // Needle
  float a = map(temperatureF * 10,
                -200, 1200,
                -1400, 1400) / 10.0 * DEG_TO_RAD;

  int tx = tcx + cos(a) * (tradius - 25);
  int ty = tcy + sin(a) * (tradius - 25);

  tempTFT->drawLine(tcx, tcy, tx, ty, TFT_GREEN);
  tempTFT->drawLine(tcx + 1, tcy, tx, ty, TFT_GREEN);

  tempTFT->fillCircle(tcx, tcy, 6, TFT_WHITE);

  // Value
  tempTFT->setTextColor(TFT_CYAN, TFT_BLACK);
  tempTFT->setTextSize(2);

  char buf[16];
  sprintf(buf, "%.1f F", temperatureF);

  drawCenterText(tempTFT, buf, tcx, tcy + 45, 2);
}*/
