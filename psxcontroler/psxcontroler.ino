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

unsigned int data = 0;           // data stores the controller response

void setup()
{
  // initialize the Psx library
  Psx.setupPins(dataPin, cmndPin, attPin, clockPin, psxDelay);
  Serial.begin(9600); // results will be displayed on the Serial Monitor
}


void loop()
{
  data = Psx.read();        // get the psx controller button data

  // check the button bits to see if a button is pressed
  if(data & 1)
    Serial.println("left button");
  if(data & 2)
    Serial.println("down button");
  if(data & 4)
    Serial.println("right button");
  if(data & 8)
    Serial.println("up button");
  if(data & 16)
    Serial.println("start button");
  if(data & 128)
    Serial.println("select button");
  
  if(data & 2048)
    Serial.println("triangle button");
  if(data & 512)
    Serial.println("x button");
  if(data & 1024)
    Serial.println("circle button");
  if(data & 256)
    Serial.println("square button");
    
   if(data & 45056)
    Serial.println("R1 button");
   if(data & 16384)
    Serial.println("R2 button");

  delay(100);
}
