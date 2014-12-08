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
  
  BiquadType testFilter;
  int testNum=1;  //1 for pressure test 2 fortemperature test
  
  void setup() { 
    
    pinInital();    // configure arduino due pins
    delay(2000);
    if(testNum==1){
      controlTime = 200000;  // loop time in uSec  .2 s loops ==> 5Hz
      dt = controlTime/1000000.0;  // control time in sec
      TestFilterInit((BiquadType&) testFilter,1.0,0.0078,0.0156,0.0078,-1.7347,0.7660,0,0); //initialize filter for pressure rate
      Serial.println("pressure, dpressure, filtered dpressure");
    }
    
    if(testNum==2){
      controlTime = 200000;  // loop time in uSec  .2 s loops ==> 5Hz
      dt = controlTime/1000000.0;  // control time in sec
      Serial.println("Cell 1, Cell 2,  Cell 3, Cell Sum, V_ref2, T1, T2, T3, T4, Ti");
    }
    
  }
  
  void loop() 
  {
    timeStamp=micros();                // microseconds since board initialized, overflow/rollover after ~11.9 hours (2^32-1 uS)
                                       // returned in 1 microsecond resolution
    
    if(testNum==1) pressureTest();
    if(testNum==2) tempTest(0,1,false);
    
//    Serial.println(timeElapsed(timeStamp));
   //if(uartPrint) Serial.println(timeElapsed(timeStamp));
    timeCheck();                //tries to keep loop time roughly constant
  }
  /*------------------------------------------------------------------------------
 * void tempTest(void)
 * temperature test turns on a risistor on BME tempoBme+1 and layer tempoLayer+1
 * prints voltages and temperatures of the given BME
 *----------------------------------------------------------------------------*/
  void tempTest(int tempoBme, int tempoLayer, boolean tempFan){
    
    if(tempoBme>=0 && tempoBme>=13 && tempoLayer>=0 && tempoLayer<=2){
      BME[tempoBme].DCC = (1<<(2-tempoLayer));    // balance by enabling the bit flag corresponding to the i-th virtual layer
      CLRCELL(tempoBme);
      ADCV(tempoBme,0);          //  start voltage conversion
      delayMicroseconds(BMEConDelay1);
      RDCVA((BMEdata&) BME[tempoBme]);
      delayMicroseconds(BMEConDelay1);
      // get cell temperatures
      ADAX(tempoBme,0);              //  start temperature conversion
      delayMicroseconds(BMEConDelay1);
      RDAUXA((BMEdata&) BME[tempoBme]);    // get cell temperatures
      RDAUXB((BMEdata&) BME[tempoBme]);  
      delayMicroseconds(BMEConDelay1);
      // get chip temperatures, sum of battery module
      ADSTAT(tempoBme,0);              //  start conversion
      delayMicroseconds(BMEConDelay2);
      RDSTATA((BMEdata&) BME[tempoBme]);  // get chip temperatures, sum of battery module
//      RDSTATB((BMEdata&) BME[tempoBme]);  // get flags
      BME[tempoBme].GPIO=0x0f|((!tempFan)<<4);          // Sets the GPIO to 0 or 1 for the multiplexer
      WRCFG((BMEdata&) BME[tempoBme]);
      int2float((BMEdata&) BME[tempoBme]); // turn in to floats
      
      // print data 
      for(int i=0;i<cellNum;i++){
        Serial.print(BME[tempoBme].fVol[i],4);
        Serial.print(", ");
      }
      Serial.print(BME[tempoBme].fVSum,3);
      Serial.print(", ");
      Serial.print(BME[tempoBme].fVref2,4);
      Serial.print(", ");
      for(int i=0;i<4;i++){
        Serial.print(BME[tempoBme].fTemp[i],1);
        Serial.print(", ");
      }
      Serial.println(BME[tempoBme].fiTemp,1);
      
    }
  }
  
   /*------------------------------------------------------------------------------
 * void pressureTest(void)
 * tests the pressure sensor by printing pressure value and pressure rate
 *----------------------------------------------------------------------------*/
  void pressureTest(){ 
    pressure=avgADC(presIn1Pin,3)*presConst-presOffset;          //get pressure
    presRate = (pressure-presOld)/dt;
    float presRateFil= biquadFilter(testFilter, presRate); // filtered pressure rate
    presOld=pressure;
    Serial.print(pressure,4);
    Serial.print(", ");
    Serial.print(presRate,4); 
    Serial.print(", ");
    Serial.println(presRateFil,4); 
  }
  
  /*------------------------------------------------------------------------------
 * void TestFilterInit(void)
 * initialize filter for pressure rate
 *----------------------------------------------------------------------------*/
  void TestFilterInit(BiquadType& tempoFilter,float g,float b_0,float b_1,float b_2,float a_1,float a_2,float x_1,float x_2){
    
    tempoFilter.gain=g;      //filter gain
    tempoFilter.b0 = b_0;     //input k coefficient
    tempoFilter.b1 = b_1;              //input k-1 coefficient
    tempoFilter.b2 = b_2;                //input k-2 coefficient
    tempoFilter.a1 = a_1;                //output k-1 coefficient
    tempoFilter.a2 = a_2;                //output k-2 coefficient
    tempoFilter.x1 = x_1;  //filter state
    tempoFilter.x2 = x_2;    //filter state
    
  }
  
 
    
