/* 
 Weather Shield Example
 By: Nathan Seidle
 SparkFun Electronics
 Date: November 16th, 2013
 Modified: Uri ZACKHEM, May 2015
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 
 Much of this is based on Mike Grusin's USB Weather Board code: https://www.sparkfun.com/products/10586
 
 This code reads all the various sensors (wind speed, direction, rain gauge, humidty, pressure, light, batt_lvl)
 and reports it over the serial comm port. This can be easily routed to an datalogger (such as OpenLog) or
 a wireless transmitter (such as Electric Imp).
 
 Measurements are reported once a second but windspeed and rain gauge are tied to interrupts that are
 calcualted at each report.
 
 This example code assumes the GPS module is not used.
 
 Some modifications by UZ 23/04/15 
 */
#include <math.h>
#include <Wire.h> //I2C needed for sensors
#include "MPL3115A2.h" //Pressure sensor
#include "HTU21D.h" //Humidity sensor

MPL3115A2 myPressure; //Create an instance of the pressure sensor
HTU21D myHumidity; //Create an instance of the humidity sensor

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//Global Variables
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

float humidity = 0; // [%]
float tempf = 0; // [temperature C]
float pressure = 0; // Pascal
float dewptf; // [dewpoint C] - It's hard to calculate dewpoint locally, do this in the agent

void setup()
{
  //Configure the pressure sensor
  myPressure.begin(); // Get sensor online
  myPressure.setModeBarometer(); // Measure pressure in Pascals from 20 to 110 kPa
  myPressure.setOversampleRate(7); // Set Oversample to the recommended 128
  myPressure.enableEventFlags(); // Enable all three pressure and temp event flags 
  //Configure the humidity sensor
  myHumidity.begin();
  int   Countdown   = 3;// Wait
  while(Countdown >= 0)
  {
     Countdown--;
  }
  calcWeather();
  Serial.begin(9600);
}

void loop()
{
  printWeather();
  delay(20000);
}

//Calculates each of the variables that wunderground is expecting
void calcWeather()
{
  //Calc humidity
  humidity = myHumidity.readHumidity();
  tempf    = myPressure.readTemp();
  pressure = myPressure.readPressure();
  // http://andrew.rsmas.miami.edu/bmcnoldy/Humidity.html
  //dewptf = 243.04 * (log(humidity * 0.01) + ( ( 17.625 * tempf) / ( 243.04 + tempf))) / (17.625 - log(humidity * 0.01) - ((17.625 * tempf)/(243.04 + tempf)));
}

//Prints the various variables directly to the port
//I don't like the way this function is written but Arduino doesn't support floats under sprintf
void printWeather()
{
  char tbuff[10] , pbuff[10] , hbuff[10] , hpcbuff[10];
  calcWeather(); //Go calc all the various sensors
  dtostrf(tempf           , 5 , 1 , tbuff);
  dtostrf(pressure * 0.01 , 6 , 1 , pbuff);
  dtostrf(humidity        , 4 , 1 , hpcbuff);
  String s = "H=";
  s += (char*)hpcbuff;
  s += "|T=";
  s += (char*)tbuff;
  s += "|P=";
  s += (char*)pbuff;
  Serial.print(s);
}



