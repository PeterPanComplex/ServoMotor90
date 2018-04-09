#include <Servo.h>
#include <LiquidCrystal.h>

/* Detects patterns of knocks and triggers a motor to unlock
   it if the pattern is correct.
   
   By Steve Hoefer http://grathio.com
   Version 0.1.10.20.10
   Licensed under Creative Commons Attribution-Noncommercial-Share Alike 3.0
   http://creativecommons.org/licenses/by-nc-sa/3.0/us/
   (In short: Do what you want, just be sure to include this line and the four above it, and don't sell it or use it in anything you sell without contacting me.)
   
   Analog Pin 0: Piezo speaker (connected to ground with 1M pulldown resistor)
   Digital Pin 2: Switch to enter a new code.  Short this to enter programming mode.
   Digital Pin 3: DC gear reduction motor attached to the lock. (Or a motor controller or a solenoid or other unlocking mechanisim.)
   Digital Pin 4: Red LED. 
   Digital Pin 5: Green LED. 
   
   Update: Nov 09 09: Fixed red/green LED error in the comments. Code is unchanged. 
   Update: Nov 20 09: Updated handling of programming button to make it more intuitive, give better feedback.
   Update: Jan 20 10: Removed the "pinMode(knockSensor, OUTPUT);" line since it makes no sense and doesn't do anything.
 */
 
// Pin definitions
const int knockSensor = 0;         // Piezo sensor on pin 0.
const int lockMotor = 3;           // Gear motor used to turn the lock.
const int redLED = 4;              // Status LED
const int greenLED = 5;            // Status LED
const int servoPin = 7;
LiquidCrystal lcd(13, 12, 11, 10, 9, 8);
int angle = 0; // servo position in degrees 

 
// Tuning constants.  Could be made vars and hoooked to potentiometers for soft configuration, etc.
const int threshold = 3;           // Minimum signal from the piezo to register as a knock
const int rejectValue = 25;        // If an individual knock is off by this percentage of a knock we don't unlock..
const int averageRejectValue = 15; // If the average timing of the knocks is off by this percent we don't unlock.
const int knockFadeTime = 150;     // milliseconds we allow a knock to fade before we listen for another one. (Debounce timer.)
const int lockTurnTime = 650;      // milliseconds that we run the motor to get it to go a half turn.

const int maximumKnocks = 20;       // Maximum number of knocks to listen for.
const int knockComplete = 1200;     // Longest time to wait for a knock before we assume that it's finished.

Servo servo;

