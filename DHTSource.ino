#include <DHT_U.h>

// Example testing sketch for various DHT humidity/temperature sensors
// Written by ladyada, public domain
// Adapted by Andrew Tester to serve requests over serial

#include "DHT.h"

#define DHTPIN 6     // what digital pin we're connected to

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
struct weather{
  float humidity;
  float tempC;
  float tempF;
  float heatiC;
  float heatiF;
  int lightLevel;
} w1;

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  //Serial.println("DHT11 test!");

  dht.begin();
}

void loop() {
  int lightVal, lightLevel;
  int chomp, lightPin=A0;
  // Wait for a serial byte
  if(Serial.available() > 0){
    chomp = Serial.read();
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    w1.humidity = dht.readHumidity();
    // Read temperature as Celsius (the default)
    w1.tempC = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    w1.tempF = dht.readTemperature(true);
  
    // Check if any reads failed and exit early (to try again).
    if (isnan(w1.humidity) || isnan(w1.tempC) || isnan(w1.tempF)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
  
    // Compute heat index in Fahrenheit (the default)
    w1.heatiF = dht.computeHeatIndex(w1.tempF, w1.humidity);
    // Compute heat index in Celsius (isFahreheit = false)
    w1.heatiC = dht.computeHeatIndex(w1.tempC, w1.humidity, false);
  
    /*Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" *C ");
    Serial.print(f);
    Serial.print(" *F\t");
    Serial.print("Heat index: ");
    Serial.print(hic);
    Serial.print(" *C ");
    Serial.print(hif);
    Serial.println(" *F");*/
    lightVal = analogRead(lightPin);
    lightLevel = map(lightVal,1000,0,1,6);
    //Serial.print("Light level: ");
    w1.lightLevel = lightLevel;
    
    Serial.write((byte*)&w1, sizeof(w1));
    
  }
}
