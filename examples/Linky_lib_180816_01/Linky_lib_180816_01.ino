/*------ Board Select ----------------------------
    Arduino Pro or Pro Mini
    Atmega328 (5V, 16Mhz)
  ------------------------------------------------*/
/*-------------------------------------------------
  tunning sensor -linetracer
  Remocon set channel (0~4) - tv remocon
  Low Battery check - 3.7v
  Speed Change => Remocon Mode
  ------------------------------------------------*/
#include "Linky.h"

//#include "config.h"

// examples
#include "ex_Avoid.h"
#include "ex_CliffBot.h"
#include "ex_Escapemaze.h"
#include "ex_LineTracer.h"
#include "ex_RemoconControl.h"

#include "RokitFirmata.h" // for Scratch & RokitBric
#include "ex_Unplugged.h" // for ex_Unplugged Coding

int mode = 0;
int basicSpeed = 0;

void setup()
{
  StartStep();
}

void loop()
{
  if (mode == AVOID)              Avoid(basicSpeed);       //  sensor Left

  else if (mode == MAZE_BOT)      EscapeMaze(basicSpeed);  //  sensor Left && sensor Right

  else if (mode == LINE_TRACER)   LineTracer(basicSpeed);  //  sensor Right
  else if (mode == UNPLUGGED)     Unplugged(basicSpeed);   //  sensor Front  (Unplugged & Remocon)

  int sound = ReadMic();      // Sensing clap sound
  float vin = ReadVoltage();  // Read Volatage

  if (vin < 3.7)  LEDColorR(100);       //  Low battery
  else if (sound > 600) LEDColorG(100); //  bright 0~100 ,100: always on
  else  LEDColorG(0); //0: off

  PrintSensor();
}

void StartStep()
{
  Serial.begin(57600);
  LowBatCheck();        //  Low Battery check - 3.7v
  initialize();
  
  //-------------------------------------------------------------------------------------------------------------------------------------------//
  if (!digitalRead(SW1) && !digitalRead(SW2))     TestMode(); // sw1, sw2 Push hold and Power On
  //-------------------------------------------------------------------------------------------------------------------------------------------//

  mode = ModeSelect();
  if (mode == FIRMATA)  RokitFiramata();
  basicSpeed = DCSpeedLoad();

  Sound_1up();
  LedDisplay(1, 100);
}
