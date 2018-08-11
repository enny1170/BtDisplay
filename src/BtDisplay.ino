#include <wifi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const char* ssid     = "myMobile";
const char* password = "VollVergessen!DenScheiss!";
const char* url = "http://192.168.43.1:17580/pebble";

const size_t bufferSize = 3*JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(10) + 250;
DynamicJsonBuffer jsonBuffer(bufferSize);

//DynamicJsonBuffer jsonBuffer;

#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 
static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };

Adafruit_SSD1306 display;
String line;
int size=1;

void setup()
{
    Serial.begin(115200);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

    // Clear the buffer.
    display.clearDisplay();

    display.drawBitmap(30, 16,  logo16_glcd_bmp, 16, 16, 1);
    display.display();
    delay(3);
    // We start by connecting to a WiFi network

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    connectWiFi();
    //display.setTextSize(1);
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setCursor(0,0);
}


void loop()
{

    if(WiFi.isConnected()==false)
    {
        connectWiFi();
    }

    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    HTTPClient http;
    // get data from Webservice    
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0)
    {
        // file found at server
        if (httpCode == HTTP_CODE_OK)
        {
            line = http.getString();
        }
    }
    else
    {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        display.clearDisplay();
        display.setTextSize(1);
        display.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        display.display();
        delay(30000);
        return;
    }

    http.end();

JsonObject &root = jsonBuffer.parseObject(line.c_str());
//root.prettyPrintTo(Serial);
long status0_now = root["status"][0]["now"]; // 1533992301941

JsonObject& bgs0 = root["bgs"][0];
const char* bgs0_sgv = bgs0["sgv"]; // "149"
int bgs0_trend = bgs0["trend"]; // 4
const char* bgs0_direction = bgs0["direction"]; // "Flat"
long bgs0_datetime = bgs0["datetime"]; // 1533992094001
long bgs0_filtered = bgs0["filtered"]; // 116235
long bgs0_unfiltered = bgs0["unfiltered"]; // 116235
int bgs0_noise = bgs0["noise"]; // 1
int bgs0_bgdelta = bgs0["bgdelta"]; // 1
const char* bgs0_battery = bgs0["battery"]; // "70"
int bgs0_iob = bgs0["iob"]; // 0

JsonObject& cals0 = root["cals"][0];
int cals0_scale = cals0["scale"]; // 1
float cals0_slope = cals0["slope"]; // 1441.0746430702911
float cals0_intercept = cals0["intercept"]; // -18801.45209007935
    Serial.println("Json results:");
    Serial.print("sgv: ");
    Serial.println(bgs0_sgv);
    Serial.print("trend: ");
    Serial.println(bgs0_trend);
    Serial.print("direction: ");
    Serial.println(bgs0_direction);

    // Wtite out Data to display
    display.clearDisplay();
    display.setTextSize(4);
    if(bgs0_sgv)
    display.setCursor(2,2);
    display.print(bgs0_sgv);
    drawArrow(bgs0_trend);

    delay(30000);
}


void drawArrow(int trend)
{
    trend=3;
    switch (trend)
    {
        case 0: //NONE
            break;
        case 1: //DOUBLE_UP
            break;
        case 2: //SINGLE_UP
            display.drawLine(88,16,88,2,WHITE);
            display.drawLine(88,2,78,14,WHITE);
            display.drawLine(88,2,98,12,WHITE);
            break;
        case 3: //UP_45
            display.drawLine(88,16,118,4,WHITE);
            display.drawLine(118,4,103,4,WHITE);
            display.drawLine(118,4,118,9,WHITE);
            break;
        case 4: //FLAT
            display.drawLine(88,16,123,16,WHITE);
            display.drawLine(123,16,113,11,WHITE);
            display.drawLine(123,16,113,21,WHITE);
            break;
        case 5: //DOWN_45
            display.drawLine(88,16,118,28,WHITE);
            display.drawLine(118,28,108,28,WHITE);
            display.drawLine(118,28,118,24,WHITE);
            break;
        case 6: //SINGLE_DOWN
            display.drawLine(88,16,88,30,WHITE);
            display.drawLine(88,30,78,20,WHITE);
            display.drawLine(88,30,98,20,WHITE);        
            break;
        case 7: //DOUBLE_DOWN
            break;
        case 8: //NOT_COMPUTABLE
            break;
        case 9: //OUT_OF_RANGE
            break;
        default:
            break;
    }
    display.display();
}

void connectWiFi(void)
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setTextWrap(true);
    display.setCursor(0,0);
    display.print("Connecting to ");
    display.print(ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        display.print(".");
        display.display();
    }

    Serial.println("");
    display.clearDisplay();
    display.setCursor(0,2);
    display.println("WiFi connected");
    Serial.println("WiFi connected");
    display.println("Ip address:");
    Serial.println("IP address: ");
    display.println(WiFi.localIP());
    Serial.println(WiFi.localIP());
    display.display();
    delay(3000);

}
