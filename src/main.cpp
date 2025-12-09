#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "lib/SoundPlayer.h"
#include "lib/RobotPet.h"
#include "lib/MotorManager.h"
#include "lib/ButtonManager.h"

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
RobotPet robotPet(display, melody, motor, SCREEN_WIDTH, SCREEN_HEIGHT, 100);

void scanI2C();

void setup()
{
  Serial.begin(115200);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(100000);
  scanI2C();

  button.begin();
  button.addClickCallback([](int count)
                          { robotPet.shortClick(count); });
  button.addLongPressCallback([]()
                              { robotPet.longClick(); });
  button.addLongPressReleaseCallback([]()
                                     { robotPet.longClickRelease(); });

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println("SSD1306 Failed");
    while (1)
      ;
  }
  robotPet.begin();

  display.clearDisplay();
  display.display();
}

void loop()
{
  robotPet.update();
  button.update();
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