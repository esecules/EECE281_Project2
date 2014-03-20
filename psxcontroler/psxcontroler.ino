/*
 * PSX sketch
 *
 * Display joystick and button values
 * uses PSX library written by Kevin Ahrendt
 * http://www.arduino.cc/playground/Main/PSXLibrary
 */

#include <Psx.h>                       // Includes the Psx Library

Psx Psx;                               // Create an instance of the Psx library

const int dataPin  = 2;
const int cmndPin  = 3;
const int attPin   = 5;
const int clockPin = 4;
const int psxDelay = 50;         // determine the clock delay in microseconds
const int handshake = 13;
const int tx = 9;
const int rx = 10;
unsigned int data = 0; // data stores the controller response

const unsigned int  iNONE= 0; 
const unsigned int  iUP	= 8;  
const unsigned int  iDOWN =2; 
const unsigned int  iLEFT =1; 
const unsigned int  iRIGHT =4; 
const unsigned int  iURIGHT =12; 
const unsigned int  iULEFT =9; 
const unsigned int  iDRIGHT =6; 
const unsigned int  iDLEFT  =3; 
const unsigned int  iR1	= 45056; 
const unsigned int  iR2	= 16384; 
const unsigned int  iX	= 512; 
const unsigned int  iTRIANGLE = 2048; 
const unsigned int  iSQUARE = 256; 
const unsigned int  iCIRCLE = 1024;
const unsigned int  iSTART = 16; 
const unsigned int  iSELECT = 128;


const unsigned char  NONE= 0; 
const unsigned char  UP	= 2;  
const unsigned char  DOWN =4; 
const unsigned char  LEFT =6; 
const unsigned char  RIGHT =8; 
const unsigned char  URIGHT =10; 
const unsigned char  ULEFT =12; 
const unsigned char  DRIGHT =14; 
const unsigned char  DLEFT  =16; 
const unsigned char  R1	= 18; 
const unsigned char  R2	= 20; 
const unsigned char  X	= 22; 
const unsigned char  TRIANGLE = 24; 
const unsigned char  SQUARE = 26; 
const unsigned char  CIRCLE = 28; 
const unsigned char  START = 30; 
const unsigned char  SELECT = 32;


void setup()
{
  pinMode(tx, OUTPUT);
  digitalWrite( tx, LOW);
  // initialize the Psx library
  Psx.setupPins(dataPin, cmndPin, attPin, clockPin, psxDelay);
  Serial.begin(9600); // results will be displayed on the Serial Monitor
}


void bitbanging (unsigned char psxbutton) {
  int j;
  unsigned char txon;

digitalWrite( tx, HIGH);
delay(50);

for (j=0; j<8; j++)
{
 txon=psxbutton&(0x01<<j)?1:0;
  if(txon==1) {
   digitalWrite( tx, HIGH);
 }
 else
   {
   digitalWrite( tx, LOW);
 }
delay(50);
}

digitalWrite( tx, LOW);
//Send the stop bits
delay(100);
  
}


void loop()
{
  data = Psx.read();        // get the psx controller button data
  // check the button bits to see if a button is pressed
    
 
    if(data & iLEFT ) { //leftbutton
    Serial.println(data);
       if(data & iUP) {
      bitbanging(ULEFT);
      data=0;
       }
      else if (data & iDOWN){
      bitbanging(DLEFT);
       data=0;
      }
      else{
      bitbanging(LEFT);
       data=0;
      }
  }
  
  if(data & iRIGHT) { //"right button"
  Serial.println(data);
    if(data & iUP) {
      bitbanging(URIGHT);
       data=0;
       }
      else if (data & iDOWN){
      bitbanging(DRIGHT);
       data=0;
      }
      else {
  bitbanging(RIGHT);
   data=0;
      }
  }  
  
  
   if(data & iDOWN) { //down button
   Serial.println(data);
    if(data & iRIGHT) {
      bitbanging(DRIGHT);
       data=0;
       }
      else if (data & iLEFT){
      bitbanging(DLEFT);
       data=0;
      }
      else {
   bitbanging(DOWN);
    data=0;
    }
   }
   
  
   if(data & iUP) {//up button
  Serial.println(data);
  if(data & iRIGHT) {
      bitbanging(URIGHT);
       data=0;
       }
      else if (data & iLEFT){
      bitbanging(ULEFT);
       data=0;
      }
      else {
       bitbanging(UP);
        data=0;
    } 
   }





 
   
   if(data & iSTART) {//start button
  Serial.println(data);
       bitbanging(START);
  } 
  
  if(data & iSELECT) {//select button
  Serial.println(data);
      bitbanging(SELECT);
  }
  
  if(data & iTRIANGLE) {//triangle button
  Serial.println(data);
        bitbanging(TRIANGLE);
  }
  
   if(data & iX) {//x button
  Serial.println(data);
       bitbanging(X);
  } 
  
  if(data & iCIRCLE){//circle button
  Serial.println(data);
       bitbanging(CIRCLE);
  } 
  
    if(data & iR1) {//r1 button
   Serial.println(data);
       bitbanging(R1);
  } 
  
   if(data & iSQUARE) {//square button
  Serial.println(data);
       bitbanging(SQUARE);
  }
  
   if(data & iR2) {//r2 button
   Serial.println(data);
    bitbanging(R2);
  }
  
  delay(10);
}
