#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <Wire.h>
using WebServerClass = WebServer;
fs::SPIFFSFS &FlashFS = SPIFFS;
#define FORMAT_ON_FAIL true
#include <JC_Button.h>
#include "qrcoded.h"

#include <AutoConnect.h>
#include <ArduinoJson.h>

#define PARAM_FILE "/elements.json"

/////////////////////////////////
///////////CHANGE////////////////
/////////////////////////////////

bool usingM5 = true; // false if not using M5Stack          
bool format = false; // true for formatting SPIFFS, use once, then make false and reflash
int portalPin = 4;

/////////////////////////////////
/////////////////////////////////
/////////////////////////////////

// Access point variables
String password;
String lnbitsServer;
String lnurlP;
String invoiceKey;
String highPin;
String timePin;
String amount;
String dataId;
String description;
String payReq;

int balance;
int oldBalance;

bool paid;
bool down = false;
bool triggerAp = false; 

String content = "<h1>Base Access-point</br>For easy variable and wifi connection setting</h1>";

// custom access point pages
static const char PAGE_ELEMENTS[] PROGMEM = R"(
{
  "uri": "/config",
  "title": "Access Point Config",
  "menu": true,
  "element": [
    {
      "name": "text",
      "type": "ACText",
      "value": "AP options",
      "style": "font-family:Arial;font-size:16px;font-weight:400;color:#191970;margin-botom:15px;"
    },
    {
      "name": "password",
      "type": "ACInput",
      "label": "Password",
      "value": "ToTheMoon"
    },
    {
      "name": "text",
      "type": "ACText",
      "value": "Project options",
      "style": "font-family:Arial;font-size:16px;font-weight:400;color:#191970;margin-botom:15px;"
    },
    {
      "name": "server",
      "type": "ACInput",
      "label": "LNbits server",
      "value": "legend.lnbits.com"
    },
    {
      "name": "lnurl",
      "type": "ACInput",
      "label": "LNURLPay optional for static QR",
      "value": ""
    },
    {
      "name": "invoicekey",
      "type": "ACInput",
      "label": "LNbits invoice key",
      "value": ""
    },
    {
      "name": "pin",
      "type": "ACInput",
      "label": "Pin to turn on",
      "value": ""
    },
    {
      "name": "time",
      "type": "ACInput",
      "label": "Time to turn on pin",
      "value": ""
    },
    {
      "name": "amount",
      "type": "ACInput",
      "label": "Amount to check for",
      "value": ""
    },
    {
      "name": "load",
      "type": "ACSubmit",
      "value": "Load",
      "uri": "/config"
    },
    {
      "name": "save",
      "type": "ACSubmit",
      "value": "Save",
      "uri": "/save"
    },
    {
      "name": "adjust_width",
      "type": "ACElement",
      "value": "<script type='text/javascript'>window.onload=function(){var t=document.querySelectorAll('input[]');for(i=0;i<t.length;i++){var e=t[i].getAttribute('placeholder');e&&t[i].setAttribute('size',e.length*.8)}};</script>"
    }
  ]
 }
)";

static const char PAGE_SAVE[] PROGMEM = R"(
{
  "uri": "/save",
  "title": "Elements",
  "menu": false,
  "element": [
    {
      "name": "caption",
      "type": "ACText",
      "format": "Elements have been saved to %s",
      "style": "font-family:Arial;font-size:18px;font-weight:400;color:#191970"
    },
    {
      "name": "validated",
      "type": "ACText",
      "style": "color:red"
    },
    {
      "name": "echo",
      "type": "ACText",
      "style": "font-family:monospace;font-size:small;white-space:pre;"
    },
    {
      "name": "ok",
      "type": "ACSubmit",
      "value": "OK",
      "uri": "/config"
    }
  ]
}
)";

TFT_eSPI tft = TFT_eSPI();

WebServerClass server;
AutoConnect portal(server);
AutoConnectConfig config;
AutoConnectAux elementsAux;
AutoConnectAux saveAux;

