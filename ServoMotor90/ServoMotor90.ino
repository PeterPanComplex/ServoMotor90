#include <Servo.h> 

Servo servo;

int servoPin = 7;
int angle = 0; // servo position in degrees 

void setup() 
{ 
  servo.attach(servoPin);
} 

void loop() 
{ 
  // rotate from 0 to 180 degrees
  for(angle = 0; angle < 95; angle++) 
  { 
    servo.write(angle); 
      delay(30); 
  }
  for(angle=95; angle >=0; angle--){    // goes from 180 degrees to 0 degrees
    servo.write(angle);
    delay(30);
}
