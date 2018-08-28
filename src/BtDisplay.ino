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

static const int timewarning1 = 6; // display timeago after this count of minutes
static const int timewarning2 = 10; // display timeago and crossed out BG after this count of minutes

const size_t bufferSize = 3*JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(10) + 250;
DynamicJsonBuffer jsonBuffer(bufferSize);

// Display definitions
#define DISPLAY_ADDRESS 0x3C
#define DISPLAY_WIDTH 128
#define DISPLAY_HIGHT 32
#define DISPLAY_SCL_PIN 15
#define DISPLAY_SDA_PIN 4
#define DISPLAY_RST_PIN 16
#define SSD1306_128_64

#ifdef DISPLAY_RST_PIN
    #define OLED_RESET DISPLAY_RST_PIN
#endif

// Helper to find Display Address, comment it in to do a I2C Scan
// #define DO_I2C_SCAN

// arrow bitmaps
static const unsigned char PROGMEM imgDblUp[64] =
  { 0x00, 0x40, 0x02, 0x00, 
    0x00, 0xe0, 0x07, 0x00, 
    0x01, 0xf0, 0x0f, 0x80, 
    0x03, 0xf8, 0x1f, 0xc0, 
    0x07, 0xfc, 0x3f, 0xe0, 
    0x0e, 0xee, 0x77, 0x70, 
    0x0c, 0xe6, 0x67, 0x30, 
    0x00, 0xe0, 0x07, 0x00, 
    0x00, 0xe0, 0x07, 0x00, 
    0x00, 0xe0, 0x07, 0x00, 
    0x00, 0xe0, 0x07, 0x00, 
    0x00, 0xe0, 0x07, 0x00, 
    0x00, 0xe0, 0x07, 0x00, 
    0x00, 0xe0, 0x07, 0x00, 
    0x00, 0xe0, 0x07, 0x00, 
    0x00, 0x00, 0x00, 0x00};
static const unsigned char PROGMEM imgUp[64] =
  { 0x00, 0x00, 0x02, 0x00, 
    0x00, 0x00, 0x07, 0x00, 
    0x00, 0x00, 0x0f, 0x80, 
    0x00, 0x00, 0x1f, 0xc0, 
    0x00, 0x00, 0x3f, 0xe0, 
    0x00, 0x00, 0x77, 0x70, 
    0x00, 0x00, 0x67, 0x30, 
    0x00, 0x00, 0x07, 0x00, 
    0x00, 0x00, 0x07, 0x00, 
    0x00, 0x00, 0x07, 0x00, 
    0x00, 0x00, 0x07, 0x00, 
    0x00, 0x00, 0x07, 0x00, 
    0x00, 0x00, 0x07, 0x00, 
    0x00, 0x00, 0x07, 0x00, 
    0x00, 0x00, 0x07, 0x00, 
    0x00, 0x00, 0x00, 0x00};
static const unsigned char PROGMEM imgUp45[64] =
  { 0x00, 0x00, 0xff, 0xff, 
    0x00, 0x00, 0xff, 0xff, 
    0x00, 0x00, 0x01, 0xff, 
    0x00, 0x00, 0x07, 0xff, 
    0x00, 0x00, 0x1f, 0xff, 
    0x00, 0x00, 0x7f, 0x8f, 
    0x00, 0x01, 0xfe, 0x0f, 
    0x00, 0x07, 0xf8, 0x0f, 
    0x00, 0x1f, 0xe0, 0x00, 
    0x00, 0x7f, 0x80, 0x00, 
    0x01, 0xfe, 0x00, 0x00, 
    0x07, 0xf8, 0x00, 0x00, 
    0x0f, 0xe0, 0x00, 0x00, 
    0x0f, 0x80, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00};
static const unsigned char PROGMEM imgFlat[64] =
  { 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0xff, 0x00, 
    0x00, 0x00, 0xff, 0xc0, 
    0x00, 0x00, 0x0f, 0xf0, 
    0x00, 0x00, 0x03, 0xfc, 
    0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 
    0x00, 0x00, 0x03, 0xfc, 
    0x00, 0x00, 0x0f, 0xf0, 
    0x00, 0x00, 0xff, 0xc0, 
    0x00, 0x00, 0xff, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00};
static const unsigned char PROGMEM imgDown45[64] =
  { 0x0f, 0x80, 0x00, 0x00, 
    0x0f, 0xe0, 0x00, 0x00, 
    0x07, 0xf8, 0x00, 0x00, 
    0x01, 0xfe, 0x00, 0x00, 
    0x00, 0x7f, 0x80, 0x00, 
    0x00, 0x1f, 0xe0, 0x00, 
    0x00, 0x07, 0xf8, 0x0f, 
    0x00, 0x01, 0xfe, 0x0f, 
    0x00, 0x00, 0x7f, 0x8f, 
    0x00, 0x00, 0x1f, 0xff, 
    0x00, 0x00, 0x07, 0xff, 
    0x00, 0x00, 0x01, 0xff, 
    0x00, 0x00, 0xff, 0xff, 
    0x00, 0x00, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00};
