#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "lib/SoundPlayer.h"
#include "lib/RobotPet.h"
#include "lib/MotorManager.h"
#include "lib/ButtonManager.h"
#include "lib/BLEManager.h"
#include "lib/MediaVisualizer.h"
#include "lib/NotificationManager.h"
#include <ArduinoJson.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define I2C_SDA_PIN 6
#define I2C_SCL_PIN 7

#define BUZZER_PIN 8
#define BUTTON_PIN 10

#define MOTOR_IN1 0
#define MOTOR_IN2 1
#define MOTOR_IN3 2
#define MOTOR_IN4 3

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
MotorManager motor(MOTOR_IN1, MOTOR_IN2, MOTOR_IN3, MOTOR_IN4);
SoundPlayer melody(BUZZER_PIN);
ButtonManager button(BUTTON_PIN);
BLEManager ble;
RobotPet robotPet(display, melody, motor, SCREEN_WIDTH, SCREEN_HEIGHT, 100);
MediaVisualizer visualizer(display, FPS_30);
NotificationManager notification(display, 15000);

enum CurrentState
{
  Animation,
  Media,
  Notification
};
CurrentState currentState = Animation;
CurrentState previousState = Animation;

unsigned long lastMediaActive = 0;
static const unsigned long MEDIA_TIMEOUT = 5000;
static const float AUDIO_THRESHOLD = 0.01f;

void handleBLEMessage(String message);
void scanI2C();
void switchState(CurrentState newState);
void updateCurrentState();

void setup()
{
  Serial.begin(115200);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(100000);
  scanI2C();

  button.begin();
  button.addClickCallback([](int count)
                          { 
    if (currentState == Animation) robotPet.shortClick(count); });
  button.addLongPressCallback([]()
                              { 
    if (currentState == Animation) robotPet.longClick(); });
  button.addLongPressReleaseCallback([]()
                                     { 
    if (currentState == Animation) robotPet.longClickRelease(); });

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println("[Display] Failed!");
    while (1)
      ;
  }

  robotPet.begin();
  visualizer.begin();
  notification.begin();

  ble.setOnMessageCallback([](String message)
                           { handleBLEMessage(message); });
  ble.setOnConnectCallback([]()
                           { Serial.println("[BLE] Connected"); });
  ble.setOnDisconnectCallback([]()
                              {
    Serial.println("[BLE] Disconnected");
    switchState(Animation); });

  ble.begin("PetRobot-c3");
  display.clearDisplay();
  display.display();
}

void loop()
{
  button.update();
  updateCurrentState();
}

void switchState(CurrentState newState)
{
  if (currentState == newState)
    return;

  if (newState != Notification)
  {
    previousState = currentState;
  }

  if (currentState == Animation)
  {
    robotPet.isRunning = false;
  }
  else if (currentState == Media)
  {
    visualizer.stop();
  }

  Serial.print("[State] ");
  Serial.print(currentState == Animation ? "Animation" : currentState == Media ? "Media"
                                                                               : "Notification");
  Serial.print(" -> ");
  Serial.println(newState == Animation ? "Animation" : newState == Media ? "Media"
                                                                         : "Notification");

  currentState = newState;

  if (currentState == Animation)
  {
    robotPet.isRunning = true;
  }
  else if (currentState == Media)
  {
    lastMediaActive = millis();
  }
}

void updateCurrentState()
{
  unsigned long now = millis();
  unsigned long elapsedMediaActive =
      (now >= lastMediaActive) ? (now - lastMediaActive) : 0;
  switch (currentState)
  {
  case Animation:
    robotPet.update();
    break;

  case Media:
    visualizer.update();

    if (elapsedMediaActive > MEDIA_TIMEOUT)
    {
      Serial.println("[Media] Timeout - no audio detected");
      switchState(Animation);
    }
    break;

  case Notification:
    notification.update();

    if (notification.isExpired())
    {
      Serial.println("[Notification] Expired");
      switchState(previousState);
    }
    break;
  }
}

void scanI2C()
{
  Serial.println("[I2C] Scanning...");
  for (byte addr = 1; addr < 127; addr++)
  {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0)
    {
      Serial.print("[I2C] Found: 0x");
      Serial.println(addr, HEX);
    }
  }
}

void handleBLEMessage(String message)
{

  if (message.startsWith("\"") && message.endsWith("\""))
  {
    message = message.substring(1, message.length() - 1);
  }
  message.replace("\\\"", "\"");

  JsonDocument doc;
  if (deserializeJson(doc, message))
  {
    Serial.println("[JSON] Parse error");
    return;
  }

  const char *type = doc["type"] | "";

  if (strcmp(type, "notification") == 0)
  {
    if (currentState != Notification)
    {
      previousState = currentState;
    }
    switchState(Notification);
    notification.show(doc);
    return;
  }

  if (strcmp(type, "media") == 0)
  {

    if (currentState == Notification)
    {
      previousState = Media;
      return;
    }

    if (doc["audio_amplitude"].is<JsonObject>())
    {
      float amplitude = doc["audio_amplitude"]["amplitude"] | 0.0f;

      if (amplitude > AUDIO_THRESHOLD)
      {

        if (currentState != Media)
        {
          switchState(Media);
        }

        visualizer.handleMediaData(doc);
        lastMediaActive = millis();
      }
    }

    return;
  }

  if (currentState == Animation)
  {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.println("Message:");
    display.println(message.substring(0, 100));
    display.display();
  }
}