void setup()
{
  Serial.begin(115200);
  Wire.begin();
  if(usingM5 == true){
    tft.init();
    tft.setRotation(1);
    tft.invertDisplay(true);
    logoScreen();
  }
  const byte BUTTON_PIN_A = 39;
  Button BTNA(BUTTON_PIN_A);
  BTNA.begin();
  int timer = 0;
  pinMode (2, OUTPUT);
  while (timer < 2000)
  {
    digitalWrite(2, LOW);
    if (usingM5 == true){
      if (BTNA.read() == 1){
        Serial.println("Launch portal");
        triggerAp = true;
        timer = 5000;
      }
    }
    else{
      Serial.println(touchRead(portalPin));
      if(touchRead(portalPin) < 60){
        Serial.println("Launch portal");
        triggerAp = true;
        timer = 5000;
      }
    }
    digitalWrite(2, HIGH);
    timer = timer + 100;
    delay(300);
  }
    
 // h.begin();
  FlashFS.begin(FORMAT_ON_FAIL);
  SPIFFS.begin(true);
  if(format == true){
    SPIFFS.format(); 
  }
  // get the saved details and store in global variables
  File paramFile = FlashFS.open(PARAM_FILE, "r");
  if (paramFile)
  {
    StaticJsonDocument<2500> doc;
    DeserializationError error = deserializeJson(doc, paramFile.readString());

    const JsonObject maRoot0 = doc[0];
    const char *maRoot0Char = maRoot0["value"];
    password = maRoot0Char;
    
    const JsonObject maRoot1 = doc[1];
    const char *maRoot1Char = maRoot1["value"];
    lnbitsServer = maRoot1Char;

    const JsonObject maRoot2 = doc[2];
    const char *maRoot2Char = maRoot2["value"];
    lnurlP = maRoot2Char;
    lnurlP.toUpperCase();

    const JsonObject maRoot3 = doc[3];
    const char *maRoot3Char = maRoot3["value"];
    invoiceKey = maRoot3Char;

    const JsonObject maRoot4 = doc[4];
    const char *maRoot4Char = maRoot4["value"];
    highPin = maRoot4Char;
    
    const JsonObject maRoot5 = doc[5];
    const char *maRoot5Char = maRoot5["value"];
    timePin = maRoot5Char;

    const JsonObject maRoot6 = doc[6];
    const char *maRoot6Char = maRoot6["value"];
    amount = maRoot6Char;
  }
  else{
    triggerAp = true;
  }
  paramFile.close();
    server.on("/", []() {
      content += AUTOCONNECT_LINK(COG_24);
      server.send(200, "text/html", content);
    });
    
    elementsAux.load(FPSTR(PAGE_ELEMENTS));
    elementsAux.on([](AutoConnectAux &aux, PageArgument &arg) {
      File param = FlashFS.open(PARAM_FILE, "r");
      if (param)
      {
        aux.loadElement(param, {"password", "server", "lnurl", "invoicekey", "pin", "time", "amount"});
        param.close();
      }

      if (portal.where() == "/config")
      {
        File param = FlashFS.open(PARAM_FILE, "r");
        if (param)
        {
          aux.loadElement(param, {"password", "server", "lnurl", "invoicekey", "pin", "time", "amount"});
          param.close();
        }
      }
      return String();
    });
    saveAux.load(FPSTR(PAGE_SAVE));
    saveAux.on([](AutoConnectAux &aux, PageArgument &arg) {
      aux["caption"].value = PARAM_FILE;
      File param = FlashFS.open(PARAM_FILE, "w");
      if (param)
      {
        // save as a loadable set for parameters.
        elementsAux.saveElement(param, {"password", "server", "lnurl", "invoicekey", "pin", "time", "amount"});
        param.close();
        // read the saved elements again to display.
        param = FlashFS.open(PARAM_FILE, "r");
        aux["echo"].value = param.readString();
        param.close();
      }
      else
      {
        aux["echo"].value = "Filesystem failed to open.";
      }
      return String();
    });
    config.auth = AC_AUTH_BASIC;
    config.authScope = AC_AUTHSCOPE_AUX;
    config.ticker = true;
    config.autoReconnect = true;
    config.apid = "Device-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    config.psk = password;
    config.menuItems = AC_MENUITEM_CONFIGNEW | AC_MENUITEM_OPENSSIDS | AC_MENUITEM_RESET;
    config.reconnectInterval = 1;

    if (triggerAp == true)
    {
      portalLaunched();
      config.immediateStart = true;
      portal.join({elementsAux, saveAux});
      portal.config(config);
      portal.begin();
      while (true)
      {
        portal.handleClient();
      }
      timer = 2000;
    }
    timer = timer + 200;
    delay(200);
  if (invoiceKey != "")
  {
    portal.join({elementsAux, saveAux});
    config.autoRise = false;
    portal.config(config);
    portal.begin();
  }
  triggerAp = false;
  lnbitsScreen();
  delay(1000);
  pinMode(highPin.toInt(), OUTPUT);
  digitalWrite(highPin.toInt(), HIGH);
  delay(1000);
  digitalWrite(highPin.toInt(), LOW);
}

