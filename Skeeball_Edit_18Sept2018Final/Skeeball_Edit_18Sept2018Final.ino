
// SoftwareSerial - Version: Latest
#include <SoftwareSerial.h>

// Keyboard - Version: Latest
#include <Keyboard.h>

#include <AccelStepper.h>
#define HALFSTEP 8  //Half-step mode (8 step control signal sequence)

// Motor pin definitions
#define mtrPin1  1     // IN1 on the ULN2003 driver 1
#define mtrPin2  2     // IN2 on the ULN2003 driver 1
#define mtrPin3  A4     // IN3 on the ULN2003 driver 1
#define mtrPin4  A5     // IN4 on the ULN2003 driver 1

//Assign pins
int pocket1 = A1; //2 should have been the sequential pin here, fuck logic.
int pocket2 = 3;
int pocket3 = 4;
int pocket4 = 5;
int pocket5 = 6;
int pocket6 = 7;
int retSensor = 8;
int up = 9;
int dwn = 10;
int lft = A0; //again. Who tf wrote this? Oh that was me.
int button1 = 11;
int rt = 12;
int button2 = 13;
int relayPin = 2; //Hey there's the two - it's not even fucking used anymore because we blew up the solenoid
int devMode = A2;

//The pin state from the previous loop (true = HIGH)
bool returnPrev = false;
bool pock1Prev = false;
bool pock2Prev = false;
bool pock3Prev = false;
bool pock4Prev = false;
bool pock5Prev = false;
bool pock6Prev = false;
bool button1Prev = false;
bool button2Prev = false;
bool upPrev = false;
bool dwnPrev = false;
bool lftPrev = false;
bool rtPrev = false;

AccelStepper stepper1(HALFSTEP, mtrPin1, mtrPin3, mtrPin2, mtrPin4);

bool rotQueued;
bool inRotation = false;
float lastReturnHitTime = 0;
//unsigned long systemTime = 0;
//const long interval = 500;

int rotTarget = 0;

void setup()
{
  stepper1.setMaxSpeed(1000.0);
  stepper1.setAcceleration(999.0);  //Make the acc quick (2 pump-chump quick)
  stepper1.setSpeed(1000);

  //Sets up everything on the skeeball machine as an input or output and assigns their pin number
  pinMode(pocket1, INPUT);
  pinMode(pocket2, INPUT);
  pinMode(pocket3, INPUT);
  pinMode(pocket4, INPUT);
  pinMode(pocket5, INPUT);
  pinMode(pocket6, INPUT);
  pinMode(retSensor, INPUT);
  pinMode(up, INPUT);
  pinMode(dwn, INPUT);
  pinMode(lft, INPUT);
  pinMode(rt, INPUT);
  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(devMode, INPUT);
  digitalWrite(A1, HIGH);
  digitalWrite(A2, LOW);
  Serial.begin(9600);

}

//reads serial data from the computer
void loop() {
  //systemTime = millis(); // starts a timer
  //if (systemTime >= 400000000) // if said timer is greater than or equal to 400 million milliseconds than
    //systemTime = 0; // reset it to 0 and start over. systemTime is an unsigned long which contains 4,294,967,295 bytes of data.
  handleSerial();
}


void handleSerial() {
  if (Serial.available() > 0) { // If there is data in the serial buffer than...
    char received = Serial.read(); // Take the shit from the serial buffer...
    char inData = inData + received;  // and store it in inData for the switch function to read
    switch (inData) {

      /*Tells skeeball machine whether the machine is in development mode or not.
        Tells skeeball machine the board ID in number of u's and y to complete */
      case 'b':
        if (digitalRead(A2) == HIGH) {
          Keyboard.press('u');
          delay(10);
          Keyboard.release('u');
          Keyboard.press('y');
          delay(10);
          Keyboard.release('y');
          Keyboard.press('o');
          delay(10);
          Keyboard.release('o');
        } else {
          Keyboard.press('u');
          delay(10);
          Keyboard.release('u');
          Keyboard.press('y');
          delay(10);
          Keyboard.release('y');
          Keyboard.press('i');
          delay(10);
          Keyboard.release('i');
          break;
        }

      //Stops balls when 'c' received from pc
      case 'x':
        rotTarget = stepper1.currentPosition() + 1500;
        rotQueued = true;
        break;

      //Releases balls when 'x' received from pc
      case 'c':
        rotTarget = stepper1.currentPosition() - 1500;
        rotQueued = true;
        break;


    }
    inData = ""; // Clear the shit stored from the buffer
  }
  //Serial.end();
  //Serial.begin(9600);
  //Check sensors            (int pinToRead, char keyToPress, bool prevState) [returns the new pin state to be used on the following loop]
  returnPrev = updateKeyStroke(retSensor, '0', returnPrev);
  pock1Prev = updateKeyStroke(pocket1, '1', pock1Prev);
  pock2Prev = updateKeyStroke(pocket2, '2', pock2Prev);
  pock3Prev = updateKeyStroke(pocket3, '3', pock3Prev);
  pock4Prev = updateKeyStroke(pocket4, '4', pock4Prev);
  pock5Prev = updateKeyStroke(pocket5, '5', pock5Prev);
  pock6Prev = updateKeyStroke(pocket6, '6', pock6Prev);

  //Check buttons              (int pinToRead, char keyToPress, bool prevState) [returns the new pin state to be used on the following loop]
  button1Prev = updateKeyStroke(button1, 'q', button1Prev);
  button2Prev = updateKeyStroke(button2, 'e', button2Prev);

  //Check joystick
  upPrev = updateKeyStroke(up, 'w', upPrev);
  dwnPrev = updateKeyStroke(dwn, 's', dwnPrev);
  lftPrev = updateKeyStroke(lft, 'a', lftPrev);
  rtPrev = updateKeyStroke(rt, 'd', rtPrev);

  if (rotQueued)
  {
    //if (millis() - lastReturnHitTime >= 10000)
    //{
    rotQueued = false;
    inRotation = true;
    stepper1.moveTo(rotTarget);
    // }
  }
  else if (inRotation)
  {
    stepper1.run();
    if (stepper1.distanceToGo() == 0)
    {
      inRotation = false;
      stepper1.stop();
    }
  }
}

//lol try to follow along
bool updateKeyStroke (int pinToRead, char keyToPress, bool prevState) { //Returns updated value for prevState
  if (prevState == true) { // We're using 400 million bytes to be safe because if you went over 4.3b it would fault the arduino.
    if (digitalRead(pinToRead) == LOW) { // && systemTime > interval) { Was high, now low, release key
      Keyboard.release(keyToPress);
      if (keyToPress == '0') //If this is the return sensor
      //  systemTime = 0;
      {
        lastReturnHitTime = millis(); //Update last hit time for gate operations
      }
      return false;
    }
    else {
      return true;
    }
  }
  else {
    if (digitalRead(pinToRead) == HIGH){ // && systemTime > interval) { Was low, now high, press key
      Keyboard.press(keyToPress);
      //systemTime = 0;
      return true;
    }
    else {
      return false;
    }
  }
}
