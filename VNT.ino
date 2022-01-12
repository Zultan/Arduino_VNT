#include <PWM.h>


//VNT duty:
//  255 = 100% open vanes - minimum boost
//  0   =   0% open vanes - maximum boost


//VARIABLES for input
 unsigned int ontime, offtime;  
 float freq, duty, period;
 int newsweep;
 int analogPin = A0;      //MAP sensor input pin
 int mapvolt = 0;         //analog input for MAP sensor
 int dutyadd = 0;
 int lastduty = 0;

//STATEMENTS
 int boostok = 1; //Statement for finding ideal VNT position for desired boost. Start with this, calcerr & overboost statement enabled, and find required VNT position to reach requested boost. Spool will be long, but with other statments disabled it is OK.
 int calcerr = 1;  //Calculation error correction statement
 int fastspool = 1; //Statement for accelerating turbo spool (mostly for low rpm conditions)
 int spikeprevent = 1; //Boost spike prevention (activated before reaching boost target)
 int spikereduct = 1; //Boost spike reduction (activated after exceeding boost target)
 int overboost = 1; //Protection in case of overboost
 int antislutter = 1; //Anti-slutter protection. Opening VNT vanes too much will result in slutter. Slutter protection from too closed vanes is integrated in "fastspool"
 

//VARIABLES for output
 int outputpin = 8;       // VNT output pin
 int32_t frequency = 300; //VNT output frequency (in Hz)


void setup()
{
//output setup
  InitTimersSafe(); //init timer
  bool success = SetPinFrequencySafe(outputpin, frequency); //configure frequency for specific pin

//input setup
  int sensorValue = analogRead(A0);
  pinMode(7,INPUT);

//debug setup
  Serial.begin(9600); // open the serial port at 9600 bps:
}


void loop()
{
  mapvolt = analogRead(analogPin);  //read MAP voltage
  
//measuring solenoid input high/low times from ecu and turning it into 0-100% duty
  ontime = pulseIn(7,HIGH);
  offtime = pulseIn(7,LOW);
  period = ontime+offtime;
  freq = 1000000.0/period;
  duty = (ontime/period)*100; 
  freq = ontime / period;
  duty = 2.5*((freq * 100));


//Safety in case of calculation error of ecu solenoid duty

  if(calcerr = 1 & freq>1.3)
  {
    duty=135;
  }

//Fast spool in case ECU asks for closed vanes and boost does not exceed 2600mbar. 45 duty is the minimum safe vane position in this case. All below 45 will result in slutter.
  
  if(fastspool = 1 & duty <= 55 & mapvolt <= 335)
    {
    duty = 45;
    }

//If 2600mbar is reached start to accelerate VNT opening to prevent boost spike

  if(spikeprevent = 1 & duty <= 135 & mapvolt > 335)
  {
    dutyadd = ((mapvolt - 335) + 55);
    duty = dutyadd;
  }

//More or less stable VNT position for 2700mbar. Tweak this to find more/less stable VNT position for desired boost before tweaking other conditions. 

  if(boostok = 1 & mapvolt >= 350) // if 2700mbar achieved
  {
    duty = 135;
    }

//In case we are going over boost limiter (2735mbar) but have not reached overboost protection, accelerate the opening of turbo vanes to reduce boost spike
  
  if(spikereduct = 1 & duty >= 135 & mapvolt > 355) // 
  {
    dutyadd = ((mapvolt - 355) + 135);
    duty = dutyadd;
  }

//Overboost protection in case of exceeding 3000mbar absolute pressure. Open vanes all the way.
  
  if(overboost = 1 & mapvolt >= 390 ) // over 3000mbar protection
    {
    duty = 205;
    }

//VNT vanes opening limitation of more than 80% to prevent turbo slutter

 if(antislutter = 1 & duty >= 205)
    {
    duty = 205;
    }

//Code for applying output duty
  lastduty = duty;
  pwmWrite(outputpin, duty);  //determine dutycycle on pin 8


  
  //Serial output for testing.
/*
  Serial.print((freq*100), DEC);
  Serial.print("\n");
  Serial.print(duty, DEC);
  Serial.print("\n");
  Serial.print(mapvolt, DEC);
  Serial.print("\n");
*/
  
}
