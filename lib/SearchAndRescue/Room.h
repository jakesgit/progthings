#ifndef Room_H
#define Room_H


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
#endif
