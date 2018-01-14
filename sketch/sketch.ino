#include <ZumoBuzzer.h>
#include <ZumoMotors.h>
#include <Pushbutton.h>
#include <QTRSensors.h>
#include <ZumoReflectanceSensorArray.h>
#include <NewPing.h>
#include <Room.h>
#include <Corridor.h>
#include <StandardCplusplus.h> 
#include <Vector>

using namespace std; //for our vector

//Zumo stuff...
#define SENSOR_THRESHOLD 1000; //desired color threshold for QTR sensors. 
#define NUM_SENSORS 6 //no. of sensors that the ZumoReflectanceSensorArray has
#define LED 13 //for the Zumo LED

//NewPing stuff...
#define TRIGGER_PIN 2   //specify which pin the U/S sensor trigger is wired to on the Arduino board.
#define ECHO_PIN 6      //specify which pin the U/S sensor echo pin is wired to on the Arduino board.
#define MAX_DISTANCE 30 //specify the desired max distance of the sonar range

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); //construct U/S sensor object with relevant info 
ZumoBuzzer buzzer;
ZumoMotors motors;
ZumoReflectanceSensorArray sensors(QTR_NO_EMITTER_PIN);
Pushbutton button(ZUMO_BUTTON); // pushbutton on pin 12
unsigned int sensor_values[NUM_SENSORS]; //create an array which holds an int value for each one of our sensors to return a value to.
int roomCounter = 0;
int corridorCounter = 1; //set to 1 as our Zumo will always already be inside a corridor when starting.
char val; //Data received from the serial port

//flags...
bool isInSubCorridor = false;
bool leftCorridor = false;
bool leftTurnOnly = false;
bool rightCorridor = false;
bool rightTurnOnly = false;

//the corridor and room data corresponding to the Zumo's location 
Corridor* theCorridor = new Corridor(corridorCounter);
Room* theRoom = new Room(roomCounter, corridorCounter);

//store room objects for places we have previously been.
vector<Room*> checkedRooms;

void setup() {

  //turn LED on and wait for button push before playing tune and handshake with processing
  digitalWrite(LED, HIGH);
  button.waitForButton();
  digitalWrite(LED, LOW);
  buzzer.play(">g32>>c32");
  delay(1000);

  Serial.begin(9600);
  establishContact();
}

void loop() {

  sensors.read(sensor_values); //read current values of the QTR sensors
  val = Serial.read(); //read incoming data from processing

  switch (val) {//change the bahavior of the Zumo depending on which data has been sent from GUI. 
    case 'Q': //stop button
        motors.setSpeeds(0, 0);
        break;
    case 'W': //forward butto
      while (!isAtDeadEnd() && val != 'Q') {//each time we read the sensor values, do the following if leftmost and rightmost sensors are not above the set threshold
        moveForwardWithinBoundaries();
      }
      motors.setSpeeds(0, 0);

      if (isInSubCorridor == true && val != 'Q') 
      {
        Serial.println("Reached end of sub-corridor, turning around.");
        isInSubCorridor = false;
      }
      else if (val != 'Q') 
      {//if we haven't sent quit command but have broken out of while loop, must be at wall, so send message indicating that
        Serial.println("Wall detected, Zumo stopping.");

        if (leftCorridor == true) {
          theCorridor->setID(theCorridor->getPreviousCorridorID());
          leftTurnOnly = true;
          leftCorridor = false;
          delay(200);
          Serial.println("Right turn restricted. You must turn left to continue search.");
          delay(50);
        }
        else if (rightCorridor == true) {
          theCorridor->setID(theCorridor->getPreviousCorridorID());  
          rightTurnOnly = true;
          rightCorridor = false;
          delay(200);
          Serial.println("Left turn restricted. You must turn right to continue search.");        
          delay(50);
        }
      }
      //Serial.println(leftTurnOnly);
      break;
    case 'A': //left button
      if (leftTurnOnly == true){
        leftTurnOnly = false;
      }

      if (rightTurnOnly == false) {
        motors.setSpeeds(-100, 100); //rotate left
      }
      break;
    case 'S': //backwards button
      motors.setSpeeds(-100, -100); //go backwards
      break;
    case 'D': //right button
      if (rightTurnOnly == true) {
        rightTurnOnly = false;
      }
    
      if (leftTurnOnly == false) {
        motors.setSpeeds(100, -100); //rotate right
      }
      break;
    case 'L'://left room signal button
      signalRoom('L');
      break;
    case 'R'://right room signal button
      signalRoom('R');
      break;
    case 'C': //corridor left signal
      Serial.println("Turn into left sub-corridor now.");
      theCorridor = new Corridor(++corridorCounter);
      isInSubCorridor = true;
      leftCorridor = true;
      break;
    case 'V': //corridor right signal
      Serial.println("Turn into right sub-corridor now.");
      theCorridor = new Corridor(++corridorCounter);
      isInSubCorridor = true;
      rightCorridor = true; 
      break;     
    case 'X': //scan button
      scanRoom();
      break;
  }
}

