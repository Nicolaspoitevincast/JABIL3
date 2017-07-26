/*********************************************************************
This is an example for our nRF8001 Bluetooth Low Energy Breakout

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/1697

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Kevin Townsend/KTOWN  for Adafruit Industries.
MIT license, check LICENSE for more information
All text above, and the splash screen below must be included in any redistribution
*********************************************************************/

// This version uses call-backs on the event and RX so there's no data handling in the main loop!
#include <OneWire.h> 
#include <DallasTemperature.h>
#include <SPI.h>
#include "Adafruit_BLE_UART.h"

#define SERIES_RESISTOR     560    // Value of the series resistor in ohms.    
#define SENSOR_PIN          5      // Analog pin which is connected to the sensor. 
#define ADAFRUITBLE_REQ 10
#define ADAFRUITBLE_RDY 2
#define ADAFRUITBLE_RST 9
#define ONE_WIRE_BUS 3

// The following are calibration values you can fill in to compute the volume of measured liquid.
// To find these values first start with no liquid present and record the resistance as the
// ZERO_VOLUME_RESISTANCE value.  Next fill the container with a known volume of liquid and record
// the sensor resistance (in ohms) as the CALIBRATION_RESISTANCE value, and the volume (which you've
// measured ahead of time) as CALIBRATION_VOLUME.
#define ZERO_VOLUME_RESISTANCE    1429.17 // 1457.18    // Resistance value (in ohms) when no liquid is present. 1457.18
#define CALIBRATION_RESISTANCE    417.61  //391.63    // Resistance value (in ohms) when liquid is at max line.
#define CALIBRATION_VOLUME        700.00  //800.00    // Volume (in any units) when liquid is at max line.

// Setup a oneWire instance to communicate with any OneWire devices  
// (not just Maxim/Dallas temperature ICs) 
OneWire oneWire(ONE_WIRE_BUS); 
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

Adafruit_BLE_UART uart = Adafruit_BLE_UART(ADAFRUITBLE_REQ, ADAFRUITBLE_RDY, ADAFRUITBLE_RST);

String temp,vol;
/**************************************************************************/
/*!
    This function is called whenever select ACI events happen
*/
/**************************************************************************/
void aciCallback(aci_evt_opcode_t event)
{
  switch(event)
  {
    case ACI_EVT_DEVICE_STARTED:
      Serial.println(F("Advertising started"));
      break;
    case ACI_EVT_CONNECTED:
      Serial.println(F("Connected!"));
      break;
    case ACI_EVT_DISCONNECTED:
      Serial.println(F("Disconnected or advertising timed out"));
      break;
    default:
      break;
  }
}

/**************************************************************************/
/*!
    This function is called whenever data arrives on the RX channel
*/
/**************************************************************************/
void rxCallback(uint8_t *buffer, uint8_t len)
{
  Serial.print(F("Received "));
  Serial.print(len);
  Serial.print(F(" bytes: "));
  for(int i=0; i<len; i++)
   Serial.print((char)buffer[i]); 

  Serial.print(F(" ["));

  for(int i=0; i<len; i++)
  {
    Serial.print(" 0x"); Serial.print((char)buffer[i], HEX); 
  }
  Serial.println(F(" ]"));

  /* Echo the same data back! */
  uart.write(buffer, len);
}

float readResistance(int pin, int seriesResistance) {
// Get ADC value.
float resistance = analogRead(pin);
// Convert ADC reading to resistance.
resistance = (1023.0 / resistance) - 1.0;
resistance = seriesResistance / resistance;
return resistance;
}

float resistanceToVolume(float resistance, float zeroResistance, float calResistance, float calVolume) {
if (resistance > zeroResistance || (zeroResistance - calResistance) == 0.0) {
  // Stop if the value is above the zero threshold, or no max resistance is set (would be divide by zero).
  return 0.0;
}
// Compute scale factor by mapping resistance to 0...1.0+ range relative to maxResistance value.
float scale = (zeroResistance - resistance) / (zeroResistance - calResistance);
// Scale maxVolume based on computed scale factor.
return calVolume * scale;
}

/**************************************************************************/
/*!
    Configure the Arduino and start advertising with the radio
*/
/**************************************************************************/
void setup(void)
{ 
  Serial.begin(9600);
  while(!Serial); // Leonardo/Micro should wait for serial init
  Serial.println(F("JABIL TEAM 3 DEMO CODE"));

  uart.setRXcallback(rxCallback);
  uart.setACIcallback(aciCallback);
  uart.setDeviceName("JABIL3"); /* 7 characters max! */
  sensors.begin(); 
  uart.begin();
}

/**************************************************************************/
/*!
    Constantly checks for new events on the nRF8001
*/
/**************************************************************************/
void loop()
{
  uart.pollACI();

  float resistance = readResistance(SENSOR_PIN, SERIES_RESISTOR);
  float volume = resistanceToVolume(resistance, ZERO_VOLUME_RESISTANCE, CALIBRATION_RESISTANCE, CALIBRATION_VOLUME);

  // call sensors.requestTemperatures() to issue a global temperature 
 // request to all devices on the bus 
/********************************************************************/
 Serial.print(" Requesting Data..."); 
 sensors.requestTemperatures(); // Send the command to get temperature readings 
 Serial.println("DONE"); 
/********************************************************************/
Serial.print("Resistance: ");
Serial.println(resistance,2); // for calibration
 Serial.print("Data is: "); 
// Serial.print(sensors.getTempFByIndex(0)); // Why "byIndex"?  
   // You can have more than one DS18B20 on the same bus.  
   // 0 refers to the first IC on the wire \
//temp = (sensors.getTempFByIndex(0));
temp=(sensors.getTempFByIndex(0));
Serial.print(temp+":"+volume);
uart.print(temp+":"+volume);
   
 delay(1000);  //Equal to min sampling rate
}
