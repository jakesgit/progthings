#include <ZumoBuzzer.h>
#include <ZumoMotors.h>
#include <Pushbutton.h>
#include <QTRSensors.h>
#include <ZumoReflectanceSensorArray.h>
#include <NewPing.h>


#define LED 13
#define SENSOR_THRESHOLD 1000;

#define TRIGGER_PIN 2
#define ECHO_PIN 6
#define MAX_DISTANCE 20
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); //NewPing constructor with necessary info

#define NUM_SENSORS 6
unsigned int sensor_values[NUM_SENSORS];

//zumo stuff...
ZumoBuzzer buzzer;
ZumoMotors motors;
ZumoReflectanceSensorArray sensors(QTR_NO_EMITTER_PIN);
Pushbutton button(ZUMO_BUTTON); // pushbutton on pin 12

char val; //Data received from the serial port
int roomCounter = 0;
int corridorCounter = 1;

bool isInSubCorridor = false;
bool leftCorridor = false;
bool leftTurnOnly = false;
bool rightCorridor = false;
bool rightTurnOnly = false;

class Room
{
public:
  Room();
  Room(const int&, const int&);
  int getID();
  int getCorridorID();
private: 
  int _corridorID;
  int _roomID;
};

//Room::Room() constructor?
Room::Room() {}

Room::Room(const int& roomID, const int& corridorID) { //int&?
  _roomID = roomID;
  _corridorID = corridorID; 
}

int Room::getID() {
  return _roomID;
}

int Room::getCorridorID() {
  return _corridorID;
}



class Corridor
{ 
public:
  Corridor();
  Corridor(const int&); //int&?
  int getID();
private:
  int _corridorID;
};

Corridor::Corridor() {}

Corridor::Corridor(const int& cID) {
  _corridorID = cID;
}

int Corridor::getID() {
  return _corridorID;
}

//Room* theRoom;
Corridor* theCorridor = new Corridor(corridorCounter);
Room* theRoom = new Room(roomCounter, corridorCounter);



void setup() {
  digitalWrite(LED, HIGH);
  button.waitForButton();
  digitalWrite(LED, LOW);
  //buzzer.playNote(NOTE_G(3), 200, 15);
  buzzer.play(">g32>>c32");
  delay(1000);

  Serial.begin(9600);
  establishContact();
}

void loop() {
  sensors.read(sensor_values);
  val = Serial.read();

  switch (val) {
    case 'Q': //stop button
      motors.setSpeeds(0, 0);

      break;
    case 'W': //forward button
      while (!isAtDeadEnd() && val != 'Q') {//each time we read the sensor values, do the following if leftmost and rightmost sensors are not both over a border.
        moveForwardWithinBoundaries();
      }
      motors.setSpeeds(0, 0);

      if (isInSubCorridor == true && val != 'Q') 
      {
        //do 180.. (eg. turn at this speed and then delay until 180 complete)
        Serial.println("Reached end of sub-corridor, turning around.");
        isInSubCorridor = false;
      }
      else if (val != 'Q') 
      {//if we haven't sent quit command but have broken out of while loop, must be at wall, so send message indicating that
        Serial.println("Wall detected, Zumo stopping.");

        if (leftCorridor == true) {
          leftTurnOnly = true;
          leftCorridor = false;
          Serial.println("Right turn restricted. You must turn left to continue search.");
        }
        else if (rightCorridor == true) {
          rightTurnOnly = true;
          rightCorridor = false;
          Serial.println("Left turn restricted. You must turn right to continue search.");          
        }
      }
      //Serial.println(leftTurnOnly);
      break;
    case 'A': //left button
      if (leftTurnOnly == true){
        leftTurnOnly = false;
      }

      if (rightTurnOnly == false) {
        motors.setSpeeds(-150, 150); //rotate left
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
        motors.setSpeeds(150, -150); //rotate right
      }
      break;

    case 'L'://left room signal button
      signalRoom('L');
      break;
    case 'R'://right room signal button
      signalRoom('R');
      break;
    case 'C': //corridor signal button LEFT
      Serial.println("Turn into left sub-corridor now.");
      theCorridor = new Corridor(++corridorCounter);
     // Serial.println(String(theCorridor->getID()));
      isInSubCorridor = true; //possibly  make Corridor.subcorridor == true? and Corridor.left == true?
      leftCorridor = true;
      break;
    case 'V': //corridor right signal
      Serial.println("Turn into right sub-corridor now.");
      theCorridor = new Corridor(++corridorCounter);
     // Serial.println(String(theCorridor->getID()));
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

  if (overLine(sensor_values[0]))
  { //if leftmost sensor detects the border
    delay(50);
    motors.setSpeeds(0, 0); //wait 50ms and then stop (this is to make sure the sensors successfully detect a dead end)
    sensors.read(sensor_values); //read sensor values

    if (overLine(sensor_values[0]) && !overLine(sensor_values[5])) //now if the leftmost sensor detects the border and the rightmost sensor does not, safe to assume it is not a dead end
    {
      motors.setSpeeds(-100, -100); //reverse
      delay(200); //for 200ms
      motors.setSpeeds(100, -100); //then rotate right
      delay(300); //for 200ms
    }
  }
  else if (overLine(sensor_values[5]))
  {
    delay(50);
    motors.setSpeeds(0, 0);
    sensors.read(sensor_values);

    if (overLine(sensor_values[5]) && !overLine(sensor_values[0]))
    {
      motors.setSpeeds(-100, -100); //reverse
      delay(200); //for 200ms
      motors.setSpeeds(-100, 100); //then rotate right
      delay(300); //for 200ms
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
    Serial.println("!");   // send a !
    delay(300);
  }
}

void signalRoom(char inDirection) {
  
    theRoom = new Room(++roomCounter, theCorridor->getID()); 
//  Serial.print("DEBUG: ");
//  Serial.print(String(theCorridor->getID()));
//  Serial.print(" ");
//  Serial.print(String(theRoom->getCorridorID()));
//  Serial.print(" ");
//  Serial.println(String(theRoom->getID()));
  
  Serial.print("Room found in corridor ");
  Serial.print(theCorridor->getID());
  //print roomcounter too?

  if (inDirection == 'L') {
    Serial.println(" to our left.");
  }
  else
  {
    Serial.println(" to our right.");
  }

//  delete theCorridor;
//  delete theRoom;
}

void resetTurnLimiterFlags() {
  leftTurnOnly = false;
}

int getObjectDistance() {
  return sonar.ping() / US_ROUNDTRIP_CM;
}

void scanRoom() {
  bool objectFoundFlag = false;

  //store theRoom object in array of Rooms

  for (int i = 0; i < 4; ++i)
  {
    if (i == 0 || i == 2)
    {
      motors.setSpeeds(150, -150);
    }
    else
    {
      motors.setSpeeds(-150, 150);
    }

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


