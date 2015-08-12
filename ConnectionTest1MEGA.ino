/****************************************************************
Based on:

ConnectionTest.ino
CC3000 Connection Test
Shawn Hymel @ SparkFun Electronics
January 30, 2014
https://github.com/sparkfun/SFE_CC3000_Library

by Uri ZACKHEM.

Distributed as-is; no warranty is given.
****************************************************************/
#include <SPI.h>
#include <SFE_CC3000.h>
#include <SFE_CC3000_Client.h>
// Pins
#define CC3000_INT       2  // Needs to be an interrupt pin (D2/D3)
#define CC3000_EN        7  // Can be any digital pin
#define CC3000_CS       22  // Preferred is pin 10 on Uno

// IP address assignment method
#define USE_DHCP        1   // 0 = static IP, 1 = DHCP

// Constants
char ap_ssid[]     = "NETWORK SSDID (NAME)";                  // SSID of network
char ap_password[] = "NETWORK PASSWORD";          // Password of network
unsigned int ap_security = WLAN_SEC_WPA2; // Security of network
unsigned int timeout = 30000;             // Milliseconds
// Global Variables
SFE_CC3000 wifi = SFE_CC3000(CC3000_INT, CC3000_EN, CC3000_CS);
// Weather underground
const char wuserver[]  = "weatherstation.wunderground.com"; //"rtupdate.wunderground.com";
const char wuwebpage[] = "GET /weatherstation/updateweatherstation.php?";
const char mywsname[]  = "WEATHER STATION ID";
const char mywspass[]  = "WEATHER STATION PASSWORD";

#define BUFFSIZE 150
char longBuff[BUFFSIZE];

void setup() 
{
  pinMode(13, OUTPUT);
  // Config the serial.
  Serial.begin(9600);
  Serial1.begin(9600);
  if(Serial)
    Serial.println(F("**** Weather Station! ****\nSerial - init'd!"));
  delay(500);
  if ( wifi.init() ) {
    if(Serial)
      Serial.println(F("CC3000 initialization complete"));
  } else {
    if(Serial)
      Serial.println(F("Something went wrong during CC3000 init!"));
    delay(500);
    SelfCrash();
  }  
  digitalWrite(13, HIGH); 
  delay(500);
  if(!Connect()) {
     if(Serial){
        Serial.print(F("Error: Could not connect to "));
        Serial.println(ap_ssid);
    }
    delay(500);
    SelfCrash();
  }
  if( !ConnectionInfoFunc()) {
    if(Serial)
      Serial.println(F("Error: Could not obtain connection details"));
    delay(500);
    SelfCrash();
  } else {
    if(Serial)
      Serial.println(F("Connected!"));
  }
  if(Serial)
    Serial.println(F("Setup finished"));
  delay(500);
  digitalWrite(13, LOW); 
}

bool Connect()
{
  const unsigned int ap_security = WLAN_SEC_WPA2;  // Security of network
  const unsigned int timeout     = 30000;          // Milliseconds
  return wifi.connect((char*)ap_ssid , ap_security , (char*)ap_password , timeout);
}

void Disconnect()
{
  wifi.disconnect();
}

bool ConnectionInfoFunc()
{
  ConnectionInfo connection_info;
  return wifi.getConnectionInfo(connection_info);
}

void  SelfCrash()
{
  void (*functionPointer)() = 0;
  Disconnect();
  functionPointer();
}
// Message length: 23. Format: 'H=47.0|T= 26.6|P= 999.1'
void loop() {
  String str("");
  // Empty the buffer
  if(Serial1.available()){
    delay(100);
    while(Serial1.available()){
      Serial1.readString();
    }
  }
  // Wait for incoming string
  while(!Serial1.available()){
    delay(100);
  }
  delay(100);
  // Available. Wait for all to arrive.
  while(Serial1.available()){
      str += Serial1.readString();
  }
  int   parseString = 0;
  if (str.length() == 23){
    Serial.println("___________");
    Serial.println(str);
    Serial.println("-----------");
    parseString = 1;
  }else{
    Serial.println("*** ILL FORMATTED STRING ***");
    delay(200);
  }
  if(parseString){    
    if( !ConnectionInfoFunc()) {
      if(Serial)
        Serial.println(F("Error: Could not obtain connection details"));
      delay(500);
      SelfCrash();
    } 
    //'01234567890123456789012
    // Message length: 23. Format: 'H=47.0|T= 26.6|P= 999.1'
    String hu = str.substring(2,6);
    String te = str.substring(10,14);
    String pr = str.substring(17,23);
    Serial.println(hu);
    Serial.println(te);
    Serial.println(pr);
    // humidity -> integer
    // temp-> fahrenheit -> int
    // pressure -> mmHg -> float XX.X
    int   send_hu = int(hu.toFloat() + 0.5);
    float fahr = te.toFloat() * 1.8 + 32.0;
    float send_te = fahr;
    float send_pr = 0.0295299830714 * pr.toFloat();
    sendData2WU(send_hu , send_te , send_pr);
    delay(2000);
  }
}

