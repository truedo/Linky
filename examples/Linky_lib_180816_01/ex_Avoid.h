#include "Linky.h"

void Avoid(int speed);

void Avoid(int speed)
{
  int sensorFL = analogRead(SFL);
  int sensorFF = analogRead(SFF);
  int sensorFR = analogRead(SFR);

  if (sensorFF < 400)
  {
    DCMove(backward, speed);
    delay(150);
    DCMove(left, speed);
    delay(250);
  }
  else if (sensorFL < 400)  DCMove(right, speed);
  else if (sensorFR < 400)  DCMove(left, speed);
  else   DCMove(forward, speed);
}

