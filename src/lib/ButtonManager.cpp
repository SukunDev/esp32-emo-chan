#include "ButtonManager.h"

ButtonManager::ButtonManager(int pin)
    : pin(pin),
      lastState(LOW),
      lastChangeTime(0),
      buttonDownTime(0),
      clickCount(0),
      longPressFired(false)
{
}

void ButtonManager::begin()
{
  pinMode(pin, INPUT_PULLUP);
}

void ButtonManager::addClickCallback(std::function<void(int)> cb)
{
  clickCallbacks.push_back(cb);
}

void ButtonManager::addLongPressCallback(std::function<void()> cb)
{
  longPressCallbacks.push_back(cb);
}

void ButtonManager::update()
{
  bool reading = digitalRead(pin);
  unsigned long now = millis();

  // Debounce change
  if (reading != lastState)
  {
    lastChangeTime = now;
    lastState = reading;
    longPressFired = false;

    if (reading == HIGH)
      buttonDownTime = now; // pressed
  }

  // Release detection (HIGH → LOW)
  if (reading == LOW && (now - lastChangeTime) > debounceDelay)
  {
    if (buttonDownTime > 0 && !longPressFired)
      clickCount++;

    buttonDownTime = 0;
  }

  // Long press
  if (reading == HIGH &&
      buttonDownTime > 0 &&
      !longPressFired &&
      (now - buttonDownTime >= longPressTime))
  {
    longPressFired = true;
    clickCount = 0;
    onLongPress();
  }

  // Multi-click
  if (clickCount > 0 &&
      (now - lastChangeTime) > multiClickDelay)
  {
    onClick(clickCount);
    clickCount = 0;
  }
}

void ButtonManager::onClick(int count)
{
  for (auto &cb : clickCallbacks)
    if (cb)
      cb(count);
}

void ButtonManager::onLongPress()
{
  for (auto &cb : longPressCallbacks)
    if (cb)
      cb();
}
