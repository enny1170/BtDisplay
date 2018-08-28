#include <wifi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "images.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <SPIFFS.h>

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
#define DISPLAY_HIGHT 64
#define DISPLAY_SCL_PIN 15
#define DISPLAY_SDA_PIN 4
#define DISPLAY_RST_PIN 16
#define SSD1306_128_64

#ifdef DISPLAY_RST_PIN
    #define OLED_RESET DISPLAY_RST_PIN
#endif

// Helper to find Display Address, comment it in to do a I2C Scan
// #define DO_I2C_SCAN


Adafruit_SSD1306 display(OLED_RESET);
String line;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncEventSource events("/events");
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

    SPIFFS.begin(true); 

    ws.onEvent(onWsEvent);
    server.addHandler(&ws);

    events.onConnect([](AsyncEventSourceClient *client){
        client->send("hello!",NULL,millis(),1000);
    });
    server.addHandler(&events);

  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.htm");

  server.onNotFound([](AsyncWebServerRequest *request){
    Serial.printf("NOT_FOUND: ");
    if(request->method() == HTTP_GET)
      Serial.printf("GET");
    else if(request->method() == HTTP_POST)
      Serial.printf("POST");
    else if(request->method() == HTTP_DELETE)
      Serial.printf("DELETE");
    else if(request->method() == HTTP_PUT)
      Serial.printf("PUT");
    else if(request->method() == HTTP_PATCH)
      Serial.printf("PATCH");
    else if(request->method() == HTTP_HEAD)
      Serial.printf("HEAD");
    else if(request->method() == HTTP_OPTIONS)
      Serial.printf("OPTIONS");
    else
      Serial.printf("UNKNOWN");
    Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

    if(request->contentLength()){
      Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
      Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
    }

    int headers = request->headers();
    int i;
    for(i=0;i<headers;i++){
      AsyncWebHeader* h = request->getHeader(i);
      Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
    }

    int params = request->params();
    for(i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isFile()){
        Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
      } else if(p->isPost()){
        Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      } else {
        Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }

    request->send(404);
    });
    server.onFileUpload([](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final){
    if(!index)
      Serial.printf("UploadStart: %s\n", filename.c_str());
    Serial.printf("%s", (const char*)data);
    if(final)
      Serial.printf("UploadEnd: %s (%u)\n", filename.c_str(), index+len);
    });
    server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    if(!index)
      Serial.printf("BodyStart: %u\n", total);
    Serial.printf("%s", (const char*)data);
    if(index + len == total)
      Serial.printf("BodyEnd: %u\n", total);
    });
    server.begin();
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

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_CONNECT){
    Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
    client->printf("Hello Client %u :)", client->id());
    client->ping();
  } else if(type == WS_EVT_DISCONNECT){
    Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
  } else if(type == WS_EVT_ERROR){
    Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
  } else if(type == WS_EVT_PONG){
    Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
  } else if(type == WS_EVT_DATA){
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    String msg = "";
    if(info->final && info->index == 0 && info->len == len){
      //the whole message is in a single frame and we got all of it's data
      Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);

      if(info->opcode == WS_TEXT){
        for(size_t i=0; i < info->len; i++) {
          msg += (char) data[i];
        }
      } else {
        char buff[3];
        for(size_t i=0; i < info->len; i++) {
          sprintf(buff, "%02x ", (uint8_t) data[i]);
          msg += buff ;
        }
      }
      Serial.printf("%s\n",msg.c_str());

      if(info->opcode == WS_TEXT)
        client->text("I got your text message");
      else
        client->binary("I got your binary message");
    } else {
      //message is comprised of multiple frames or the frame is split into multiple packets
      if(info->index == 0){
        if(info->num == 0)
          Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
        Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
      }

      Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT)?"text":"binary", info->index, info->index + len);

      if(info->opcode == WS_TEXT){
        for(size_t i=0; i < info->len; i++) {
          msg += (char) data[i];
        }
      } else {
        char buff[3];
        for(size_t i=0; i < info->len; i++) {
          sprintf(buff, "%02x ", (uint8_t) data[i]);
          msg += buff ;
        }
      }
      Serial.printf("%s\n",msg.c_str());

      if((info->index + len) == info->len){
        Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
        if(info->final){
          Serial.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
          if(info->message_opcode == WS_TEXT)
            client->text("I got your text message");
          else
            client->binary("I got your binary message");
        }
      }
    }
  }
}
