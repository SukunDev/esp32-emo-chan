#include <Arduino.h>
#include "MotorManager.h"

MotorManager::MotorManager(int m1, int m2, int m3, int m4)
    : in1(m1), in2(m2), in3(m3), in4(m4) {}

void MotorManager::begin()
{
  Serial.println("Init Motor");
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  stop();
}

void MotorManager::stop()
{
  Serial.println("Motor Stop");
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}

void MotorManager::forward()
{
  Serial.println("Forward");
  // Motor A backward
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);

  // Motor B backward
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
}

void MotorManager::backward()
{
  Serial.println("Motor Backward");
  // Motor A forward
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);

  // Motor B forward
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

void MotorManager::right()
{
  Serial.println("Motor Right");
  // Motor A backward (or stop)
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);

  // Motor B forward
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

void MotorManager::left()
{
  Serial.println("Motor Left");
  // Motor A forward
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);

  // Motor B backward (or stop)
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
}