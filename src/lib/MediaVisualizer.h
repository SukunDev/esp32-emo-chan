#ifndef MEDIA_VISUALIZER_H
#define MEDIA_VISUALIZER_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>

class MediaVisualizer
{
private:
  Adafruit_SSD1306 &display;

  // Media metadata
  String mediaTitle;
  String mediaArtist;
  String mediaStatus;
  bool isPlaying;

  // Audio amplitude data
  float currentAmplitude;
  float peakValue;
  float rmsValue;

  // Visualizer bars
  static const int NUM_BARS = 16;
  float barHeights[NUM_BARS];
  float barTargets[NUM_BARS];
  float barVelocities[NUM_BARS];

  // Peak indicators
  int peakPositions[NUM_BARS];
  unsigned long peakTimers[NUM_BARS];

  // Scrolling text
  int titleScrollPos;
  int artistScrollPos;
  unsigned long lastScrollTime;
  static const unsigned long SCROLL_DELAY = 100;

  // Display dimensions
  const int SCREEN_WIDTH = 128;
  const int SCREEN_HEIGHT = 64;
  const int METADATA_HEIGHT = 20;
  const int VISUALIZER_HEIGHT = 44;
  const int VISUALIZER_Y_START = 20;
  const int TEXT_MARGIN_LEFT = 12; // Margin for play/pause icon

  // Animation
  unsigned long lastUpdateTime;
  static const unsigned long UPDATE_INTERVAL = 33; // ~30fps

  unsigned long lastAmplitudeReceived;
  static const unsigned long AMPLITUDE_TIMEOUT = 500; // 500ms timeout

  bool isActive;

  void drawMetadata()
  {
    // Draw status indicator (play/pause icon) FIRST - at the left
    if (isPlaying)
    {
      // Play triangle
      display.fillTriangle(2, 2, 2, 8, 7, 5, SSD1306_WHITE);
    }
    else
    {
      // Pause bars
      display.fillRect(2, 2, 2, 6, SSD1306_WHITE);
      display.fillRect(6, 2, 2, 6, SSD1306_WHITE);
    }

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setTextWrap(false);

    // Calculate available width for text (screen width - margins)
    int availableWidth = SCREEN_WIDTH - TEXT_MARGIN_LEFT - 2;

    // Title - skip if Unknown or empty
    if (mediaTitle.length() > 0 && mediaTitle != "Unknown")
    {
      String displayTitle = mediaTitle;
      int titleWidth = displayTitle.length() * 6;

      if (titleWidth > availableWidth)
      {
        // Scrolling title - continuous loop
        int loopWidth = titleWidth + 18; // text + 3 spaces
        int xPos = TEXT_MARGIN_LEFT - (titleScrollPos % loopWidth);

        // Always draw at least 2 instances for seamless loop
        display.setCursor(xPos, 0);
        display.print(displayTitle + "   ");

        display.setCursor(xPos + loopWidth, 0);
        display.print(displayTitle + "   ");

        // Clear the icon area to prevent text overlap
        display.fillRect(0, 0, TEXT_MARGIN_LEFT - 1, 9, SSD1306_BLACK);

        // Redraw icon on top
        if (isPlaying)
        {
          display.fillTriangle(2, 2, 2, 8, 7, 5, SSD1306_WHITE);
        }
        else
        {
          display.fillRect(2, 2, 2, 6, SSD1306_WHITE);
          display.fillRect(6, 2, 2, 6, SSD1306_WHITE);
        }
      }
      else
      {
        // Centered title with margin
        int centerPos = TEXT_MARGIN_LEFT + (availableWidth - titleWidth) / 2;
        display.setCursor(centerPos, 0);
        display.print(displayTitle);
      }
    }

    // Artist - skip if Unknown or empty
    if (mediaArtist.length() > 0 && mediaArtist != "Unknown")
    {
      int artistWidth = mediaArtist.length() * 6;
      if (artistWidth > availableWidth)
      {
        // Scrolling artist - continuous loop
        int loopWidth = artistWidth + 18; // text + 3 spaces
        int xPos = TEXT_MARGIN_LEFT - (artistScrollPos % loopWidth);

        // Always draw at least 2 instances for seamless loop
        display.setCursor(xPos, 10);
        display.print(mediaArtist + "   ");

        display.setCursor(xPos + loopWidth, 10);
        display.print(mediaArtist + "   ");

        // Clear the icon area to prevent text overlap
        display.fillRect(0, 10, TEXT_MARGIN_LEFT - 1, 9, SSD1306_BLACK);
      }
      else
      {
        // Centered artist with margin
        int centerPos = TEXT_MARGIN_LEFT + (availableWidth - artistWidth) / 2;
        display.setCursor(centerPos, 10);
        display.print(mediaArtist);
      }
    }

    display.setTextWrap(true);
  }

