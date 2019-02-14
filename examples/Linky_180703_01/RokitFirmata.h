#include "config.h"

#define VERSION             0b00000011
#define SERIAL_BAUDRATE     57600

#define DCMOTOR               0x0D
#define LED_COLOR             0x0E
#define BUZZER                0x0F
#define STREAM_ON             0x0B
#define STREAM_OFF            0x0C
#define SENSOR                0x0D
#define VOLT                  0x0E
#define ETC                   0x0F
#define LINETRACING           0x10
#define READY                 0xE0
#define OFF                   0xD0

#define  EEP_LINE_SEN_BASE_LH  21
#define  EEP_LINE_SEN_BASE_LL  22

#define  EEP_LINE_SEN_BASE_RH  23
#define  EEP_LINE_SEN_BASE_RL  24

int packetStep = 0; 
byte recvBytes[5]={0};
byte bufferBytes[20][6];
byte IRSensor[5];
byte voltage[2];
byte etc[4];
byte tempBuf[10] = {};
int sensorVal = 0;
int cnt = 0;
int col = 0;
int follow = 0;
long exMillis = 0;
long exLTMillis = 0;
boolean isReceiving = false;
boolean isReady = true;
boolean isStreaming = true;
boolean isLinetracing = false;
byte exInByte = 0;
volatile unsigned int pitchData = 0;
volatile unsigned int dur = 0; 
int baseLV_L = 950;
int baseLV_R = 950;
int LTSpeed = 0; 


void serialRead();
void sendPacket(byte type);
void linetracing(int speed);
//-----------------------------------------//

int soundCount = 0;
int soundCheck = 0;
int soundMn = 1024;
int soundMx = 0;

//-----------------------------------------//

void RokitFiramata()
{
  //  Serial.begin(SERIAL_BAUDRATE);
  //  initialize();

  LEDColorR( 0x20);
  LEDColorG( 0x20);
  LEDColorB( 0x20);
  Sound_FireBall();
  LEDColorR( 0x00);
  LEDColorG( 0x00);
  LEDColorB( 0x00);

  byte i = EEPROM.read(EEP_LINE_SEN_BASE_LH);
  byte j = EEPROM.read(EEP_LINE_SEN_BASE_LL);
  byte k = EEPROM.read(EEP_LINE_SEN_BASE_RH);
  byte l = EEPROM.read(EEP_LINE_SEN_BASE_RL);
  
  baseLV_L = ((i << 8) | (j & 0xff));
  baseLV_R = ((k << 8) | (l & 0xff));

  for (int j = 0; j < 20; j++)
  {
    bufferBytes[j][5] = OFF;
  }

  while (1)
  {
    //---------------------------------------------//
    if (soundCount++ < 3500)
    {
      int val = analogRead(MIC_PIN);
      soundMn = min(soundMn, val);
      soundMx = max(soundMx, val);
    }
    else
    {
      soundCheck = soundMx - soundMn;
      soundCount = 0;
      soundMn = 1024;
      soundMx = 0;
    }
    //---------------------------------------------//

    serialRead();

    if (isReady == true)
    {
      switch (bufferBytes[follow][1])
      {
        case DCMOTOR:
          {
            DCMotor(bufferBytes[follow][2], bufferBytes[follow][3], bufferBytes[follow][4]);
            isLinetracing = false;
            break;
          }
        case LED_COLOR:
          {
            LEDColorR( bufferBytes[follow][2]);
            LEDColorG( bufferBytes[follow][3]);
            LEDColorB( bufferBytes[follow][4]);
            break;
          }
        case BUZZER:
          {
            pitchData = ((unsigned int)bufferBytes[follow][3] << 8) | bufferBytes[follow][2];
            dur =  bufferBytes[follow][4] * 2;
            //Serial.print("pitch: "); Serial.print(pitchData); Serial.print(" duration: "); Serial.println(dur);
            tone(4, pitchData, dur);
            break;
          }
        case LINETRACING: 
          {  
            LTSpeed =  bufferBytes[follow][2];
            if(LTSpeed > 0)  {isLinetracing = true; exLTMillis = millis(); }              
            else  { isLinetracing = false; DCMove(stop, LTSpeed); }                       
          }
        case STREAM_ON:
          {
            isStreaming = true;
            break;
          }
        case STREAM_OFF:
          {
            isStreaming = false;
            break;
          }
      }
      bufferBytes[follow][5] = OFF;

      follow++;
      if (follow >= 20) follow = 0;
      if (bufferBytes[follow][5] == OFF) isReady = false;  else isReady = true;
    }
    else
    {
      for (int j = 0; j < 20; j++)  {
        if (bufferBytes[j][5] == READY) {
          isReady = true;
          follow = j;
          break;
        }
      }
    }

    if ((millis() - exMillis) >= 40 )
    {
      if(packetStep == 0)                 //ADC
      {
        for (int j = 0; j < 5; j++) {
        sensorVal = analogRead(j);
        unsigned char k = map(sensorVal, 0, 1023, 0, 255);
        IRSensor[j] = k;       
        }
        sendPacket(SENSOR);     
      }
      else if(packetStep == 1)
      {
        uint16_t volt = ReadVoltage() * 100;
        voltage[0] = (byte)((volt & 0xFF00) >> 8);
        voltage[1] = (byte)(volt & 0xFF);
        sendPacket(VOLT);    
      }
      else if(packetStep == 2)
      {
        int mic = ReadMic();
        unsigned char k = map(mic, 0, 1023, 0, 255);
        etc[0] = k;
        etc[1] = (byte)(TVRemoconData());
        etc[2] = 'L'; etc[3] = VERSION;  
        sendPacket(ETC);    
      }    

      if(isLinetracing == true) linetracing(LTSpeed); 
      packetStep++;
      if(packetStep >= 3) packetStep = 0;
      exMillis = millis();   
    }
  }
}

