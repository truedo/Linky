#include "Linky.h"

void LineTracer(int speed);

void LineTracer(int speed)
{
  delay(300);

  int  base_L = 1023;
  int  base_R = 1023;

  for (int i = 0; i < 100; i++)
  {
    base_L  = min(analogRead(SBL), base_L);
    base_R  = min(analogRead(SBR), base_R);
  }

  base_L += 50;
  base_R += 50;

  EEPROM.write(EEP_LINE_SEN_BASE_LH, (base_L >> 8) & 0xff);
  EEPROM.write(EEP_LINE_SEN_BASE_LL, base_L & 0xff);

  EEPROM.write(EEP_LINE_SEN_BASE_RH, (base_R >> 8) & 0xff);
  EEPROM.write(EEP_LINE_SEN_BASE_RL, base_R & 0xff);

  // Sound_Beep2();

  int melody[] = {1702, 593, 1243};
  int tempo[] = {12, 12, 8};
  SoundProcess(melody, tempo, 3);

  while (1)
  {
    int sensorBL = analogRead(SBL);
    int sensorBR = analogRead(SBR);

    if (sensorBL > base_L && sensorBR > base_R)   DCMove(forward, speed);
    else if (sensorBL < base_L && sensorBR > base_R)    DCMove(right, speed);
    else if (sensorBL > base_L && sensorBR < base_R)   DCMove(left, speed);
    else if (sensorBL > base_L)   DCMove(left, speed);
    else if (sensorBR > base_R)   DCMove(right, speed);
  }
}
