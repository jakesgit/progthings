import g4p_controls.*;
import processing.serial.*;

Serial myPort;  // Create object from Serial class
int bgcol = 15;
int count = 0;
String portName;
String val;
boolean firstContact = false;


public void setup() {
  size(500, 360);

  portName = Serial.list()[1];
  myPort = new Serial(this, portName, 9600);
  myPort.bufferUntil('\n');

  createGUI();
}

public void draw() {
  background(200, 200, 200);
}

void serialEvent(Serial myPort) {
  val = myPort.readStringUntil('\n');

  if (val != null)
  { //make sure our data isn't empty before continuing
    val = trim(val); //trim whitespace and formatting characters
    //println(val);
    //look for our '!' string to start the handshake
    //if it's there, clear the buffer, and send a request for data
    if (firstContact == false)
    {
      if (val.equals("!")) 
      {
        myPort.clear();
        firstContact = true;
        myPort.write("!");
        println("Contact made.");
       // txtMsgFromZumo.appendText("\n" + val);
      }
    }
    else 
    { //if we've already established contact, keep getting and parsing data
      //println(val + " after else stmt");

      txtMsgFromZumo.appendText(++count + ": " + val);

      // when you've parsed the data you have, ask for more:
      myPort.write("!");
    }
  }
}