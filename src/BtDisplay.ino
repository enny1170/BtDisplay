#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "images.h"

//#define DEBUG

const char* ssid     = "myMobile";
const char* password = "VollVergessen!DenScheiss!";
const char* url = "http://192.168.43.1:17580/pebble";

static const int timewarning1 = 6; // display timeago after this count of minutes
static const int timewarning2 = 10; // display timeago and crossed out BG after this count of minutes

const size_t bufferSize = 3*JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(10) + 250;
DynamicJsonBuffer jsonBuffer(bufferSize);

// Display definitions
#define DISPLAY_ADDRESS 0x3C
#define DISPLAY_RST_PIN 16
#define SSD1306_128_64

#ifdef DISPLAY_RST_PIN
    #define OLED_RESET DISPLAY_RST_PIN
#endif

Adafruit_SSD1306 display(OLED_RESET);
String line;

void setup()
{
    #ifdef DEBUG
      Serial.begin(115200);
    #endif
    display.begin(SSD1306_SWITCHCAPVCC, DISPLAY_ADDRESS);

    // Clear the buffer.
    display.clearDisplay();

    // init display
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setCursor(0, 0);

    // start with WiFi connection
    connectWiFi();
}

void loop()
{

    if (WiFi.isConnected() == false)
    {   
        connectWiFi();
    }

    // create HTTP Client
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
        #ifdef DEBUG
          Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        #endif
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextWrap(true);
        display.setCursor(0,0);
        display.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        display.display();
        delay(30000);
        return;
    }

    http.end();

    // create Json Buffer for parsing results
    JsonObject &root = jsonBuffer.parseObject(line.c_str());
    long status0_now = root["status"][0]["now"]; // 1533992301941

    JsonObject &bgs0 = root["bgs"][0];
    const char *bgs0_sgv = bgs0["sgv"];             // "149"
    int bgs0_trend = bgs0["trend"];                 // 4
    const char *bgs0_direction = bgs0["direction"]; // "Flat"
    long bgs0_datetime = bgs0["datetime"];          // 1533992094001
    long bgs0_filtered = bgs0["filtered"];          // 116235
    long bgs0_unfiltered = bgs0["unfiltered"];      // 116235
    int bgs0_noise = bgs0["noise"];                 // 1
    //int bgs0_bgdelta = bgs0["bgdelta"];             // 1
    signed int bgs0_bgdelta = bgs0["bgdelta"];             // 1
    const char *bgs0_battery = bgs0["battery"];     // "70"
    int bgs0_iob = bgs0["iob"];                     // 0

    JsonObject &cals0 = root["cals"][0];
    int cals0_scale = cals0["scale"];           // 1
    float cals0_slope = cals0["slope"];         // 1441.0746430702911
    float cals0_intercept = cals0["intercept"]; // -18801.45209007935
    #ifdef DEBUG
      //  write out Debug Values
      Serial.println("Json results:");
      Serial.print("sgv: ");
      Serial.println(bgs0_sgv);
      Serial.print("trend: ");
      Serial.println(bgs0_trend);
      Serial.print("direction: ");
      Serial.println(bgs0_direction);
    #endif

    // Write out Data to display
    display.clearDisplay();
    int olddata = 0;
    int timeago = (int)((status0_now - bgs0_datetime) / 1000 /60);

    display.setTextSize(4);
    
    if ( timeago >= timewarning2)
    {   olddata = 2;
        display.setTextSize(3);
    }
    else if (timeago >= timewarning1)
    {
        olddata = 1;
        display.setTextSize(3);
    }

    // set Offset for Values smaller than 100
    int sgvValue = atoi(bgs0_sgv);
    int offset;
    if (sgvValue > 99)
    {   offset = 0;
        display.setCursor(2 + offset , 2);
    }
    else
    {   offset = 19;
        display.setCursor(2 + offset, 2);
    }

    if (sgvValue < 90)
    {
        display.invertDisplay(5);
    }
    else
    {
        display.invertDisplay(0);
    }

    display.print(bgs0_sgv);

if (olddata > 0)
{   display.setTextSize(1);
    display.setCursor(2, 24);
    display.print("-- ");
    display.print(((long)(status0_now - bgs0_datetime) / 1000 /60));
    display.println(" min --");
}
if (olddata > 1)
{   
    //display.drawLine(0,12,64,12,WHITE);
    display.drawLine(0 + offset,13,58,13,WHITE);
    display.drawLine(0 + offset,14,58,14,WHITE);
}

    display.setTextSize(2);
    display.setCursor(88,16);

    if(bgs0_bgdelta > 0)
    {
        display.print("+");
        display.print(bgs0_bgdelta);
    }
    else 
    {
        display.print(bgs0_bgdelta);
    }
    
    drawArrow(bgs0_trend);
    
    // Wait 15 sec before polling the next BG
    delay(15000);
}

void drawArrow(int trend)
{
    //trend=4;
    switch (trend)
    {
        case 0: //NONE
            break;
        case 1: //DOUBLE_UP
            display.drawBitmap(96,0,imgDblUp,32,16,1);
            break;
        case 2: //SINGLE_UP
            display.drawBitmap(96,0,imgUp,32,16,1);
            break;
        case 3: //UP_45
            display.drawBitmap(96,0,imgUp45,32,16,1);
            break;
        case 4: //FLAT
            display.drawBitmap(96,0,imgFlat,32,16,1);
            break;
        case 5: //DOWN_45
            display.drawBitmap(96,0,imgDown45,32,16,1);
            break;
        case 6: //SINGLE_DOWN
            display.drawBitmap(96,0,imgDown,32,16,1);      
            break;
        case 7: //DOUBLE_DOWN
            display.drawBitmap(96,0,imgDblDown,32,16,1);
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
{   int i;
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    //display.setTextWrap(true);
    display.setCursor(0,0);
    //display.print("Connecting to ");
    //display.print(ssid);
    display.display();
    //jro
        // try to solve WiFi connect Error
        WiFi.setAutoConnect(1);
    //jro
    WiFi.begin(ssid, password);
    display.setCursor(0,0);
    display.println("Connecting to ");
    display.print(ssid);
    display.display();

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        display.print(".");
        display.display();
        //jro
        // sometimes WiFi was not connecting
            i ++;
            if (i > 4)
            {
                WiFi.reconnect();
            i = 0;}
        //jro
    }

    //delay(500);
    display.clearDisplay();
    display.setCursor(0,2);
    display.println("WiFi connected");
    #ifdef DEBUG
        Serial.println("WiFi connected");
    #endif
    display.print("IP: ");
    #ifdef DEBUG
        Serial.println("IP address: ");
    #endif
    display.println(WiFi.localIP());
    #ifdef DEBUG
        Serial.println(WiFi.localIP());
    #endif
    display.print("GW: ");
    display.println(WiFi.gatewayIP());
    display.display();
    delay(2000);
}
