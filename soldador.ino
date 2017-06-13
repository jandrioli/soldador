/*
Note: use the latest Arduino software and install the 3 libraries.
Arduino spot welder controller
http://www.avdweb.nl/arduino/hardware-interfacing/spot-welder-controller.html
Copyright (C) 2012  Albert van Dalen
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License at http://www.gnu.org/licenses .
Version 1-02-2015
Program with FTDI programmer, Extra > Board > Arduino Uno
              <   20ms   >< >sinusMax_us          
               _____       _____       _____    
zeroCross   __|     |_____|     |_____|     |__                
                              _____________          
weld       __________________|             |____       


*******************//********************
4 Digit 7seg display pinout:
Pins labelled as a-g are the LED forming a digit.
Pins labelled D1-4 are the cathode of each digit on the display.
 12  11 10 9 8 7
 _   _   _   _
|_| |_| |_| |_|
|_|.|_|.|_|.|_|.
   1 2 3 4 5 6

1 e (pin8)
2 d (pina1)
3 dp (pina4)
4 c a3
5 g a5
6 d4 a4
7 b (pin7)
8 d3 (pin6)
9 d2 (pin5)
10 f 4
11 a (pin3)
12 d1 (pin2)
                                            
*/
#include <Arduino.h>
#include <SevSeg.h>
//#include <SevenSeg.h>
const int sinusMax_us = 4583; // time required for a sine-wave at 60Hz to half-cycle once

const byte weldPin = 9;
const byte weldPin_Temporary = 12; //relay connected here
const byte weldButtonPin = 10;
const byte weldBuzzerPin = A5;
const byte ledPin = 13;
const byte zeroCrossPin = 11;
const int preWeld_ms = 250;
const int weldPause_ms = 500;
const int step_ms = 250;
      int weldTime_ms = 0;
     bool continuously = true;
     bool onoff = false;
SevSeg sevseg; //Instantiate a seven segment object

/*SevenSeg disp (3, 7, A3, A1, 8, 4, A5) ;
const int numOfDigits =4;
int digit_Pins [ numOfDigits ]={2,5,6,A2};*/


 
void setup() {
  
  Serial.begin(115200);
  Serial.println("Starting...");
  
  byte digitPins[] = {2,5,6,A2};
  byte segmentPins[] = {3, 7, A3, A1, 8, 4, A5, A4};
  sevseg.begin(COMMON_CATHODE, 4, digitPins, segmentPins);

  /*disp.setDigitPins ( numOfDigits , digit_Pins );
  disp.setDPPin(A4);
  disp.setCommonCathode();*/
  
  pinMode(weldButtonPin, INPUT_PULLUP);  
  pinMode(weldPin, OUTPUT); // welding doesnt work yet because i dont have the right WIMA capacitor for the rc-filter that activates the weld
  pinMode(weldPin_Temporary, OUTPUT); // a temporary workaround to the lack of the correct rc-filter (which activates the weld) is this pin which is a relay board
  pinMode(ledPin, OUTPUT);
  pinMode(weldBuzzerPin, OUTPUT);
  pinMode(zeroCrossPin, INPUT);
  
  digitalWrite(weldPin_Temporary, HIGH); // immediately turn off the relay
  //disp.setDigitDelay(250);
  //disp.write("Hello");
  
  for (int ii = 0; ii<= 20; ii++) {  
    digitalWrite(ledPin, onoff); 
    onoff = !onoff;
    delay(50);
  }
  digitalWrite(ledPin, 1); // power on indication
}

void loop() {

  // potentiometer on pin A0 will tell me the duration of the pulse of weld
  // read the input on analog pin 0:
  int sensorValue = analogRead(A0);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float duration = map(sensorValue, 0, 1023, 0, 16) * step_ms;
  if (weldTime_ms != duration)
  {
    weldTime_ms = duration;
    // print out the value you read:
    Serial.print("Weld duration ");
    Serial.println(weldTime_ms);
    sevseg.setNumber(weldTime_ms,0); 
    //disp.write(weldTime_ms);
    delay(100);
  }
  
  
  if(weldTime_ms < step_ms /*BCDswitch()==0*/ && !digitalRead(weldButtonPin)) 
  {
    //disp.write("HOLD");
    sevseg.setNumber(8888,0);
    weld(true); // continuous welding
  }
  else if (!digitalRead(weldButtonPin)) //will be false when pressed coz PULLUP
  {
    Serial.println("soldering...");
    //disp.write("WELD");
    sevseg.setNumber(8888,0);
    weldCyclus(duration/*BCDswitch() * step_ms*/); // use timer
  } 
  else
  {
    sevseg.refreshDisplay();
  }
  
}

void weldCyclus(int weldTime_ms)
{ 
  for (int k = 0; k < 4; k++) {
    analogWrite(weldBuzzerPin, 750);
    delay(1000);
    analogWrite(weldBuzzerPin, 0);
    delay(1000);
  }
  sinusMax();
  pulseWeld(preWeld_ms);
  delay(weldPause_ms);
  sinusMax();
  pulseWeld(weldTime_ms);
}

/*int BCDswitch(int nothingness)
{
  int bcd;
  bitWrite(bcd, 0, !digitalRead(BCDswitch0));
  bitWrite(bcd, 1, !digitalRead(BCDswitch1));
  bitWrite(bcd, 2, !digitalRead(BCDswitch2));
  bitWrite(bcd, 3, !digitalRead(BCDswitch3));
  return bcd;
}
*/
// Performs an intervalled operation of welding 
void pulseWeld(int ms)
{
  weld(1);
  delay(ms);
  weld(0);
  //Serial << ms << endl; 
}

// Connect welder to Mains (turn it on [actually welds])
void weld(bool b)
{
  digitalWrite(weldPin, b);
  digitalWrite(weldPin_Temporary, !b);
  digitalWrite(ledPin, !b);
}

// Waits for a zerocrossing of AC line
void sinusMax()
{
  /*while(digitalRead(zeroCrossPin));
  while(!digitalRead(zeroCrossPin));*/
  delayMicroseconds(sinusMax_us); // to prevent inrush current, turn-on at the sinus max
}
