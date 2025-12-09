#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <Arduino.h>
#include <functional>
#include <vector>

class ButtonManager
{
private:
  int pin;
  bool lastState;
  unsigned long lastChangeTime;
  unsigned long buttonDownTime;
  int clickCount;
  bool longPressFired;

  const unsigned long debounceDelay = 50;
  const unsigned long multiClickDelay = 300;
  const unsigned long longPressTime = 800;

  // MULTI CALLBACKS
  std::vector<std::function<void(int)>> clickCallbacks;
  std::vector<std::function<void()>> longPressCallbacks;

public:
  ButtonManager(int pin);
  void begin();
  void update();

  // ADD CALLBACK (multi listener)
  void addClickCallback(std::function<void(int)> cb);
  void addLongPressCallback(std::function<void()> cb);

private:
  // INTERNAL DISPATCHER
  void onClick(int count);
  void onLongPress();
};

#endif
