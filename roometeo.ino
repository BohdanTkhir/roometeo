/************************************************************/
/*     Author:   Bohdan Tkhir                               */
/*     Email:    Tkhir.Bogdan9@gmail.com                    */
/*     Date:     23.01.2020                                 */
/*     Ver.:     0.9 nightly                                */
/*     Codename: Roometeo                                   */            
/*                                                          */
/*     Board:  Arduino Leonardo                             */
/*                                                          */
/*     Sensors:                                             */
/*             CO2           - MH-Z19B                      */
/*             TVOC/CO2      - CJMCU-81 1                   */
/*             Humidity/Temp - AM2320                       */
/*             Bluetooth     - HC-06                        */
/*                                                          */
/*                                                          */
/*   Please, feel free using this sketch                    */
/*   Let me know if it was helpful, I will be glad to hear  */
/*                                                          */
/************************************************************/

#include <SoftwareSerial.h>
#include <Wire.h>
#include "Adafruit_AM2320.h"
#include "Adafruit_CCS811.h"

// I2C OLED
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#define I2C_ADDRESS 0x3C
SSD1306AsciiWire oled;

// CO2 ( MH-Z19B ) :
SoftwareSerial mySerial(9, 8); // RX,TX
byte cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
unsigned char response[9];

// Humidity + Temperature: (AM2320)
Adafruit_AM2320 am2320 = Adafruit_AM2320();

// TVOC + CO2( CJMCU-811 / CCS811 )
Adafruit_CCS811 ccs;

int oledSaveCounter = 0;

/* CONFIGURATION SECTION*/
#define TVOC_LIMIT     1000 // TVOC ppd buzzer signal 
#define OLED_CYCLES    10    // oled cycles before turning off to save lifetime
#define PERFECT_CO2    450  // 400.....450 ppm
#define GOOD_CO2       600  // 450.....600 ppm 
#define ACCEPTABLE_CO2 1000 // 600....1000 ppm 
#define BAD_CO2        2500 // 1000...2500 ppm 

// NOTE: After 2500ppm will be buzzer signal cause it stars of health influence

// NOTE: To wake OLED you need to hold button to time when it can be readed.
                /*( Yes, It can be done by interrupt, but maybe later :) )*/

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
  pinMode(11, INPUT_PULLUP);

  // OLED
  Wire.begin();
  oled.begin(&Adafruit128x32, I2C_ADDRESS);
  oled.set400kHz();
  oled.setFont(Callibri15);
  oled.clear();
  oled.println(" Warming up sensors");
  oled.println(" Please, wait 60s...");
  beep(2, 13);
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
    beep(3, 13);
    while (mySerial.available()) {
      mySerial.read();
    }
  }
  else {
    unsigned int responseHigh = (unsigned int) response[2];
    unsigned int responseLow = (unsigned int) response[3];
    unsigned int ppm = (256 * responseHigh) + responseLow;

    if (ccs.available()) {
      if (!ccs.readData()) {

        Serial.println(" CO2(TVOC): ");
        Serial.print(ccs.geteCO2());
        Serial.print(" ppm");
        Serial1.print(" CO2(TVOC): ");
        Serial1.print(ccs.geteCO2());
        Serial1.print(" ppm\n");

        int show = digitalRead(11);
        if (!show)
        {
          oledSaveCounter = 0;
        }
        if (oledSaveCounter < OLED_CYCLES)
        {
          oled.print(" CO2(TVOC): \n ");
          oled.print(ccs.geteCO2());
          oled.print(" ppm");
          delay(5000);
          oled.clear();
        }

        Serial.println(" TVOC: ");
        Serial.print(ccs.getTVOC());
        Serial.print(" ppd\n");
        Serial1.print(" TVOC: ");
        Serial1.print(ccs.getTVOC());
        Serial1.print(" ppd\n");
        if (ccs.getTVOC() > TVOC_LIMIT )
        {
          Serial1.print("TVOC WARNING!\n");
        }
        show = digitalRead(11);
        if (!show)
        {
          oledSaveCounter = 0;
        }
        if (oledSaveCounter < OLED_CYCLES)
        {
          oled.print(" TVOC: ");
          oled.print(ccs.getTVOC());
          oled.print(" ppd");
          if (ccs.getTVOC() > 1000 )
          {
            oled.print("\n TVOC WARNING!\n");
            beep(5, 13);
          }
          delay(5000);
          oled.clear();
        }

      }
      else {
        Serial.println("TVOC sensor isn ready!");
        while (1);
      }
    }

    int show = digitalRead(11);
    if (!show)
    {
      oledSaveCounter = 0;
    }
    if (oledSaveCounter < OLED_CYCLES)
    {
      oled.print(" Temp:   ");
      oled.print(am2320.readTemperature());
    }
    Serial1.print(" Temp: ");
    Serial1.print(am2320.readTemperature());
    Serial1.print("C\n");
    Serial.print(" Temp: ");
    Serial.print(am2320.readTemperature());
    Serial.print("C\n");
    delay(5000);
    oled.clear();
    show = digitalRead(11);
    if (!show)
    {
      oledSaveCounter = 0;
    }
    if (oledSaveCounter < OLED_CYCLES)
    {
      oled.print(" Humidity:   ");
      oled.print(am2320.readHumidity());
    }
    Serial1.print(" Humidity:   ");
    Serial1.print(am2320.readHumidity());
    Serial1.print("C\n");
    Serial.print(" Humidity:   ");
    Serial.print(am2320.readHumidity());
    Serial.print("C\n");
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

    show = digitalRead(11);
    if (!show)
    {
      oledSaveCounter = 0;
    }
    if (ppm <= 400 || ppm > 4900) {
      if (oledSaveCounter < OLED_CYCLES)
      {
        oled.println(" CO2: incorrect data");
      }
    } else {
      if (oledSaveCounter < OLED_CYCLES)
      {
        oled.println(" CO2: " + String(ppm) + " ppm");
      }
      if (ppm < 450) {
        if (oledSaveCounter < OLED_CYCLES)
        {
          oled.println(" Great");
        }
        digitalWrite(13, HIGH);
      }
      else if (ppm < 600) {
        if (oledSaveCounter < OLED_CYCLES)
        {
          oled.println(" Good");
        }
        digitalWrite(13, HIGH);
      }
      else if (ppm < 1000) {
        if (oledSaveCounter < OLED_CYCLES)
        {
          oled.println(" Acceptable");
        }
        digitalWrite(13, HIGH);
      }
      else if (ppm < 2500) {
        if (oledSaveCounter < OLED_CYCLES)
        {
          oled.println(" Bad");
        }
      }
      else {
        if (oledSaveCounter < OLED_CYCLES)
        {
          oled.println(" Health risk");
        }
        beep(5, 13);
      }
    }
  }
  Serial1.print("------------------------\n");
  delay(10000);
  oled.clear();
  oledSaveCounter += 1;
}


void beep(int times, int pin) // LOW ACTIVE LEVEL BUZZER BEEPER
{
  for (int i = 0; i <= times; i++)
  {
    digitalWrite(pin, LOW);
    delay(50);
    digitalWrite(pin, HIGH);
    delay(50);
  }
}
