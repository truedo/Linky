#include "Linky.h"

void EscapeMaze(int speed);

void EscapeMaze(int speed)
{
  int _senLvL = 300;
  int _senLvC = 300;
  int _senLvR = 300;

  byte senL = 0, senC = 0, senR = 0;

  int sensorFL = analogRead(SFL);
  int sensorFF = analogRead(SFF);
  int sensorFR = analogRead(SFR);

  if (sensorFL < _senLvL) senL = 1;
  if (sensorFF < _senLvC) senC = 1;
  if (sensorFR < _senLvR) senR = 1;

  if ((senC))
  {
    if ((senL) && (senR))
    {
      if (sensorFL > sensorFR)
      {
        while (analogRead(SFF) < _senLvC) DCMove(left, speed);
      }
      else if (sensorFL < sensorFR)
      {
        while (analogRead(SFF) < _senLvC)   DCMove(right, speed);
      }
    }
    else if (senL)
    {
      while (analogRead(SFF) < _senLvC)   DCMove(right, speed);
    }
    else if (senR)
    {
      while (analogRead(SFF) < _senLvC) DCMove(left, speed);
    }
  }

  else if (!(senL) && !(senR))    DCMove(forward, speed);

  else if (senL)
  {
    DCMotor(M1, CCW, speed );
    DCMotor(M2, CW, speed - 10);
  }

  else if (senR)
  {
    DCMotor(M1, CCW, speed - 10);
    DCMotor(M2, CW, speed );
  }
}