  void drawVisualizer()
  {
    int barWidth = SCREEN_WIDTH / NUM_BARS;
    int spacing = 1;
    int actualBarWidth = barWidth - spacing;

    for (int i = 0; i < NUM_BARS; i++)
    {
      int x = i * barWidth;

      // Smooth bar animation
      float diff = barTargets[i] - barHeights[i];
      barVelocities[i] = barVelocities[i] * 0.7 + diff * 0.3;
      barHeights[i] += barVelocities[i];

      // Clamp values
      if (barHeights[i] < 0)
        barHeights[i] = 0;
      if (barHeights[i] > VISUALIZER_HEIGHT)
        barHeights[i] = VISUALIZER_HEIGHT;

      int barHeight = (int)barHeights[i];
      int y = VISUALIZER_Y_START + (VISUALIZER_HEIGHT - barHeight);

      // Draw bar
      if (barHeight > 0)
      {
        display.fillRect(x, y, actualBarWidth, barHeight, SSD1306_WHITE);
      }

      // Draw peak indicator
      if (peakPositions[i] > 0)
      {
        int peakY = VISUALIZER_Y_START + (VISUALIZER_HEIGHT - peakPositions[i]);
        display.drawFastHLine(x, peakY, actualBarWidth, SSD1306_WHITE);

        // Peak decay
        if (millis() - peakTimers[i] > 50)
        {
          peakPositions[i]--;
          peakTimers[i] = millis();
        }
      }

      // Update peak if current bar is higher
      if (barHeight > peakPositions[i])
      {
        peakPositions[i] = barHeight;
        peakTimers[i] = millis();
      }
    }
  }

  void updateScrolling()
  {
    unsigned long now = millis();
    if (now - lastScrollTime >= SCROLL_DELAY)
    {
      // Calculate available width for text
      int availableWidth = SCREEN_WIDTH - TEXT_MARGIN_LEFT - 2;

      // Scroll title - increment continuously, modulo handled in drawMetadata
      int titleWidth = mediaTitle.length() * 6;
      if (titleWidth > availableWidth)
      {
        titleScrollPos += 2;
        // No reset needed, modulo in drawMetadata handles the loop
      }

      // Scroll artist - increment continuously, modulo handled in drawMetadata
      int artistWidth = mediaArtist.length() * 6;
      if (artistWidth > availableWidth)
      {
        artistScrollPos += 2;
        // No reset needed, modulo in drawMetadata handles the loop
      }

      lastScrollTime = now;
    }
  }

  void generateBarTargets()
  {
    // Create smooth wave pattern from amplitude
    float baseHeight = currentAmplitude * VISUALIZER_HEIGHT;

    for (int i = 0; i < NUM_BARS; i++)
    {
      // Create wave effect
      float phase = (float)i / NUM_BARS * 3.14159 * 2;
      float wave = sin(phase + millis() * 0.002) * 0.3 + 1.0;

      // Add some randomness for natural look
      float randomFactor = (random(80, 120) / 100.0);

      // Calculate target height
      float target = baseHeight * wave * randomFactor;

      // Apply peak influence
      if (peakValue > currentAmplitude)
      {
        target += (peakValue - currentAmplitude) * VISUALIZER_HEIGHT * 0.5;
      }

      barTargets[i] = constrain(target, 0, VISUALIZER_HEIGHT);
    }
  }

public:
  MediaVisualizer(Adafruit_SSD1306 &disp)
      : display(disp),
        currentAmplitude(0),
        peakValue(0),
        rmsValue(0),
        isPlaying(false),
        titleScrollPos(0),
        artistScrollPos(0),
        lastScrollTime(0),
        lastUpdateTime(0),
        lastAmplitudeReceived(0),
        isActive(false)
  {
    // Initialize arrays
    for (int i = 0; i < NUM_BARS; i++)
    {
      barHeights[i] = 0;
      barTargets[i] = 0;
      barVelocities[i] = 0;
      peakPositions[i] = 0;
      peakTimers[i] = 0;
    }
  }

