#include <Room.h>

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
