#include <Corridor.h>

Corridor::Corridor() {}

Corridor::Corridor(const int& cID) {
  _corridorID = cID;
  _previousCorridorID = cID - 1;
}

int Corridor::getPreviousCorridorID() {
  return _previousCorridorID;
}

int Corridor::getID() {
  return _corridorID;
}

void Corridor::setID(const int& theID){
  _corridorID = theID;
}
