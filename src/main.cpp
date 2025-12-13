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
MediaVisualizer visualizer(display);

bool isBLEConnected = false;
bool isVisualizerMode = false;
unsigned long lastVisualizerUpdate = 0;
static const unsigned long VISUALIZER_TIMEOUT = 15000;

void handleBLEMessage(String message);
void scanI2C();

void setup()
{
  Serial.begin(115200);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(100000);
  scanI2C();

  button.begin();
  button.addClickCallback([](int count)
                          { 
                            if (!isVisualizerMode) {
                              robotPet.shortClick(count); 
                            } });
  button.addLongPressCallback([]()
                              { 
                                if (!isVisualizerMode) {
                                  robotPet.longClick(); 
                                } });
  button.addLongPressReleaseCallback([]()
                                     { 
                                       if (!isVisualizerMode) {
                                         robotPet.longClickRelease(); 
                                       } });

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println("SSD1306 Failed");
    while (1)
      ;
  }

  robotPet.begin();
  visualizer.begin();

  ble.setOnMessageCallback([](String message)
                           { handleBLEMessage(message); });

  ble.setOnConnectCallback([]()
                           {
    Serial.println("[BLE] Connected!");
    isBLEConnected = true; });

  ble.setOnDisconnectCallback([]()
                              {
    Serial.println("[BLE] Disconnected!");
    isBLEConnected = false;
    isVisualizerMode = false;
    robotPet.isRunning = true; });

  ble.begin("PetRobot-c3");
  Serial.println("[BLE] BLE Manager initialized");

  display.clearDisplay();
  display.display();
}

void loop()
{
  unsigned long now = millis();
  unsigned long elapsedVisualizer =
      (now >= lastVisualizerUpdate) ? (now - lastVisualizerUpdate) : 0;

  button.update();

  if (isVisualizerMode)
  {

    visualizer.update();

    if (elapsedVisualizer >= VISUALIZER_TIMEOUT)
    {
      Serial.print("[Visualizer] ");
      Serial.print(elapsedVisualizer);
      Serial.print(" | ");
      Serial.print(VISUALIZER_TIMEOUT);
      Serial.print(" | ");
      Serial.print(elapsedVisualizer >= VISUALIZER_TIMEOUT);
      Serial.print(" | ");
      Serial.println("Timeout - returning to robot pet mode");
      isVisualizerMode = false;
      visualizer.stop();
      robotPet.isRunning = true;
    }
  }
  else
  {

    robotPet.update();
  }
}

void scanI2C()
{
  Serial.println("Scanning I2C devices...");
  for (byte address = 1; address < 127; address++)
  {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0)
    {
      Serial.print("Found device at 0x");
      Serial.println(address, HEX);
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
  DeserializationError err = deserializeJson(doc, message);

  if (err)
  {
    Serial.print("[JSON Error] ");
    Serial.println(err.c_str());
    return;
  }

  const char *type = doc["type"] | "";

  bool isMedia = strcmp(type, "media") == 0;

  if (isMedia)
  {

    if (!isVisualizerMode)
    {
      Serial.println("[Mode] Switching to Visualizer Mode");
      isVisualizerMode = true;
      robotPet.isRunning = false;
    }

    visualizer.handleMediaData(doc);

    lastVisualizerUpdate = millis();

    return;
  }

  if (!isVisualizerMode)
  {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.println(message);
    display.display();
  }
}