void linetracing(int speed) {
  if((millis() - exLTMillis) < 200) {
    int sensorBL = analogRead(A3);
    int sensorBR = analogRead(A4);

    if (sensorBL > baseLV_L && sensorBR > baseLV_R)   DCMove(forward, speed);
    else if (sensorBL < baseLV_L && sensorBR > baseLV_R)  {
      //DCMove(right, speed);
      DCMotor(M1, CCW, speed);  DCMotor(M2, STOP, speed); 
    }
    else if (sensorBL > baseLV_L && sensorBR < baseLV_R)  {
      //DCMove(left, speed);
      DCMotor(M1, STOP, speed);  DCMotor(M2, CW, speed); 
    }
    else if (sensorBL > baseLV_L)   { 
      //DCMove(left, speed);
      DCMotor(M1, STOP, speed);  DCMotor(M2, CW, speed); 
    }
    else if (sensorBR > baseLV_R)   { 
      //DCMove(right, speed);  
      DCMotor(M1, CCW, speed);  DCMotor(M2, STOP, speed);     
    }
  } 
}

void sendPacket(byte type) {
 
  tempBuf[0] = 0xFF; tempBuf[1] = 0xFF;
  if(type == SENSOR) {
    for(int j = 0; j < sizeof(IRSensor); j++) { tempBuf[j+4] = IRSensor[j]; }
    tempBuf[2] = sizeof(IRSensor) + 1;
    tempBuf[3] = SENSOR;
    for(int i = 0; i < (tempBuf[2] + 3); i++) { Serial.write(tempBuf[i]); }     
  }
  else if(type == VOLT) {
    for(int j = 0; j < sizeof(voltage); j++) { tempBuf[j+4] = voltage[j]; }
    tempBuf[2] = sizeof(voltage) + 1;
    tempBuf[3] = VOLT;
    for(int i = 0; i < (tempBuf[2] + 3); i++) { Serial.write(tempBuf[i]); } 
  }
  else if(type == ETC) {
    for(int j = 0; j < sizeof(etc); j++) { tempBuf[j+4] = etc[j]; }
    tempBuf[2] = sizeof(etc) + 1;
    tempBuf[3] = ETC;
    for(int i = 0; i < (tempBuf[2] + 3); i++) { Serial.write(tempBuf[i]); }     
  }
}

//FF FF dataNumber command parameter..
void serialRead() {
  
  while (Serial.available()) {

    byte inByte = (byte)Serial.read();
    //Serial.print(inByte, HEX);
    if (isReceiving == true)
    {
      recvBytes[cnt] =  inByte;
      //Serial.print(recvBytes[cnt]);
      cnt++;

      if (recvBytes[0] < cnt)
      {
        isReady = true; isReceiving = false; cnt = 0;
        for (int j = 0; j < 5; j++)
        {
          bufferBytes[col][j] = recvBytes[j];
        }
        bufferBytes[col][5] = READY; col++;
        if (col >= 20) col = 0;
      }
    }
    if ((inByte == 0xFF) && (exInByte == 0xFF)) {
      isReceiving = true;
      exInByte = 0;
    }
    else {
      exInByte = inByte;
    }
  }
}



