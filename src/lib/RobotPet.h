#ifndef ROBOT_PET_H
#define ROBOT_PET_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <lib/FluxGarage_RoboEyes.h>
#include "SoundPlayer.h"
#include "MotorManager.h"

class RobotPet
{
private:
  Adafruit_SSD1306 &display;
  RoboEyes<Adafruit_SSD1306> roboEyes;
  SoundPlayer &melody;
  MotorManager &motor;

  int screenWidth, screenHeight, refreshDelay;

  enum EyeState
  {
    Default,
    Happy,
    LongHappy,
    Scared,
    Scare,
    Curiosity,
    Sleepy,
    Asleep,
    Angry
  } currentEyeState;

  enum MotorState
  {
    Right,
    Left,
    Forward,
    Backward,
    Stop
  } currentMotorState;

  static const unsigned long IDLE_DELAY = 5000;
  static const unsigned long DEEP_SLEEP_DELAY = 10000;
  static const unsigned long ANGRY_DURATION = 5000;
  static const unsigned long HAPPY_DURATION = 500;
  static const unsigned long CURIOSITY_DURATION = 10000;
  static const unsigned long SCARE_DURATION = 4000;
  static const unsigned long SCARED_DURATION = 2000;

  unsigned long lastActionTime;
  unsigned long lastMotorActionTime;
  unsigned long randomMotorInterval;

  void setDefaultState()
  {
    roboEyes.setMood(DEFAULT);
    roboEyes.setWidth(30, 30);
    roboEyes.setHeight(36, 36);
    roboEyes.setSpacebetween(16);
    roboEyes.setBorderradius(6, 6);
    roboEyes.setPosition(DEFAULT);
    roboEyes.setAutoblinker(ON, 2, 2);
    roboEyes.setIdleMode(OFF);
    roboEyes.setHFlicker(OFF);
    roboEyes.setVFlicker(OFF);
    roboEyes.setSweat(OFF);
    roboEyes.setCuriosity(OFF);
  }

  void enterDefaultState()
  {
    currentEyeState = Default;
    setDefaultState();
  }

  void enterHappyState()
  {
    currentEyeState = Happy;
    roboEyes.setMood(HAPPY);
    roboEyes.setWidth(32, 32);
    roboEyes.setHeight(36, 36);
    roboEyes.setBorderradius(8, 8);
    roboEyes.anim_laugh();
    roboEyes.setIdleMode(OFF);
    roboEyes.setAutoblinker(OFF);
    Serial.println("CurrentState: Happy");
  }

  void enterLongHappyState()
  {
    currentEyeState = LongHappy;
    roboEyes.setMood(HAPPY);
    roboEyes.setWidth(32, 32);
    roboEyes.setHeight(36, 36);
    roboEyes.setBorderradius(8, 8);
    roboEyes.setVFlicker(ON, 5);
    roboEyes.setIdleMode(OFF);
    roboEyes.setAutoblinker(OFF);
    Serial.println("CurrentState: LongHappy");
  }

  void enterScaredState()
  {
    currentEyeState = Scared;
    roboEyes.setMood(TIRED);
    roboEyes.setWidth(32, 32);
    roboEyes.setHeight(36, 36);
    roboEyes.setBorderradius(8, 8);
    roboEyes.setPosition(DEFAULT);
    roboEyes.setSweat(ON);
    roboEyes.setVFlicker(ON, 3);
    roboEyes.setHFlicker(ON, 3);
    roboEyes.setAutoblinker(OFF);
    Serial.println("CurrentState: Scared");
  }

  void enterScareState()
  {
    currentEyeState = Scare;
    roboEyes.setMood(TIRED);
    roboEyes.setWidth(32, 32);
    roboEyes.setHeight(36, 36);
    roboEyes.setBorderradius(8, 8);
    roboEyes.setPosition(DEFAULT);
    roboEyes.setSweat(OFF);
    roboEyes.setVFlicker(ON, 3);
    roboEyes.setHFlicker(ON, 3);
    roboEyes.setAutoblinker(OFF);
    Serial.println("CurrentState: Scare");
  }

  void enterCuriosityState()
  {
    currentEyeState = Curiosity;
    roboEyes.setMood(DEFAULT);
    roboEyes.setWidth(32, 32);
    roboEyes.setHeight(36, 36);
    roboEyes.setBorderradius(8, 8);
    roboEyes.setPosition(DEFAULT);
    roboEyes.setAutoblinker(ON, 2, 2);
    roboEyes.setIdleMode(ON, 2, 2);
    roboEyes.setHFlicker(OFF);
    roboEyes.setSweat(OFF);
    roboEyes.setCuriosity(ON);
    Serial.println("CurrentState: Curiosity");
  }

  void enterSleepyState()
  {
    currentEyeState = Sleepy;
    roboEyes.setMood(TIRED);
    roboEyes.setHeight(20, 20);
    roboEyes.setPosition(DEFAULT);
    roboEyes.setSweat(OFF);
    roboEyes.setAutoblinker(ON, 2, 2);
    roboEyes.setIdleMode(OFF);
    Serial.println("CurrentState: Sleepy");
  }

  void enterAsleepState()
  {
    currentEyeState = Asleep;
    roboEyes.setMood(DEFAULT);
    roboEyes.setPosition(SOUTH);
    roboEyes.setHeight(3, 3);
    roboEyes.setAutoblinker(OFF);
    roboEyes.setIdleMode(OFF);
    roboEyes.setBorderradius(0, 0);
    Serial.println("CurrentState: Asleep");
  }