void loop() {
  while(WiFi.status() != WL_CONNECTED){
    if(usingM5 == true){
      connectionError();
    }
    Serial.println("Failed to connect");
    delay(500);
  }
  Serial.println(highPin.toInt());
  Serial.println(timePin.toInt());
  Serial.println(lnurlP.substring(0, 5));
  if(lnurlP.substring(0, 5) == "LNURL"){
    if(usingM5 == true){
      qrdisplayScreen();
    }
    checkBalance();
    oldBalance = balance;
    while((oldBalance + amount.toInt()) > balance){
      checkBalance();
      delay(2000);
    }
    oldBalance = balance;
    Serial.println("Paid");
    if(usingM5 == true){
      paidScreen();
    }
    digitalWrite(highPin.toInt(), HIGH);
    delay(timePin.toInt());
    digitalWrite(highPin.toInt(), LOW); 
    delay(2000);
    paid = false;
  }
  else{
    getInvoice();
    if(down){
      errorScreen();
      getInvoice();
      delay(5000);
    }
    if(payReq != ""){
      if(usingM5 == true){
        qrdisplayScreen();
      }
      delay(5000);
    }
    while(paid == false && payReq != ""){
      checkInvoice();
      if(paid){
        Serial.println("Paid");
        if(usingM5 == true){
          completeScreen();
        }
        digitalWrite(highPin.toInt(), HIGH);
        delay(timePin.toInt());
        digitalWrite(highPin.toInt(), LOW); 
      }
      delay(2000);
    }
    payReq = "";
    dataId = "";
    paid = false;
    delay(4000);
  }
}

//////////////////DISPLAY///////////////////

void serverError()
{
  tft.fillScreen(TFT_WHITE);
  tft.setCursor(0, 80);
  tft.setTextSize(3);
  tft.setTextColor(TFT_RED);
  tft.println("Server connect fail");
}

void connectionError()
{
  tft.fillScreen(TFT_WHITE);
  tft.setCursor(0, 80);
  tft.setTextSize(3);
  tft.setTextColor(TFT_RED);
  tft.println("Wifi connect fail");
}

void connection()
{
  tft.fillScreen(TFT_WHITE);
  tft.setCursor(0, 80);
  tft.setTextSize(3);
  tft.setTextColor(TFT_RED);
  tft.println("Wifi connected");
}

void logoScreen()
{ 
  tft.fillScreen(TFT_WHITE);
  tft.setCursor(0, 80);
  tft.setTextSize(4);
  tft.setTextColor(TFT_PURPLE);
  tft.println("bitcoinSwitch");
}

void portalLaunched()
{ 
  tft.fillScreen(TFT_WHITE);
  tft.setCursor(0, 80);
  tft.setTextSize(4);
  tft.setTextColor(TFT_PURPLE);
  tft.println("PORTAL LAUNCH");
}

void processingScreen()
{ 
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(40, 80);
  tft.setTextSize(4);
  tft.setTextColor(TFT_WHITE);
  tft.println("PROCESSING");
}

void lnbitsScreen()
{ 
  tft.fillScreen(TFT_WHITE);
  tft.setCursor(10, 90);
  tft.setTextSize(3);
  tft.setTextColor(TFT_BLACK);
  tft.println("POWERED BY LNBITS");
}

void portalScreen()
{ 
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(30, 80);
  tft.setTextSize(3);
  tft.setTextColor(TFT_WHITE);
  tft.println("PORTAL LAUNCHED");
}

void paidScreen()
{ 
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(110, 80);
  tft.setTextSize(4);
  tft.setTextColor(TFT_WHITE);
  tft.println("PAID");
}

void completeScreen()
{ 
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(60, 80);
  tft.setTextSize(4);
  tft.setTextColor(TFT_WHITE);
  tft.println("COMPLETE");
}

void errorScreen()
{ 
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(70, 80);
  tft.setTextSize(4);
  tft.setTextColor(TFT_WHITE);
  tft.println("ERROR");
}

