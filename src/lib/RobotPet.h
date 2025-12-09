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
  } currentState;

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
    currentState = Default;
    setDefaultState();
  }

  void enterHappyState()
  {
    currentState = Happy;
    roboEyes.setMood(HAPPY);
    roboEyes.setWidth(32, 32);
    roboEyes.setHeight(36, 36);
    roboEyes.setBorderradius(8, 8);
    roboEyes.anim_laugh();
    roboEyes.setIdleMode(OFF);
    roboEyes.setAutoblinker(OFF);
    Serial.print("CurrentState: Happy");
    Serial.println();
  }
  void enterLongHappyState()
  {
    currentState = LongHappy;
    roboEyes.setMood(HAPPY);
    roboEyes.setWidth(32, 32);
    roboEyes.setHeight(36, 36);
    roboEyes.setBorderradius(8, 8);
    roboEyes.setVFlicker(ON, 5);
    roboEyes.setIdleMode(OFF);
    roboEyes.setAutoblinker(OFF);
    Serial.print("CurrentState: Happy");
    Serial.println();
  }
  void enterScaredState()
  {
    currentState = Scared;
    roboEyes.setMood(TIRED);
    roboEyes.setWidth(32, 32);
    roboEyes.setHeight(36, 36);
    roboEyes.setBorderradius(8, 8);
    roboEyes.setPosition(DEFAULT);
    roboEyes.setSweat(ON);
    roboEyes.setVFlicker(ON, 3);
    roboEyes.setHFlicker(ON, 3);
    roboEyes.setAutoblinker(OFF);
    Serial.print("CurrentState: Scared");
    Serial.println();
  }

  void enterScareState()
  {
    currentState = Scare;
    roboEyes.setMood(TIRED);
    roboEyes.setWidth(32, 32);
    roboEyes.setHeight(36, 36);
    roboEyes.setBorderradius(8, 8);
    roboEyes.setPosition(DEFAULT);
    roboEyes.setSweat(OFF);
    roboEyes.setVFlicker(ON, 3);
    roboEyes.setHFlicker(ON, 3);
    roboEyes.setAutoblinker(OFF);
    Serial.print("CurrentState: Scare");
    Serial.println();
  }

  void enterCuriosityState()
  {
    currentState = Curiosity;
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
    Serial.print("CurrentState: Curiosity");
    Serial.println();
  }

  void enterSleepyState()
  {
    currentState = Sleepy;
    roboEyes.setMood(TIRED);
    roboEyes.setHeight(20, 20);
    roboEyes.setPosition(DEFAULT);
    roboEyes.setSweat(OFF);
    roboEyes.setAutoblinker(ON, 2, 2);
    roboEyes.setIdleMode(OFF);
    Serial.print("CurrentState: Sleepy");
    Serial.println();
  }

  void enterAsleepState()
  {
    currentState = Asleep;
    roboEyes.setMood(DEFAULT);
    roboEyes.setPosition(S);
    roboEyes.setHeight(3, 3);
    roboEyes.setAutoblinker(OFF);
    roboEyes.setIdleMode(OFF);
    roboEyes.setBorderradius(0, 0);
    Serial.print("CurrentState: Asleep");
    Serial.println();
  }

  void enterAngryState()
  {
    currentState = Angry;
    roboEyes.setMood(ANGRY);
    roboEyes.setWidth(32, 32);
    roboEyes.setHeight(36, 36);
    roboEyes.setBorderradius(8, 8);
    roboEyes.setPosition(DEFAULT);
    roboEyes.setHFlicker(ON, 2);
    Serial.print("CurrentState: Angry");
    Serial.println();
  }

public:
  RobotPet(Adafruit_SSD1306 &disp, SoundPlayer &buzzer, MotorManager &mtr, int width, int heigh, int delay)
      : display(disp), roboEyes(disp), melody(buzzer), motor(mtr), screenWidth(width), screenHeight(heigh), refreshDelay(delay), currentState(Default), lastActionTime(0) {}

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
    unsigned long now = millis();
    unsigned long elapsed = now - lastActionTime;
    unsigned long motorElapsed = now - lastMotorActionTime;

    switch (currentState)
    {
    case Default:
      if (elapsed >= IDLE_DELAY)
      {
        unsigned int randomChoice = random(1, 11);
        if (randomChoice > 8)
        {
          enterSleepyState();
          lastActionTime = now;
        }
        else
        {
          enterCuriosityState();
          lastActionTime = now;
        }
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
        {
          enterSleepyState();
          lastActionTime = now;
        }
        else
        {
          enterDefaultState();
          lastActionTime = now;
        }
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
        {
          enterSleepyState();
          lastActionTime = now;
        }
        else
        {
          enterDefaultState();
          lastActionTime = now;
        }
      }
      break;
    }

    roboEyes.update();
  }
  void shortClick(int clickCount)
  {
    Serial.print("Click: ");
    Serial.println(clickCount);
    if ((currentState == Asleep || currentState == Sleepy))
    {
      enterAngryState();
      motor.backward();
      delay(75);
      motor.stop();
      lastActionTime = millis();
    }
    if (clickCount == 1)
    {
      if ((currentState == Default || currentState == Curiosity))
      {
        unsigned int randomChoice = random(1, 11);
        if (randomChoice > 7)
        {
          enterAngryState();
          motor.backward();
          delay(75);
          motor.stop();
          lastActionTime = millis();
        }
        else
        {
          enterHappyState();
          lastActionTime = millis();
        }
      }
    }
    if (clickCount == 4)
    {
      if ((currentState != Scared))
      {
        enterScaredState();
        lastActionTime = millis();
      }
    }
  }

  void longClick()
  {
    Serial.println("LONG PRESS!");
    if ((currentState == Default || currentState == Curiosity))
    {
      enterLongHappyState();
      lastActionTime = millis();
    }
  }

  void longClickRelease()
  {
    Serial.println("LONG PRESS RELEASE!");
    currentState = Happy;
  }

  void randomMotorMovement()
  {
    unsigned int motorChoice = random(1, 5);
    switch (motorChoice)
    {
    case 1:
      motor.forward();
      delay(75);
      motor.stop();
      break;
    case 2:
      motor.backward();
      delay(75);
      motor.stop();
      break;
    case 3:
      motor.left();
      delay(75);
      motor.stop();
      break;
    case 4:
      motor.right();
      delay(75);
      motor.stop();
      break;
    }
  }
};

#endif
