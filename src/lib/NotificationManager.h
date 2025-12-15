#ifndef NOTIFICATION_MANAGER_H
#define NOTIFICATION_MANAGER_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>

class NotificationManager
{
private:
  Adafruit_SSD1306 &display;

  // NotificationManager data
  String appName;
  String timestamp;
  String textLines[5]; // Max 5 lines
  int lineCount;

  // Display settings
  const int SCREEN_WIDTH = 128;
  const int SCREEN_HEIGHT = 64;
  const int LINE_HEIGHT = 10;
  const int MAX_CHARS_PER_LINE = 21; // 128 / 6 pixels per char

  // Timing
  unsigned long notificationStartTime;
  unsigned long displayDuration;
  bool isActive;
  bool hasExpired;

  // Scrolling for long text
  int scrollPositions[5];
  unsigned long lastScrollTime;
  static const unsigned long SCROLL_DELAY = 100;

  // Icons
  void drawAppIcon(const char *app);
  void drawWhatsAppIcon();
  void drawTelegramIcon();
  void drawGmailIcon();
  void drawDefaultIcon();

  // Helper methods
  void resetScrollPositions();
  void updateScrolling();
  int getTextWidth(String text);

public:
  NotificationManager(Adafruit_SSD1306 &disp, unsigned long duration = 15000);

  void begin();
  void show(JsonDocument &doc);
  void show(const char *app, const char *time, JsonArray texts);
  void update();
  void dismiss();
  bool isExpired();
  bool isShowing();
  unsigned long getRemainingTime();
};

#endif