  void begin()
  {
    mediaTitle = "";
    mediaArtist = "";
    mediaStatus = "";
    isPlaying = false;
    isActive = false;
  }

  void handleMediaData(JsonDocument &doc)
  {
    const char *type = doc["type"];

    if (strcmp(type, "media") == 0)
    {
      // Get new title and artist
      String newTitle = doc["title"] | "";
      String newArtist = doc["artist"] | "";

      // Only reset scroll if title or artist actually changed
      bool titleChanged = (newTitle != mediaTitle);
      bool artistChanged = (newArtist != mediaArtist);

      // Update media metadata
      mediaTitle = newTitle;
      mediaArtist = newArtist;
      mediaStatus = doc["status"] | "";
      isPlaying = doc["is_playing"] | false;

      // Reset scroll positions only when media actually changes
      if (titleChanged)
      {
        titleScrollPos = 0;
        Serial.println("Title changed, reset scroll");
      }

      if (artistChanged)
      {
        artistScrollPos = 0;
        Serial.println("Artist changed, reset scroll");
      }

      // Check if audio_amplitude object exists
      bool hasAmplitude = false;
      if (doc["audio_amplitude"].is<JsonObject>())
      {
        JsonObject audioAmp = doc["audio_amplitude"];
        currentAmplitude = audioAmp["amplitude"] | 0.0f;
        peakValue = audioAmp["peak"] | 0.0f;
        rmsValue = audioAmp["rms"] | 0.0f;

        // Update last received time when we get amplitude data
        lastAmplitudeReceived = millis();

        hasAmplitude = (currentAmplitude > 0.0f || peakValue > 0.0f);

        Serial.print("Audio amplitude: ");
        Serial.print(currentAmplitude);
        Serial.print(", peak: ");
        Serial.print(peakValue);
        Serial.print(", rms: ");
        Serial.println(rmsValue);
      }

      // Activate visualizer if there's audio amplitude data
      // OR if there's valid media info (not all Unknown)
      bool hasValidMedia = (mediaTitle.length() > 0 && mediaTitle != "Unknown") ||
                           (mediaArtist.length() > 0 && mediaArtist != "Unknown");

      if (hasAmplitude || hasValidMedia)
      {
        isActive = true;
      }

      // Only log when media actually changes
      if (titleChanged || artistChanged)
      {
        Serial.println("Media updated:");
        Serial.println("  Title: " + (mediaTitle.length() > 0 ? mediaTitle : "(empty)"));
        Serial.println("  Artist: " + (mediaArtist.length() > 0 ? mediaArtist : "(empty)"));
        Serial.println("  Status: " + mediaStatus);
        Serial.println("  Is Active: " + String(isActive ? "YES" : "NO"));
      }
    }
  }

  void update()
  {
    if (!isActive)
      return;

    unsigned long now = millis();

    // Check if amplitude data has timed out
    if (now - lastAmplitudeReceived > AMPLITUDE_TIMEOUT)
    {
      // No amplitude data received recently, fade to zero
      currentAmplitude = 0;
      peakValue = 0;
      rmsValue = 0;
    }

    if (now - lastUpdateTime >= UPDATE_INTERVAL)
    {
      display.clearDisplay();

      // Draw metadata section
      drawMetadata();

      // Draw separator line
      display.drawFastHLine(0, 19, SCREEN_WIDTH, SSD1306_WHITE);

      // Generate and draw visualizer based on audio amplitude
      // Show visualizer if there's any amplitude, regardless of is_playing status
      if (currentAmplitude > 0.01 || peakValue > 0.01)
      {
        generateBarTargets();
      }
      else if (isPlaying)
      {
        // Only generate when playing if no amplitude data
        generateBarTargets();
      }
      else
      {
        // Fade out when paused AND no amplitude
        for (int i = 0; i < NUM_BARS; i++)
        {
          barTargets[i] *= 0.95;
        }
      }

      drawVisualizer();

      // Update scrolling text
      updateScrolling();

      display.display();
      lastUpdateTime = now;
    }
  }

  void stop()
  {
    isActive = false;
    display.clearDisplay();
    display.display();
  }

  bool isVisualizerActive()
  {
    return isActive;
  }

  void setAmplitude(float amplitude)
  {
    currentAmplitude = constrain(amplitude, 0.0f, 1.0f);
  }

  void setPeak(float peak)
  {
    peakValue = constrain(peak, 0.0f, 1.0f);
  }
};

#endif