static const unsigned char PROGMEM imgDown[64] =
  { 0x00, 0x00, 0x07, 0x00, 
    0x00, 0x00, 0x07, 0x00, 
    0x00, 0x00, 0x07, 0x00, 
    0x00, 0x00, 0x07, 0x00, 
    0x00, 0x00, 0x07, 0x00, 
    0x00, 0x00, 0x07, 0x00, 
    0x00, 0x00, 0x07, 0x00, 
    0x00, 0x00, 0x07, 0x00, 
    0x00, 0x00, 0x67, 0x30, 
    0x00, 0x00, 0x77, 0x70, 
    0x00, 0x00, 0x3f, 0xe0, 
    0x00, 0x00, 0x1f, 0xc0, 
    0x00, 0x00, 0x0f, 0x80, 
    0x00, 0x00, 0x07, 0x00, 
    0x00, 0x00, 0x02, 0x00, 
    0x00, 0x00, 0x00, 0x00};
static const unsigned char PROGMEM imgDblDown[64] =
  { 0x00, 0xe0, 0x07, 0x00, 
    0x00, 0xe0, 0x07, 0x00, 
    0x00, 0xe0, 0x07, 0x00, 
    0x00, 0xe0, 0x07, 0x00, 
    0x00, 0xe0, 0x07, 0x00, 
    0x00, 0xe0, 0x07, 0x00, 
    0x00, 0xe0, 0x07, 0x00, 
    0x00, 0xe0, 0x07, 0x00, 
    0x0c, 0xe6, 0x67, 0x30, 
    0x0e, 0xee, 0x77, 0x70, 
    0x07, 0xfc, 0x3f, 0xe0, 
    0x03, 0xf8, 0x1f, 0xc0, 
    0x01, 0xf0, 0x0f, 0x80, 
    0x00, 0xe0, 0x07, 0x00, 
    0x00, 0x40, 0x02, 0x00, 
    0x00, 0x00, 0x00, 0x00};

Adafruit_SSD1306 display(OLED_RESET);
String line;
//int size=1;

//extern void I2CScanner();

void setup()
{
    Serial.begin(115200);
    #ifdef DISPLAY_SCL_PIN
        // if you use the Display with nos standard I2C Pins, you have to comment out the Wire.begin() 
        // in the Adafruit_SSD1306.cpp Line 209 or use this patched version 
        Wire.begin(DISPLAY_SDA_PIN,DISPLAY_SCL_PIN);
    #else
        Wire.begin();
    #endif
/*     #ifdef DISPLAY_RST_PIN
        digitalWrite(DISPLAY_RST_PIN,LOW);
        delay(10);
        digitalWrite(DISPLAY_RST_PIN,HIGH);
    #endif */
    display.begin(SSD1306_SWITCHCAPVCC, DISPLAY_ADDRESS);

    // Clear the buffer.
    display.clearDisplay();


    // int iDelay;
    // int iw;
    // int ih;
    // iDelay = 2000;
    // iw=32;
    // ih=16;

    // display.drawBitmap(96,0,imgDblUp,iw,ih,1);
    // display.display();
    // delay(iDelay);
    // display.clearDisplay();

    // display.drawBitmap(96,0,imgDblDown,iw,ih,1);
    // display.display();
    // delay(iDelay);
    // display.clearDisplay();
    
    // display.drawBitmap(96,0,imgUp45,iw,ih,1);
    // display.display();
    // delay(iDelay);
    // display.clearDisplay();
    
    // display.drawBitmap(96,0,imgDown45,iw,ih,1);
    // display.display();
    // delay(iDelay);
    // display.clearDisplay();
    
    // display.drawBitmap(96,0,imgUp,iw,ih,1);
    // display.display();
    // delay(iDelay);
    // display.clearDisplay();
    
    // display.drawBitmap(96,0,imgDown,iw,ih,1);
    // display.display();
    // delay(iDelay);
    // display.clearDisplay();
    
    // display.drawBitmap(96,0,imgFlat,iw,ih,1);
    // display.display();
    // delay(iDelay);
    // display.clearDisplay();

    // We start by connecting to a WiFi network
    
    // debug output
    // Serial.println();
    // Serial.println();
    // Serial.print("Connecting to ");
    // Serial.println(ssid);
    
    // init display
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setCursor(0, 0);

    #ifdef DO_I2C_SCAN
        Serial.println("----------------------------------------------------------------------------------------");
        I2CScanner();
        Serial.println("----------------------------------------------------------------------------------------");
    #endif

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
    {   Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
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
    //root.prettyPrintTo(Serial);
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
    // // write out Debug Values
    // Serial.println("Json results:");
    // Serial.print("sgv: ");
    // Serial.println(bgs0_sgv);
    // Serial.print("trend: ");
    // Serial.println(bgs0_trend);
    // Serial.print("direction: ");
    // Serial.println(bgs0_direction);

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
       // Serial.print(".");
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

    // Serial.println("");
    //delay(500);
    display.clearDisplay();
    display.setCursor(0,2);
    display.println("WiFi connected");
    //Serial.println("WiFi connected");
    display.print("IP: ");
    Serial.println("IP address: ");
    display.println(WiFi.localIP());
    Serial.println(WiFi.localIP());
    display.print("GW: ");
    display.println(WiFi.gatewayIP());
    display.display();
    delay(2000);

}
