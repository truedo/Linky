#include "Linky.h"

void RemoconControl(int speed);

void RemoconControl(int speed)
{
  byte _speedOffsetL = EEPROM.read(EEP_MOTOR_OFFSET_L);
  byte _speedOffsetR = EEPROM.read(EEP_MOTOR_OFFSET_R);

  boolean _OffsetResetFlag = false;
  unsigned long OffsetResetMillis;

  Serial.println(speed);

  int melody[] = {NOTE_F5, NOTE_A5, NOTE_C6};
  int tempo[] = {8, 8, 8};
  SoundProcess(melody, tempo, 3);
  delay(200);

  while (1)
  {
    int keyData = RFreceive();
    if (keyData == 0) keyData =  TVRemoconData();

    //------------------------------------------------- DC CONTROL -----------------------------------------------------//

    if (keyData == RemoconCheckCode(KEY_U))       DCMove(forward, speed);
    else  if (keyData == RemoconCheckCode(KEY_L)) DCMove(left, speed);
    else  if (keyData == RemoconCheckCode(KEY_R)) DCMove(right, speed);
    else  if (keyData == RemoconCheckCode(KEY_D)) DCMove(backward, speed);
    else  DCMove(stop, 0);

    //----------------------------------------------- DC SPEED CHANGE ---------------------------------------------------//
    if ((keyData == RemoconCheckCode(KEY_F1)) || (keyData == RemoconCheckCode(KEY_F2)))
    {
      if ((keyData == RemoconCheckCode(KEY_F1)) &&  (speed < MX_SP))
      {
        speed += 10;
        if (speed > MX_SP)  speed = MX_SP;  //MX_SP = 100;
        EEPROM.write(EEP_MOTOR_SPEED, speed);
        tone(4, 750 + (speed * 5), 80);
      }

      else  if ((keyData == RemoconCheckCode(KEY_F2)) && (speed > 20))
      {
        speed -= 10;
        if (speed < 20)  speed = 20;
        EEPROM.write(EEP_MOTOR_SPEED, speed);
        tone(4, 750 + (speed * 5), 80);
      }

      Serial.print("Speed");  Serial.print("\t"); Serial.println(speed);
      delay(350);
    }

    //------------------------------------ DC OFFSET CALIBRATION ------------------------------------//
    //--------------------------------------------- M1 ----------------------------------------------//
    if ((keyData == RemoconCheckCode(KEY_CH1)) || (keyData == RemoconCheckCode(KEY_CH2))) // M1
    {
      if (((keyData == RemoconCheckCode(KEY_CH1)) && (_speedOffsetL < 100)) || ((keyData == RemoconCheckCode(KEY_CH2)) && (_speedOffsetL > 0)))
      {
        if ((keyData == RemoconCheckCode(KEY_CH1)) && (_speedOffsetL < 100))    DCOffsetInput(M1, _speedOffsetL++); // M1 Offset Up
        else if ((keyData == RemoconCheckCode(KEY_CH2)) && (_speedOffsetL > 0)) DCOffsetInput(M1, _speedOffsetL--); // M1 Offset Down
        EEPROM.write(EEP_MOTOR_OFFSET_L, _speedOffsetL);
      }
      tone(4, 1100, 60);
      delay(500);
      Serial.print("OffsetL");  Serial.print("\t"); Serial.println(_speedOffsetL);
    }
    //--------------------------------------------- M2 ----------------------------------------------//
    else if ((keyData == RemoconCheckCode(KEY_CH3)) || (keyData == RemoconCheckCode(KEY_CH4))) // M2
    {
      if (((keyData == RemoconCheckCode(KEY_CH3)) &&  (_speedOffsetR < 100)) || ((keyData == RemoconCheckCode(KEY_CH4)) && (_speedOffsetR > 0)))
      {
        if ((keyData == RemoconCheckCode(KEY_CH3)) &&  (_speedOffsetR < 100))   DCOffsetInput(M2, _speedOffsetR++); // M2 Offset up
        else if ((keyData == RemoconCheckCode(KEY_CH4)) && (_speedOffsetR > 0)) DCOffsetInput(M2, _speedOffsetR--); // M2 Offset Down
        EEPROM.write(EEP_MOTOR_OFFSET_R, _speedOffsetR);
      }
      tone(4, 1000, 60);
      delay(500);
      Serial.print("OffsetR");  Serial.print("\t"); Serial.println(_speedOffsetR);
    }

    //---------------------------------------- Offset Reset -----------------------------------------//

    if (keyData == RemoconCheckCode(KEY_F4))    //Offset Reset
    {
      if (_OffsetResetFlag == false)
      {
        _OffsetResetFlag = true;
        OffsetResetMillis = millis();
      }
      else if ((_OffsetResetFlag == true) && ((millis() > OffsetResetMillis + 1500)))
      {
        _OffsetResetFlag = false;
        _speedOffsetL = 50;
        _speedOffsetR = 50;
        DCOffsetInput(M1, _speedOffsetL);
        DCOffsetInput(M2, _speedOffsetR);
        EEPROM.write(EEP_MOTOR_OFFSET_L, _speedOffsetL);
        EEPROM.write(EEP_MOTOR_OFFSET_R, _speedOffsetR);
        int melody[] = {988, 1019};
        int tempo[] = {16, 4};
        SoundProcess(melody, tempo, 2);
        delay(500);
        Serial.println("Offset Reset");
      }
    }

    //----------------------------------------------------------------------------------------------//
  }
}