void moveForwardWithinBoundaries() {
  motors.setSpeeds(100, 100);
  sensors.read(sensor_values);
  val = Serial.read();

  if (overLine(sensor_values[0])) //if leftmost sensor detects the border
  {
    motors.setSpeeds(0, 100); //power right motor for 70ms and then stop (this is to make sure the sensors successfully detect a dead end)
    delay(70);
    motors.setSpeeds(0, 0);
    sensors.read(sensor_values); //read sensor values

    if (overLine(sensor_values[0]) && !overLine(sensor_values[5])) //now if the leftmost sensor detects the border and the rightmost sensor does not, safe to assume it is not a dead end
    {
      motors.setSpeeds(-100, -100); //reverse
      delay(300); //for 200ms
      motors.setSpeeds(100, -100); //then rotate right
      delay(200); //for 200ms
    }
  }
  else if (overLine(sensor_values[5])) //same again for the other (rightmost) side of the Zumo...
  {
    motors.setSpeeds(100, 0);
    delay(70);
    motors.setSpeeds(0, 0);
    sensors.read(sensor_values);

    if (overLine(sensor_values[5]) && !overLine(sensor_values[0]))
    {
      motors.setSpeeds(-100, -100); //reverse
      delay(300); //for 200ms
      motors.setSpeeds(-100, 100); //then rotate right
      delay(200); //for 200ms
    }
  }
}

bool isAtDeadEnd() {//return true if leftmost and rightmost sensors detect border
  return ( overLine(sensor_values[0]) && overLine(sensor_values[5]) );
}

bool overLine(int sensorPin) {//returns true if passed sensor is over border
  return sensorPin > SENSOR_THRESHOLD;
}

void establishContact() {
  while (Serial.available() <= 0) {
    Serial.println("!");   // send a ! (to request data)
    delay(300);
  }
}

void signalRoom(char inDirection) {
  
  theRoom = new Room(++roomCounter, theCorridor->getID()); //set our current room as the room we have signalled.
  
  Serial.print("Room found in corridor ");
  Serial.print(theCorridor->getID());

  if (inDirection == 'L') {
    Serial.println(" to our left.");
  }
  else
  {
    Serial.println(" to our right.");
  }
}

void resetTurnLimiterFlags() {
  leftTurnOnly = false;
}

int getObjectDistance() {//returns distance of object infront of U/S sensor 
  return sonar.ping() / US_ROUNDTRIP_CM;
}

void scanRoom() {
  bool objectFoundFlag = false;

  //add the room being scanned into the collection of checked rooms
  checkedRooms.push_back (theRoom);

  for (int i = 0; i < 15; ++i)
  {
    motors.setSpeeds(150,-150);
    delay(200);
    motors.setSpeeds(0,0);
    delay(100);

    if (getObjectDistance() > 0)
    {
      objectFoundFlag = true;
    }
    delay(500);
  }
  motors.setSpeeds(0, 0);

  if (objectFoundFlag)
  {
    Serial.print("Object was detected in room ");
    Serial.print(String(theRoom->getID()));
    Serial.println(".");
  }
  else
  {
    Serial.print("Object was not detected in room ");
    Serial.print(String(theRoom->getID()));
    Serial.println(".");
  }
}