void qrdisplayScreen()
{
  String qrCodeData;
  if(lnurlP.substring(0, 5) == "LNURL"){
    qrCodeData = lnurlP;
  }
  else{
    qrCodeData = payReq;
  }
  tft.fillScreen(TFT_WHITE);
  qrCodeData.toUpperCase();
  const char *qrDataChar = qrCodeData.c_str();
  QRCode qrcoded;
  uint8_t qrcodeData[qrcode_getBufferSize(20)];
  qrcode_initText(&qrcoded, qrcodeData, 11, 0, qrDataChar);
  for (uint8_t y = 0; y < qrcoded.size; y++)
  {
    // Each horizontal module
    for (uint8_t x = 0; x < qrcoded.size; x++)
    {
      if (qrcode_getModule(&qrcoded, x, y))
      {
        tft.fillRect(65 + 3 * x, 20 + 3 * y, 3, 3, TFT_BLACK);
      }
      else
      {
        tft.fillRect(65 + 3 * x, 20 + 3 * y, 3, 3, TFT_WHITE);
      }
    }
  }
}

//////////////////NODE CALLS///////////////////

void checkConnection(){
  WiFiClientSecure client;
  client.setInsecure();
  const char* lnbitsserver = lnbitsServer.c_str();
  const char* invoicekey = invoiceKey.c_str();
  if (!client.connect(lnbitsserver, 443)){
    down = true;
    serverError();
    return;   
  }
}

void checkBalance(){
  WiFiClientSecure client;
  client.setInsecure();
  const char* lnbitsserver = lnbitsServer.c_str();
  const char* invoicekey = invoiceKey.c_str();
  if (!client.connect(lnbitsserver, 443)){
    down = true;
    return;   
  }

  String url = "/api/v1/wallet";
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                "Host: " + lnbitsserver + "\r\n" +
                "X-Api-Key: "+ invoicekey +" \r\n" +
                "User-Agent: ESP32\r\n" +
                "Content-Type: application/json\r\n" +
                "Connection: close\r\n\r\n");
   while (client.connected()) {
   String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
    if (line == "\r") {
      break;
    }
  }
  String line = client.readString();
  Serial.println(line);
  StaticJsonDocument<500> doc;
  DeserializationError error = deserializeJson(doc, line);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  int charBalance = doc["balance"];
  balance = charBalance;
}


void getInvoice() {
  WiFiClientSecure client;
  client.setInsecure();
  const char* lnbitsserver = lnbitsServer.c_str();
  const char* invoicekey = invoiceKey.c_str();
  const char* lnbitsamount = amount.c_str();
  const char* lnbitsdescription = description.c_str();

  if (!client.connect(lnbitsserver, 443)){
    down = true;
    return;   
  }

  String topost = "{\"out\": false,\"amount\" : " + String(lnbitsamount) + ", \"memo\" :\""+ String(lnbitsdescription) + String(random(1,1000)) + "\"}";
  String url = "/api/v1/payments";
  client.print(String("POST ") + url +" HTTP/1.1\r\n" +
                "Host: " + lnbitsserver + "\r\n" +
                "User-Agent: ESP32\r\n" +
                "X-Api-Key: "+ invoicekey +" \r\n" +
                "Content-Type: application/json\r\n" +
                "Connection: close\r\n" +
                "Content-Length: " + topost.length() + "\r\n" +
                "\r\n" + 
                topost + "\n");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
    if (line == "\r") {
      break;
    }
  }
  
  String line = client.readString();

  StaticJsonDocument<1000> doc;
  DeserializationError error = deserializeJson(doc, line);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  const char* payment_hash = doc["checking_id"];
  const char* payment_request = doc["payment_request"];
  payReq = payment_request;
  dataId = payment_hash;
}


void checkInvoice(){
  WiFiClientSecure client;
  client.setInsecure();
  const char* lnbitsserver = lnbitsServer.c_str();
  const char* invoicekey = invoiceKey.c_str();
  if (!client.connect(lnbitsserver, 443)){
    down = true;
    return;   
  }

  String url = "/api/v1/payments/";
  client.print(String("GET ") + url + dataId +" HTTP/1.1\r\n" +
                "Host: " + lnbitsserver + "\r\n" +
                "User-Agent: ESP32\r\n" +
                "Content-Type: application/json\r\n" +
                "Connection: close\r\n\r\n");
   while (client.connected()) {
   String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
    if (line == "\r") {
      break;
    }
  }
  String line = client.readString();
  Serial.println(line);
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, line);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  bool charPaid = doc["paid"];
  paid = charPaid;
}