void  sendData2WU(int   send_hu , float  send_te , float send_pr)
{
  SFE_CC3000_Client client = SFE_CC3000_Client(wifi);
  char pbuff[8];
  if (client.connect(wuserver , 80)) { 
    if(Serial)
      Serial.println(F("Connectd to WU"));
      Serial.print(F("Server: "));
      Serial.println(wuserver);
  }else{
    if(Serial){
      Serial.println(F("Cannot connect to WU. "));
      delay(500);
      SelfCrash();
    }
  } 
  float humidity = send_hu;
  // fahrenheit to celsius for the dew point calculation.
  float tempf    = (send_te - 32.0) / 1.8;
  // Calculation of the dew point by 
  // http://andrew.rsmas.miami.edu/bmcnoldy/Humidity.html
  // Curtsey Prof. Brian McNoldy.
  float dewptf = 243.04 * (log(humidity * 0.01) + ( ( 17.625 * tempf) / ( 243.04 + tempf))) / (17.625 - log(humidity * 0.01) - ((17.625 * tempf)/(243.04 + tempf)));
  if(Serial){
    Serial.print("T= ");
    Serial.print(tempf);
    Serial.print(" Td= ");
    Serial.print(dewptf);
    Serial.println("  deg. C");
  }
  // dewpoint from celsius to fahrenheit
  dewptf = dewptf * 1.8 + 32.0;
  if(Serial){
    Serial.print("T= ");
    Serial.print(send_te);
    Serial.print(" Td= ");
    Serial.print(dewptf);
    Serial.println("  deg. F");
  }
  if(Serial){
    Serial.print(F("Sending to "));
    Serial.println(wuserver);
  }
  String st;
  // only one iteration. This loop can be removed altogether.
  for (int i = 0 ; i < 1 ; i++){
    if(!client.connected())
      break;
    digitalWrite(13, HIGH);
    if(Serial)
      Serial.println(i);
    st = wuwebpage;
    st += String(F("ID="));
    st += mywsname;
    st += String(F("&PASSWORD="));
    st += mywspass;
    st += String(F("&dateutc=now"));
    st.toCharArray(longBuff , BUFFSIZE);
    client.print(longBuff);
    st = String(F("&tempf="));
    if(send_te >= 100.0){
      dtostrf(send_te , 5 , 1 , pbuff);
    }else{
      dtostrf(send_te , 4 , 1 , pbuff);
    }
    st += String(pbuff);
    st += String("&dewptf=");
    if(dewptf >= 100.0){
      dtostrf(dewptf , 5 , 1 , pbuff);
    }else{
      dtostrf(dewptf , 4 , 1 , pbuff);
    }
    st += String(pbuff);
    /*
    I commented out sending the pressure because my press. sensor failed...
    Uncomment if yours is fine.
    
    st += String(F("&baromin="));
    dtostrf(send_pr , 4 , 1 , pbuff);
    st += String(pbuff);
    */
    st += String(F("&humidity="));
    st += String(send_hu);
    st += String(F("&action=updateraw"));
    st.toCharArray(longBuff , BUFFSIZE);
    client.print(longBuff);
    client.println();
    delay(1000);
    digitalWrite(13, LOW);
    delay(1000);
  }
  if(Serial)
    Serial.println(F("Finished"));
  if(client.connected())
    client.close();
  if(Serial){
     Serial.println(F("Disconnected from WU"));  
     Serial.println();
  }
  digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(3000);              // wait for a second
  digitalWrite(13, LOW); 
}

