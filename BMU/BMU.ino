/*
created 10/3/2013
 modified 8/13/2014
 by Nima Ghods and Greg Mills
 
 BMU code made to communicate with the BME's 
 Circuit: BMU Shield v4_0
          BME V5_6 attached to arduino due
 */

//included libraries:
  #include <SPI.h>           //use Serial Peripheral Interface (SPI) to communicate to BMEs, included in arduino environment
  //#include <SD.h>
  #include <Ethernet.h>      //use Ethernet to communicate to BMC, included in arduino environment
  #include "BMU.h"           //all BMU variables
  
  
  void setup() { 
    controlTime=200000;  // loop time in uSec  .2 s loops ==> 5Hz
    dt=controlTime/1000000.0;  // control time in sec
    pinInital();    // configure arduino due pins
  }
  
  void loop() 
  {
    timeStamp=micros();                // microseconds since board initialized, overflow/rollover after ~11.9 hours (2^32-1 uS)
                                       // returned in 1 microsecond resolution
    
    pressure=avgADC(presIn1Pin,5)*presConst-presOffset;          //get pressure
    presRate= biquadFilter(biPresrate, pressure);                // filtered pressure rate
    Serial.print(pressure);
    Serial.print(", ");
    Serial.println(presRate);
    
   //if(uartPrint) Serial.println(timeElapsed(timeStamp));
    timeCheck();                //tries to keep loop time roughly constant
  }
  
