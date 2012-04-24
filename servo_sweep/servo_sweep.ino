// Sweep
// by BARRAGAN <http://barraganstudio.com> 
// This example code is in the public domain.


#include <Servo.h> 

Servo myservo;  // create servo object to control a servo 
Servo yourservo;  // create servo object to control a servo 
                // a maximum of eight servo objects can be created 
 
int pos = 0;    // variable to store the servo position 
 
void setup() 
{ 
  myservo.attach(9, 544, 2600);  // attaches the servo on pin 9 to the servo object 
  yourservo.attach(10, 544, 2600);
} 
 
void loop() 
{ 
  unsigned long l = millis() % 4000;
  if (l < 2000) {
    myservo.write(0);
  }
  else {
    myservo.write(180);
  }
  yourservo.write(l / 800 * 45);
} 
