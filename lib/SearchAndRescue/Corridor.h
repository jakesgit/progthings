#ifndef Corridor_H
#define Corridor_H

class Corridor
{ 
public:
  Corridor();
  Corridor(const int&); //int&?
  int getID();
  void setID(const int&);
  int getPreviousCorridorID();
private:
  int _corridorID;
  int _previousCorridorID;
};
#endif