//An implementation of weather server with LCD display
//Same as 4 but with combined http send instructions
//Works with DHT Source on helper Arduino to serve DHT over serial
#include <EEPROM.h>

#include <SPI.h>
#include <WiFi101.h>
#include <DHT_U.h>

//#include "DHT.h"
//#define DHTPIN 14
//#define DHTTYPE DHT11   // DHT 11
//#define WLAN_SSID       "ATT9adS9gs"  
//#define WLAN_PASS       "2ihf9768wzk8"


//Display includes
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>

typedef enum disp{HUM,TEMP, HEAT, LIGHT} Disp;
byte curr_disp = HUM;
char *tf[2]={"false","true"};

//float tempC, tempF, humidity, heatiC, heatiF;
//int lightVal, lightLevel, lightPin=A0;
long timeMS;

//LCD Initializers
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

struct weather{
  float humidity;
  float tempC;
  float tempF;
  float heatiC;
  float heatiF;
  int lightLevel;
} w1;

  
//Server Variables
int status;
int keyIndex = 0;
boolean wifi=false;
WiFiClient client; 
WiFiServer server(80);
//DHT dht(DHTPIN, DHTTYPE);
int matchedWifi;
#define KNOWN_NETWORKS 3

int8_t matched_wifi, j;
struct wifi_data{
  char ssid[32];
  char pass[31];
  uint8_t sec;
}curr_wifi;


void setup(){
  int tries = 0;
  matched_wifi = -1;
  Serial.begin(115200);
  Serial1.begin(115200);
  lcd.begin(16,2);
  lcd.print(F("Initializing..."));
  Serial.println(F("Running WeatherCmb4a"));
    // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println(F("WiFi shield not present"));
    // don't continue:
    while (true);
  }
  listNetworks();
  // attempt to connect to WiFi network:
  if(matchedWifi != -1){
    while ((status != WL_CONNECTED)&&(tries<5)) {
      Serial.print(F("Attempting to connect to SSID: "));
      
      Serial.println(curr_wifi.ssid);
      // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
      if(curr_wifi.sec >= 1){
        status = WiFi.begin(curr_wifi.ssid, curr_wifi.pass);
      }
      else{
        status = WiFi.begin(curr_wifi.ssid);
      }
  
      // wait 5 seconds for connection:
      tries++;
      delay(5000);
    }
  }
  if(tries<5){
    wifi=true;
    lcd.setCursor(0,1);
    lcd.print(F("Connected."));
    delay(1000);
  }
  if(wifi){
    server.begin();
    // you're connected now, so print out the status:
    printWiFiStatus();
  }else{
    lcd.setCursor(0,1);
    lcd.print(F("No WiFi!"));
    delay(1000);
  }
}

void loop(){
  //Serial.println(F("In weather's runLoop"));
  //timeMS = millis();
  readWeather();
  if(wifi)
    servePage();
  displaySomething();
  //delay(2000);
}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print(F("IP Address: "));
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print(F("signal strength (RSSI):"));
  Serial.print(rssi);
  Serial.println(F(" dBm"));
}


void readWeather(){
  static long lastTimeMS = 0;
  static boolean ran = false;
  int bytesRead;
  //Serial.println(F("At top of weather."));
  if((millis() >= lastTimeMS + 15000) || (ran == false)){
    Serial.print(F("Reading weather: "));
    Serial1.write('0');
    
    while(Serial1.available() == 0){
      //Serial.println("No data");
      delay(100);
    }
    //Serial.println("Reading data.");
    bytesRead=Serial1.readBytes((byte*)&w1, sizeof(w1));
    Serial.print(bytesRead);
    Serial.println(F(" bytes read."));
    ran=true;
    lastTimeMS = millis();
  }
}