// Variables.
int secretCode1[maximumKnocks] = {50, 25, 25, 50, 100, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  // Initial setup: "Shave and a Hair Cut, two bits."
int secretCode2[maximumKnocks] = {50, 25, 25, 50, 100, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int secretCode3[maximumKnocks] = {50, 25, 25, 50, 100, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int knockReadings[maximumKnocks];   // When someone knocks this array fills with delays between knocks.
int knockSensorValue = 0;           // Last reading of the knock sensor.
int WhoYouAre = 0;

int validateKnock();

void setup() {
  pinMode(lockMotor, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);

  servo.attach(servoPin);
  
  Serial.begin(9600);               			// Uncomment the Serial.bla lines for debugging.
  
  digitalWrite(greenLED, HIGH);      // Green LED on, everything is go.
}

void loop() {
  // Listen for any knock at all.
  knockSensorValue = analogRead(knockSensor);  
  if (knockSensorValue >=threshold){
    listenToSecretKnock();
  }
} 

// Records the timing of knocks.
void listenToSecretKnock(){
  //Serial.println("knock starting");   
  int i = 0;
  // First lets reset the listening array.
  for (i=0;i<maximumKnocks;i++){
    knockReadings[i]=0;
  }
  
  int currentKnockNumber=0;         			// Incrementer for the array.
  int startTime=millis();           			// Reference for when this knock started.
  int now=millis();
  
  digitalWrite(greenLED, LOW);
  delay(knockFadeTime);                       	        // wait for this peak to fade before we listen to the next one.
  digitalWrite(greenLED, HIGH);
  do {
    //listen for the next knock or wait for it to timeout. 
    knockSensorValue = analogRead(knockSensor);
    if (knockSensorValue >=threshold){                   //got another knock...
      //record the delay time.
      now=millis();
      knockReadings[currentKnockNumber] = now-startTime;
      currentKnockNumber ++;                             //increment the counter
      startTime=now;          
      // and reset our timer for the next knock
      digitalWrite(greenLED, LOW);
      delay(knockFadeTime);                              // again, a little delay to let the knock decay.
      digitalWrite(greenLED, HIGH);
    }

    now=millis();
    
    //did we timeout or run out of knocks?
  } while ((now-startTime < knockComplete) && (currentKnockNumber < maximumKnocks));
  
  //we've got our knock recorded, lets see if it's valid
    if(validateKnock() == 1)
    {
      triggerDoorUnlock(1); 
    }
    else if(validateKnock() == 2)
    {
      triggerDoorUnlock(2); 
    }
    else if(validateKnock() == 3)
    {
      triggerDoorUnlock(3); 
    }
    else {
      lcd.print("Secret knock failed.");
      digitalWrite(greenLED, LOW);  		// We didn't unlock, so blink the red LED as visual feedback.
      for (i=0;i<4;i++){					
        digitalWrite(redLED, HIGH);
        delay(100);
        digitalWrite(redLED, LOW);
        delay(100);
      }
      digitalWrite(greenLED, HIGH);
    }
    lcd.noDisplay();  
}


// Runs the motor (or whatever) to unlock the door.
void triggerDoorUnlock(int WhoYouAre){
  int i=0 ;  
  // rotate from 0 to 180 degrees
  for(angle = 0; angle < 95; angle++) 
  { 
      servo.write(angle); 
      delay(30); 
  }
  unsigned long startLCD = millis();
  while(millis() < startLCD + 5000){
    if(WhoYouAre == 1)
    {
      lcd.print("Kim Doyeon");
    }
    else if(WhoYouAre == 2)
    {
      lcd.print("Bae Euibin");
    }
    else if(WhoYouAre == 3)
    {
      lcd.print("Lee Jieon");
    }
    unsigned long starton = millis();
    while (millis() < starton + 500){   
      digitalWrite(greenLED, LOW);
    }
    unsigned long startoff = millis();
    while (millis() < startoff + 500)
    {
      digitalWrite(greenLED, HIGH);
    }
  }
  lcd.noDisplay();
  
  for(angle=95; angle >=0; angle--){    // goes from 180 degrees to 0 degrees
    servo.write(angle);
    delay(30); 
  }
}

// Sees if our knock matches the secret.
// returns true if it's a good knock, false if it's not.
// todo: break it into smaller functions for readability.
int validateKnock(){
  int i=0;
 
  // simplest check first: Did we get the right number of knocks?
  int currentKnockCount = 0;
  int secretKnockCount1 = 0;
  int secretKnockCount2 = 0;
  int secretKnockCount3 = 0;
  int maxKnockInterval = 0;          			// We use this later to normalize the times.
  
  for (i=0;i<maximumKnocks;i++){
    if (knockReadings[i] > 0){
      currentKnockCount++;
    }
    if (secretCode1[i] > 0){  					//todo: precalculate this.
      secretKnockCount1++;
    }
    if (secretCode2[i] > 0){            //todo: precalculate this.
      secretKnockCount2++;
    }
    if (secretCode3[i] > 0){            //todo: precalculate this.
      secretKnockCount3++;
    }
    
    if (knockReadings[i] > maxKnockInterval){ 	// collect normalization data while we're looping.
      maxKnockInterval = knockReadings[i];
    }
  } 
  if (currentKnockCount != secretKnockCount1 && currentKnockCount != secretKnockCount2 && currentKnockCount != secretKnockCount3){
    return false; 
  }
  
  /*  Now we compare the relative intervals of our knocks, not the absolute time between them.
      (ie: if you do the same pattern slow or fast it should still open the door.)
      This makes it less picky, which while making it less secure can also make it
      less of a pain to use if you're tempo is a little slow or fast. 
  */
  int totaltimeDifferences=0;
  int timeDiff=0;
  int Allmismatch = 0;
  int mismatch1 = 0;
  int mismatch2 = 0;
  int mismatch3 = 0;
  for (i=0;i<maximumKnocks;i++){ // Normalize the times
    knockReadings[i]= map(knockReadings[i],0, maxKnockInterval, 0, 100);      
    timeDiff = abs(knockReadings[i]-secretCode1[i]);
    if (timeDiff > rejectValue){ // Individual value too far out of whack
      mismatch1++;
      break;
    }
    totaltimeDifferences += timeDiff;
  }
  if (totaltimeDifferences/secretKnockCount1>averageRejectValue || mismatch1 == 1){
    Allmismatch++;
  }
  totaltimeDifferences = 0;
  for (i=0;i<maximumKnocks;i++){ // Normalize the times
    knockReadings[i]= map(knockReadings[i],0, maxKnockInterval, 0, 100);      
    timeDiff = abs(knockReadings[i]-secretCode2[i]);
    if (timeDiff > rejectValue){ // Individual value too far out of whack
      mismatch2++;
      break;
    }
    totaltimeDifferences += timeDiff;
  }
  if (totaltimeDifferences/secretKnockCount2>averageRejectValue || mismatch2 == 1){
    Allmismatch++;
  }
  totaltimeDifferences = 0;
  for (i=0;i<maximumKnocks;i++){ // Normalize the times
    knockReadings[i]= map(knockReadings[i],0, maxKnockInterval, 0, 100);      
    timeDiff = abs(knockReadings[i]-secretCode3[i]);
    if (timeDiff > rejectValue){ // Individual value too far out of whack
      mismatch3++;
      break;
    }
    totaltimeDifferences += timeDiff;
  }
  if (totaltimeDifferences/secretKnockCount3>averageRejectValue || mismatch3 == 1){
    Allmismatch++;
  }
  if(Allmismatch == 3)
  {
    return 0;  
  }
  if(mismatch1 == 0)
  {
    return 1;
  }
  else if(mismatch2 == 0)
  {
    return 2;
  }
  else if(mismatch3 == 0)
  {
    return 3;
  }  
}
