#include <SoftwareSerial.h>
#include <Wire.h>
#include "Adafruit_AM2320.h"
#include "Adafruit_CCS811.h"


// I2C OLED
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#define I2C_ADDRESS 0x3C

SSD1306AsciiWire oled;

// CO2 sensor:
SoftwareSerial mySerial(9, 8); // RX,TX
byte cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
unsigned char response[9];

// Humidity + Temperature:
Adafruit_AM2320 am2320 = Adafruit_AM2320();

// TVOC + Humidity + Temperature
Adafruit_CCS811 ccs;

void setup() {
  // Serial
  Serial.begin(9600);
  delay(100);
  ccs.begin();
  delay(100);
  Serial1.begin(9600);
  delay(100);
  mySerial.begin(9600);
  delay(100);
  am2320.begin();
  delay(100);
  pinMode(13, OUTPUT);

  // OLED
  Wire.begin();
  oled.begin(&Adafruit128x32, I2C_ADDRESS);
  oled.set400kHz();
  oled.setFont(Callibri15);
  oled.clear();
  oled.println(" Warming up sensors");
  oled.println(" Please, wait 60s...");
  digitalWrite(13, HIGH);
  delay(100);
  digitalWrite(13, LOW);
  delay(100);
  digitalWrite(13, HIGH);
  delay(100);
  digitalWrite(13, LOW);
  delay(100);
  digitalWrite(13, HIGH);
  delay(100);
  digitalWrite(13, LOW);
  delay(100);
  digitalWrite(13, HIGH);
  delay(100);
  delay(60000);
}

void loop()
{
  mySerial.write(cmd, 9);
  memset(response, 0, 9);
  mySerial.readBytes(response, 9);
  int i;
  byte crc = 0;
  for (i = 1; i < 8; i++) crc += response[i];
  crc = 255 - crc;
  crc++;

  oled.clear();
  delay(100);
  if ( !(response[0] == 0xFF && response[1] == 0x86 && response[8] == crc) ) {
    Serial.println("CRC error: " + String(crc) + " / " + String(response[8]));
    oled.println(" Initialization failed");
    oled.println(" still trying to init...");
    digitalWrite(13, LOW); 
    delay(100);            
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    delay(100);
    digitalWrite(13, HIGH);
    while (mySerial.available()) {
      mySerial.read();
    }
  }
  else {
    unsigned int responseHigh = (unsigned int) response[2];
    unsigned int responseLow = (unsigned int) response[3];
    unsigned int ppm = (256 * responseHigh) + responseLow;

  if(ccs.available()){
    if(!ccs.readData()){
      Serial.println(" CO2(TVOC): ");
      Serial.print(ccs.geteCO2());
      Serial.print(" ppm");

      oled.print(" CO2(TVOC): ");
      oled.print(ccs.geteCO2());
      oled.print(" ppm");

      delay(5000);
      oled.clear();

      Serial.println(" TVOC: ");
      Serial.print(ccs.getTVOC());
      Serial.print(" ppd\n");

      oled.print(" TVOC: ");
      oled.print(ccs.getTVOC());
      oled.print(" ppd");   
      delay(5000);
      oled.clear();
    }
    else{
      Serial.println("TVOC sensor isn ready!");
      while(1);
    }
  }
  delay(500);


    oled.print(" Temperature:   ");
    oled.print(am2320.readTemperature());
    delay(5000);
    oled.clear();

    oled.print(" Humidity:   ");
    oled.print(am2320.readHumidity());
    delay(5000);
    oled.clear();

    Serial.print(ppm);
    String stringppm = String(ppm);   
    delay(100);
    Serial1.write(" CO2: ");
    Serial1.print(stringppm);
    Serial1.write(" ppm\n");
    delay(100);
    Serial.print("");
    
    if (ppm <= 400 || ppm > 4900) {
      oled.println(" CO2: incorrect data");
    } else {
      oled.println(" CO2: " + String(ppm) + " ppm");
      if (ppm < 450) {
        oled.println(" Great");
        digitalWrite(13, HIGH);
      }
      else if (ppm < 600) {
        oled.println(" Good");
        digitalWrite(13, HIGH);
      }
      else if (ppm < 1000) {
        oled.println(" Acceptable");
        digitalWrite(13, HIGH); 
      }
      else if (ppm < 2500) {
        oled.println(" Bad");
      }
      else {
        oled.println(" Health risk");
        digitalWrite(13, LOW);
        delay(100);
        digitalWrite(13, HIGH);
        delay(100);
        digitalWrite(13, LOW);
        delay(100);
        digitalWrite(13, HIGH);
        delay(100);
        digitalWrite(13, LOW);
        delay(100);
        digitalWrite(13, HIGH);
        delay(100);
      }
    }
  }
  delay(10000);
  oled.clear();
}