void servePage(){
  const char deg = 186;
  String req = String("");
  static boolean ran = false;
  boolean favicon = false;
  static long lastTimeMS = 0;
  if((millis() >= lastTimeMS + 200) || (ran == false)){
    //Serial.print(F(". "));
    client = server.available();
    if(client){
      Serial.println(F("Client present."));
      boolean currentLineIsBlank = true;
      String currentLine = "";
      while (client.connected()) {
        if (client.available()) {
          char c = client.read();
          Serial.write(c);
          req += c;
          if (c == '\n') {
            Serial.print(F("server received:"));
            Serial.println(req);
            Serial.println(req.length());
            //Serial.println(req.substring(4,11));
            if(req.substring(4,12) == String(F("/favicon"))){
              favicon = true;
            }
            Serial.print(F("Favicon request: ")); Serial.println(tf[favicon]);
            if(currentLine.length() == 0 && favicon == false ) {
              // send a standard http response header
              Serial.println(F("Sending webpage information."));
              client.println(F("HTTP/1.1 200 OK"));
              client.println(F("Content-Type: text/html"));
              client.println(F("Connection: close"));  // the connection will be closed after completion of the response
              //client.println("Refresh: 5");  // refresh the page automatically every 5 sec
              client.println();
              client.println(F("<!DOCTYPE HTML>"));
              //client.print(F("<html>"));
              client.println(F("<html><head><title>Weather and Brightness</title></head>"));
              client.print(F("<body><h3>I'm feeling much better now.<br>"));
              /*client.print(F("Humidity: "));
              client.print(w1.humidity);
              client.print(F("%<br>Temperature: "));
              client.print(w1.tempC);
              client.print(deg);
              client.print(F("C  "));
              client.print(w1.tempF);
              client.print(deg);
              client.println(F("F<br>"));*/
              /*All of this ^ on one client.print*/
              String output = String(F("Humidity: ")) + String(w1.humidity) + String(F("%<br>Temperature: ")) + String(w1.tempC) + String(deg) + String(F("C  ")) + String(w1.tempF) + String(deg) + String(F("F<br>"));
              client.println(output);
              /*client.print(F("Heat Index: "));
              client.print(w1.heatiC);
              client.print(deg);
              client.print(F("C  "));
              client.print(w1.heatiF);
              client.print(deg);
              client.println(F("F<br>"));*/
              output = String(F("Heat Index: ")) + String(w1.heatiC) + String(deg) + String(F("C  ")) + String(w1.heatiF) + String(deg) + String(F("F<br>"));
              client.println(output);
              /*client.print(F("Light level: ")); //Try to print on same line
              client.print(w1.lightLevel);
              client.println(F("</h3></body></html>"));
              client.println("");*/
              output = String(F("Light level: ")) + String(w1.lightLevel);
              client.println(output);
              client.println(F("<br>You just got served<br>A web page.</h3></body></html>\r\n"));
              // break out of the while loop:
              break;
            }
            else {
              currentLine = "";
              //Serial.println(F("Current line >0 characters."));
            }
          }
          else if (c != '\r') {    // if you got anything else but a carriage return character,
            //Serial.print(F("server recieved "));
            //Serial.println(c);
            currentLine += c;      // add it to the end of the currentLine
          }
        }
        else{
          break; // Out of 'while client connected' if no data available - I hope
        }
      }
      client.stop();
      Serial.println(F("Client disconntected."));
    }
    lastTimeMS = millis();
    ran = true;
  }
}

void displaySomething(){
  
  const char deg = 0xDF;
  char str_level[6]; //Needs to be 6 bytes for some reason
  static boolean ran = false;
  static long lastTimeMS = 0;
  if( (millis() >= lastTimeMS + 3500) || (ran == false) ){
    //Serial.println(F("Running display"));
    switch(curr_disp){
      case HUM:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print(F("Humidity: "));
        lcd.setCursor(0,1);
        lcd.print(w1.humidity);
        lcd.print("%");
        curr_disp=TEMP;
        break;
      case TEMP:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print(F("Temperature: "));
        lcd.setCursor(0,1);
        lcd.print(w1.tempC);
        lcd.print(deg);
        lcd.print("C ");
        lcd.print(w1.tempF);
        lcd.print(deg);
        lcd.print("F");
        curr_disp=HEAT;
        break;
      case HEAT:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print(F("Heat index: "));
        lcd.setCursor(0,1);
        lcd.print(w1.heatiC);
        lcd.print(deg);
        lcd.print("C ");
        lcd.print(w1.heatiF);
        lcd.print(deg);
        lcd.print("F");
        curr_disp=LIGHT;
        break;
      case LIGHT:
        itoa(w1.lightLevel,str_level,6);
        lcd.clear();
        lcd.print(F("Light level: "));
        lcd.setCursor(0,1);
        lcd.print(str_level);
        curr_disp=HUM;
        break;
    }
    lastTimeMS = millis();
    ran = true;
  }
}

void printMacAddress() {
  // the MAC address of your WiFi shield
  byte mac[6];

  // print your MAC address:
  WiFi.macAddress(mac);
  Serial.print(F("MAC: "));
  printMacAddress(mac);
}

void listNetworks() {
  // scan for nearby networks:
  Serial.println(F("** Scanning Networks **"));
  int cmp_result;
  int numSsid = WiFi.scanNetworks();
  if (numSsid == -1)
  {
    Serial.println(F("Couldn't get a wifi connection"));
    while (true);
  }

  // print the list of networks seen:
  Serial.print(F("Number of available networks: "));
  Serial.println(numSsid);

  // print the network number and name for each network found:
  for (int thisNet = 0; thisNet < numSsid; thisNet++) {
    Serial.print(thisNet);
    Serial.print(F(") "));
    if(strcmp(WiFi.SSID(thisNet),"")==0){
      Serial.print(F("Name Hidden"));
    }
    else{
      Serial.print(WiFi.SSID(thisNet));
      //Insert EEPROM matching code
      //Copy data from EEPROM
      EEPROM.get(j*sizeof(wifi_data),curr_wifi);
      cmp_result=strcmp(WiFi.SSID(), curr_wifi.ssid);

      if(cmp_result==0){
        Serial.print(F("Matched: "));
        Serial.println(curr_wifi.ssid);
        matched_wifi=thisNet;
        break;
      }
    }
    
    Serial.print(F("\tSignal: "));
    Serial.print(WiFi.RSSI(thisNet));
    Serial.print(F(" dBm"));
    Serial.print(F("\tEncryption: "));
    printEncryptionType(WiFi.encryptionType(thisNet));
    Serial.flush();
  }
}

void printEncryptionType(int thisType) {
  // read the encryption type and print out the name:
  switch (thisType) {
    case ENC_TYPE_WEP:
      Serial.println("WEP");
      break;
    case ENC_TYPE_TKIP:
      Serial.println("WPA");
      break;
    case ENC_TYPE_CCMP:
      Serial.println("WPA2");
      break;
    case ENC_TYPE_NONE:
      Serial.println("None");
      break;
    case ENC_TYPE_AUTO:
      Serial.println("Auto");
      break;
  }
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}
