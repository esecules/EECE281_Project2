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


const unsigned char  UP	= 1;  
const unsigned char  DOWN =2; 
const unsigned char  LEFT =3; 
const unsigned char  RIGHT =4; 
const unsigned char  UPRIGHT =5; 
const unsigned char  UPLEFt =6; 
const unsigned char  DOWNRIGHT =7; 
const unsigned char  DOWNLEFT  =8; 
const unsigned char  R1	= 9; 
const unsigned char  R2	= 10; 
const unsigned char  X	= 11; 
const unsigned char  TRIANGLE = 12; 
const unsigned char  SQUARE = 13; 
const unsigned char  CIRCLE = 14; 
const unsigned char  START = 15; 
const unsigned char  SELECT = 16;


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
 else  {
   digitalWrite( tx, LOW);
 }
delay(50);
}

digitalWrite( tx, LOW);
//Send the stop bits
delay(50);
delay(50);  
}


void loop()
{
  data = Psx.read();        // get the psx controller button data
  Serial.println(data);
  // check the button bits to see if a button is pressed
  if(data & 1) { //leftbutton
    Serial.println(3);
    bitbanging(LEFT);
  }
  
  if(data & 2) {//down button
    Serial.println(2);
    bitbanging(DOWN);
  }
  
  if(data & 4) { //"right button"
    Serial.println(4);
      bitbanging(RIGHT);
  }  
  
  if(data & 8) {//up button
    Serial.println(1);
       bitbanging(UP);
  } 
  
  if(data & 16) {//start button
    Serial.println(15);
       bitbanging(START);
  } 
  
  if(data & 128) {//select button
    Serial.println(16);
      bitbanging(SELECT);
  }
  
  if(data & 2048) {//triangle button
    Serial.println(12);
        bitbanging(TRIANGLE);
  }
  
  if(data & 512) {//x button
    Serial.println(11);
       bitbanging(X);
  } 
  
  if(data & 1024){//circle button
    Serial.println(14);
       bitbanging(CIRCLE);
  } 
  
  if(data & 256) {//square button
    Serial.println(13);
        bitbanging(SQUARE);
  }
  
   if(data & 45056) {//r1 button
    Serial.println(9);
       bitbanging(R1);
  } 
  
   if(data & 16384) {//r2 button
    Serial.println(10);
    bitbanging(R2);
  }
  
  delay(100);
}