  void enterAngryState()
  {
    currentEyeState = Angry;
    roboEyes.setMood(ANGRY);
    roboEyes.setWidth(32, 32);
    roboEyes.setHeight(36, 36);
    roboEyes.setBorderradius(8, 8);
    roboEyes.setPosition(DEFAULT);
    roboEyes.setHFlicker(ON, 2);
    Serial.println("CurrentState: Angry");
  }

public:
  // === PUBLIC VARIABLE UNTUK STOP SEMUA AKSI ===
  bool isRunning = true;

  RobotPet(Adafruit_SSD1306 &disp, SoundPlayer &buzzer, MotorManager &mtr, int width, int heigh, int delay)
      : display(disp), roboEyes(disp), melody(buzzer), motor(mtr),
        screenWidth(width), screenHeight(heigh), refreshDelay(delay),
        currentEyeState(Default), lastActionTime(0) {}

  void begin()
  {
    motor.begin();
    melody.play("G4 100 20 C5 100 20 E5 100 20 G5 100 20 C6 100 20 D6 100 20 E6 200 200");
    roboEyes.begin(screenWidth, screenHeight, refreshDelay);
    setDefaultState();

    lastActionTime = millis();
  }

  void update()
  {
    // === GLOBAL STOP ===
    if (!isRunning)
    {
      return;
    }

    unsigned long now = millis();
    unsigned long elapsed = now - lastActionTime;
    unsigned long motorElapsed = now - lastMotorActionTime;

    switch (currentEyeState)
    {
    case Default:
      if (elapsed >= IDLE_DELAY)
      {
        unsigned int randomChoice = random(1, 11);
        if (randomChoice > 8)
          enterSleepyState();
        else
          enterCuriosityState();

        lastActionTime = now;
      }
      if (motorElapsed >= randomMotorInterval)
      {
        randomMotorMovement();
        randomMotorInterval = random(1600, 10000);
        lastMotorActionTime = now;
      }
      break;

    case Angry:
      if (elapsed >= ANGRY_DURATION)
      {
        enterDefaultState();
        lastActionTime = now;
      }
      break;

    case Scare:
      if (elapsed >= SCARE_DURATION)
      {
        enterDefaultState();
        lastActionTime = now;
      }
      break;

    case Scared:
      if (elapsed >= SCARED_DURATION)
      {
        enterScareState();
        lastActionTime = now;
      }
      break;

    case Happy:
      if (elapsed >= HAPPY_DURATION)
      {
        enterDefaultState();
        lastActionTime = now;
      }
      break;

    case Curiosity:
      if (elapsed >= CURIOSITY_DURATION)
      {
        unsigned int randomChoice = random(1, 11);
        if (randomChoice > 7)
          enterSleepyState();
        else
          enterDefaultState();

        lastActionTime = now;
      }
      if (motorElapsed >= randomMotorInterval)
      {
        randomMotorMovement();
        randomMotorInterval = random(3200, 10000);
        lastMotorActionTime = now;
      }
      break;

    case Sleepy:
      if (elapsed >= DEEP_SLEEP_DELAY)
      {
        enterAsleepState();
        lastActionTime = now;
      }
      break;

    case Asleep:
      if (elapsed >= DEEP_SLEEP_DELAY)
      {
        unsigned int randomChoice = random(1, 11);
        if (randomChoice > 8)
          enterSleepyState();
        else
          enterDefaultState();

        lastActionTime = now;
      }
      break;
    }

    roboEyes.update();
  }

  void shortClick(int clickCount)
  {
    if (!isRunning)
      return;

    Serial.print("Click: ");
    Serial.println(clickCount);

    if ((currentEyeState == Asleep || currentEyeState == Sleepy))
    {
      enterAngryState();
      motor.backward();
      delay(75);
      motor.stop();
      lastActionTime = millis();
    }

    if (clickCount == 1)
    {
      if ((currentEyeState == Default || currentEyeState == Curiosity))
      {
        unsigned int randomChoice = random(1, 11);
        if (randomChoice > 7)
        {
          enterAngryState();
          motor.backward();
          delay(75);
          motor.stop();
        }
        else
        {
          enterHappyState();
        }
        lastActionTime = millis();
      }
    }

    if (clickCount == 4)
    {
      if (currentEyeState != Scared)
      {
        enterScaredState();
        lastActionTime = millis();
      }
    }
  }

  void longClick()
  {
    if (!isRunning)
      return;

    Serial.println("LONG PRESS!");
    if (currentEyeState == Default || currentEyeState == Curiosity)
    {
      enterLongHappyState();
      lastActionTime = millis();
    }
  }

  void longClickRelease()
  {
    if (!isRunning)
      return;

    Serial.println("LONG PRESS RELEASE!");
    currentEyeState = Happy;
  }

  void randomMotorMovement()
  {
    if (!isRunning)
    {
      motor.stop();
      return;
    }

    unsigned int motorChoice = random(1, 5);
    switch (motorChoice)
    {
    case 1:
      motor.forward();
      break;
    case 2:
      motor.backward();
      break;
    case 3:
      motor.left();
      break;
    case 4:
      motor.right();
      break;
    }

    delay(75);
    motor.stop();
  }
};

#endif
