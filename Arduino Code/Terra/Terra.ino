// Latest working code as of March 26 2024.

bool debugMode = true;

// Include Libraries
#include <TinyGPSPlus.h>
#include <TFT_eSPI.h>
#include <Wire.h>
#include <map>
#include "Adafruit_DRV2605.h"

// Include Configurations and Images
#include "config.h"     // Assumes this contains GPS waypoints and number of stops
#include "imagelist.h"  // Assumes this includes declarations for images

// Sensor and Display Setup
#define CMPS12_ADDRESS 0x60
#define ANGLE_8 1
unsigned char high_byte, low_byte, angle8;
char pitch, roll;
unsigned int angle16;
#define BACKLIGHT_PIN 12

// TFT Display Settings
#define logoWidth 240
#define logoHeight 240
TFT_eSPI tft = TFT_eSPI();

// The TinyGPSPlus object
TinyGPSPlus gps;

// Vibration motor
Adafruit_DRV2605 drv;
int effectNumber = 58;
bool proximityVibrationTriggered = false;

// Thresholds for triggers in meters
int checkpointTrigger = 10;  // Distance from the checkpoint that is considered an arrival
int vibrationTrigger = 20;   // When to trigger vibration to indicate the checkpoint is getting close

unsigned long lastVibrationTime = 0;   // Tracks the last time vibration was triggered
int proximityVibrationDelayMs = 500;   // To determine the time between vibrations when close to the current stop.

// Image Types Enum
enum ImageType {
  NONE,
  PENDING,
  GOTOSTART,
  ARROW_N,
  ARROW_NNE,
  ARROW_NE,
  ARROW_ENE,
  ARROW_E,
  ARROW_ESE,
  ARROW_SE,
  ARROW_SSE,
  ARROW_S,
  ARROW_SSW,
  ARROW_SW,
  ARROW_WSW,
  ARROW_W,
  ARROW_WNW,
  ARROW_NW,
  ARROW_NNW,
  CHECKPOINT_1,
  CHECKPOINT_2,
  CHECKPOINT_3,
  CHECKPOINT_4,
  CHECKPOINT_5,
  CHECKPOINT_6,
  CHECKPOINT_7,
  CHECKPOINT_8,
  CHECKPOINT_9,
  CHECKPOINT_10
};
ImageType currentDisplayedImage = NONE;

enum NavigationState {
  NOT_STARTED,
  NAVIGATING,
  AT_CHECKPOINT,
  TRAIL_ENDED
};

// Initialize your navigation state
NavigationState navigationState = NOT_STARTED;  // Initial navigation state
unsigned long lastCheckpointTime = 0;           // Timestamp of when the last checkpoint was reached

// Global Variables for GPS Data and State
int currentStop = 0;
double currentLat, currentLon;
bool trailStarted = false;
bool dataReceived = false;
bool screenOn = false;
double distance;  // Distance to the next checkpoint

// Function Prototypes
void setup();
void loop();
void handleGPSData();
bool readSerialGPS();
bool readGPS();
void determineTrailStatusAndNavigate();
void processGPSData(double lat, double lon);
bool nonBlockingDelay(unsigned long ms);
void displayImage(ImageType image);
void fadeOut();
void fadeIn();
double getDistanceTo(double lat, double lon);
String getCardinalTo(double lat, double lon);
int getCourseTo(double lat, double lon);
int readCompass();

// Setup Function
void setup() {
  Serial.begin(115200);

  if (!debugMode) {
    Serial1.begin(9600);
  }

  // Screen
  tft.init();
  tft.setRotation(1);
  pinMode(BACKLIGHT_PIN, OUTPUT);
  digitalWrite(BACKLIGHT_PIN, LOW);


  // Vibration motor
  if (!drv.begin()) {
    Serial.println("Could not find DRV2605");
    while (1) delay(10);
  }

  drv.selectLibrary(1);
  // I2C trigger by sending 'go' command
  // default, internal trigger when sending GO command
  drv.setMode(DRV2605_MODE_INTTRIG);

  Wire.begin();
}

// Main Loop
void loop() {
  if (nonBlockingDelay(1000)) {
    handleGPSData();
    determineTrailStatusAndNavigate();
  }
  if (!debugMode) {
    smartDelay(1000);  // for reading the GPS module
  }
  // Continuously trigger vibration when within a certain distance of the next stop
  if (proximityVibrationTriggered && millis() - lastVibrationTime >= proximityVibrationDelayMs) {
    triggerProximityVibration();
    lastVibrationTime = millis();  // Update the last vibration time
  }
}

// This custom version of delay() ensures that the gps object is being "fed".
static void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (Serial1.available())
      gps.encode(Serial1.read());
  } while (millis() - start < ms);
}

// Handle GPS Data
void handleGPSData() {
  if (debugMode) {  // If we're in debug mode, read the serial input
    if (Serial.available()) {
      if (readSerialGPS()) {
        dataReceived = true;  // Once the first data has arrived, we can get into our start routine
      }
    }
  } else {  // If we're not in serial input mode, start handling real GPS data
    while (Serial1.available()) {
      if (readGPS()) {
        dataReceived = true;
      }
    }
  }
}

// Read the GPS signal for current lat/lon - this is a serial input for now but can be replaced with code to read the GPS module
bool readSerialGPS() {
  String inputString = Serial.readStringUntil('\n');
  int commaIndex = inputString.indexOf(',');

  // Check if the comma exists and it's not at the end of the string
  if (commaIndex != -1 && commaIndex < inputString.length() - 1) {
    currentLat = inputString.substring(0, commaIndex).toDouble();
    currentLon = inputString.substring(commaIndex + 1).toDouble();
    return true;  // Data was successfully parsed
  } else {
    Serial.println("Invalid format. Please enter in format: lat,lon");
    return false;  // Data parsing failed
  }
}

// Read the GPS module signal for current lat/lon
bool readGPS() {
  if (gps.location.isValid()) {
    currentLat = gps.location.lat();
    currentLon = gps.location.lng();
    return true;
  }
  return false;
}

void determineTrailStatusAndNavigate() {
  static double lastDistance = -1;

  // Obtain the target coordinates based on the trail's current status
  double targetLat = !trailStarted ? startLat : stopLats[currentStop - 1];
  double targetLon = !trailStarted ? startLon : stopLons[currentStop - 1];

  // Calculate the current distance and direction to the target
  double distance = getDistanceTo(targetLat, targetLon);
  String cardinal = getCardinalTo(targetLat, targetLon);
  int targetAngle = getCourseTo(targetLat, targetLon);  // Assume implementation exists
  int currentAngle = readCompass();

  // Calculate the relative direction for navigation
  int relativeDirection = calculateRelativeDirection(currentAngle, targetAngle);

  if (navigationState == NOT_STARTED) {
    if (!dataReceived) {
      displayImage(PENDING);  // Show PENDING only when waiting for the first GPS data
    } else {
      Serial.println("Please proceed to the start of the trail.");
      displayImage(GOTOSTART);  // Now we're sure we've received data, show GOTOSTART
    }

    // Transition to navigating state once within close range to the start and data has been received
    if (dataReceived && distance <= checkpointTrigger) {
      trailStarted = true;
      currentStop = 1;
      Serial.println("Trail started. Heading to Stop 1.");
      navigationState = NAVIGATING;
    }
    return;  // Continue to skip rest of the function logic when NOT_STARTED
  }

  // Only update navigation arrow if we are in the navigating phase
  if (navigationState == NAVIGATING) {
    ImageType arrowImage = selectArrowImage(relativeDirection);
    displayImage(arrowImage);
  }

  // Start of the trail
  if (!trailStarted) {
    if (distance <= checkpointTrigger) {  // "Closeness" threshold
      trailStarted = true;
      currentStop = 1;  // Moving towards the first checkpoint
      Serial.println("Trail started. Heading to Stop 1.");
      navigationState = NAVIGATING;
    } else {
      Serial.println("Please proceed to the start of the trail.");
      if (dataReceived) {
        displayImage(GOTOSTART);  // Indicating to go to the starting point
      }
    }
  }
  // Navigating the trail
  else {
    if (distance <= checkpointTrigger && currentStop <= numberOfStops) {
      if (navigationState != AT_CHECKPOINT) {
        // Just arrived at this checkpoint
        Serial.print("Arrived at Stop ");
        Serial.println(currentStop);
        ImageType checkpointImage = static_cast<ImageType>(CHECKPOINT_1 + currentStop - 1);
        displayImage(checkpointImage);    // Show the checkpoint image
        navigationState = AT_CHECKPOINT;  // Update state to at checkpoint
        lastCheckpointTime = millis();    // Capture the time we arrived at the checkpoint

        // Check if this is the final stop
        if (currentStop == numberOfStops) {
          Serial.println("Final stop reached. Trail is complete.");
          // Display PENDING image to indicate completion
          displayImage(PENDING);
          // Optionally, you might want to change the navigation state or take other actions here
          navigationState = TRAIL_ENDED;  // Resetting the state to NOT_STARTED or another appropriate state
        }

        proximityVibrationTriggered = false;  // Allow vibration to trigger again for the next stop
        currentStop++;                        // Prepare for the next stop or complete the trail
      }
    } else if (navigationState == AT_CHECKPOINT) {
      if (millis() - lastCheckpointTime > 5000) {  // 5 seconds have passed since arriving at the checkpoint
        navigationState = NAVIGATING;              // Transition back to navigating after the delay
        proximityVibrationTriggered = false;       // Reset vibration trigger flag
      }
    } else if (navigationState == NAVIGATING && currentStop <= numberOfStops) {
      // Continue with the condition to update navigation info only if there's a significant change in distance
      if (abs(lastDistance - distance) > 0.5) {
        Serial.print("Distance to next stop: ");
        Serial.print(distance, 1);  // One decimal place for distance
        Serial.print(" meters. Direction to next stop: ");
        Serial.print(cardinal);
        Serial.print(" (Target angle: ");
        Serial.print(targetAngle);
        Serial.println(" degrees)");
        lastDistance = distance;  // Update lastDistance for next comparison

        // Here, potentially display the arrow again if needed, based on your logic for selecting and displaying arrows
        ImageType arrowImage = selectArrowImage(relativeDirection);
        displayImage(arrowImage);

        // Set the flag to start continuous vibration when within a certain distance from the next stop
        if (distance <= vibrationTrigger && !proximityVibrationTriggered) {
          proximityVibrationTriggered = true;
          lastVibrationTime = millis();  // Ensure we start timing from now
        }
      }
    }
  }
}

void triggerProximityVibration() {
  // Check if we should still be vibrating (if within a certain distance and navigating)
  if (distance <= proximityVibrationTriggered && navigationState == NAVIGATING) {
    // set the effect to play
    drv.setWaveform(0, effectNumber);  // play effect
    drv.setWaveform(1, 0);             // end waveform
    // play the effect!
    drv.go();
    // Serial.println("Vibration triggered");
    // Note: We do not set vibrationTriggered to false here as we want continuous vibration
  } else {
    // Stop vibrating if no longer within 20 meters or not in navigating state
    proximityVibrationTriggered = false;
  }
}

// Display image function - fades images out then in again
void displayImage(ImageType image) {
  if (currentDisplayedImage != image) {
    if (screenOn) {
      fadeOut();
    }
    tft.fillScreen(TFT_BLACK);  // This line is common to all cases
    switch (image) {
      case PENDING:
        drawBitmap(pending);
        Serial.println("Drawn pending.h");
        break;
      case GOTOSTART:
        drawBitmap(gotostart);
        Serial.println("Drawn gotostart.h");
        break;
      case ARROW_N:
        drawBitmap(arrow_N);
        Serial.println("Drawn arrow_N.h");
        break;
      case ARROW_NNE:
        drawBitmap(arrow_NNE);
        Serial.println("Drawn arrow_NNE.h");
        break;
      case ARROW_NE:
        drawBitmap(arrow_NE);
        Serial.println("Drawn arrow_NE.h");
        break;
      case ARROW_ENE:
        drawBitmap(arrow_ENE);
        Serial.println("Drawn arrow_ENE.h");
        break;
      case ARROW_E:
        drawBitmap(arrow_E);
        Serial.println("Drawn arrow_E.h");
        break;
      case ARROW_ESE:
        drawBitmap(arrow_ESE);
        Serial.println("Drawn arrow_ESE.h");
        break;
      case ARROW_SE:
        drawBitmap(arrow_SE);
        Serial.println("Drawn arrow_SE.h");
        break;
      case ARROW_SSE:
        drawBitmap(arrow_SSE);
        Serial.println("Drawn arrow_SSE.h");
        break;
      case ARROW_S:
        drawBitmap(arrow_S);
        Serial.println("Drawn arrow_S.h");
        break;
      case ARROW_SSW:
        drawBitmap(arrow_SSW);
        Serial.println("Drawn arrow_SSW.h");
        break;
      case ARROW_SW:
        drawBitmap(arrow_SW);
        Serial.println("Drawn arrow_SW.h");
        break;
      case ARROW_WSW:
        drawBitmap(arrow_WSW);
        Serial.println("Drawn arrow_WSW.h");
        break;
      case ARROW_W:
        drawBitmap(arrow_W);
        Serial.println("Drawn arrow_W.h");
        break;
      case ARROW_WNW:
        drawBitmap(arrow_WNW);
        Serial.println("Drawn arrow_WNW.h");
        break;
      case ARROW_NW:
        drawBitmap(arrow_NW);
        Serial.println("Drawn arrow_NW.h");
        break;
      case ARROW_NNW:
        drawBitmap(arrow_NNW);
        Serial.println("Drawn arrow_NNW.h");
        break;
      case CHECKPOINT_1:
        drawBitmap(checkpoint_1);
        Serial.println("Drawn checkpoint_1.h");
        break;
      case CHECKPOINT_2:
        drawBitmap(checkpoint_2);
        Serial.println("Drawn checkpoint_2.h");
        break;
      case CHECKPOINT_3:
        drawBitmap(checkpoint_3);
        Serial.println("Drawn checkpoint_3.h");
        break;
      case CHECKPOINT_4:
        drawBitmap(checkpoint_4);
        Serial.println("Drawn checkpoint_4.h");
        break;
      case CHECKPOINT_5:
        drawBitmap(checkpoint_5);
        Serial.println("Drawn checkpoint_5.h");
        break;
      case CHECKPOINT_6:
        drawBitmap(checkpoint_6);
        Serial.println("Drawn checkpoint_6.h");
        break;
      case CHECKPOINT_7:
        drawBitmap(checkpoint_7);
        Serial.println("Drawn checkpoint_7.h");
        break;
      case CHECKPOINT_8:
        drawBitmap(checkpoint_8);
        Serial.println("Drawn checkpoint_8.h");
        break;
      case CHECKPOINT_9:
        drawBitmap(checkpoint_9);
        Serial.println("Drawn checkpoint_9.h");
        break;
      case CHECKPOINT_10:
        drawBitmap(checkpoint_10);
        Serial.println("Drawn checkpoint_10.h");
        break;
      default:
        tft.fillScreen(TFT_BLACK);
        Serial.println("No image to display");
    }
    fadeIn();
    currentDisplayedImage = image;
  }
}

void drawBitmap(const unsigned char* bitmap) {
  tft.drawXBitmap(0, 0, bitmap, logoWidth, logoHeight, TFT_BLACK, TFT_WHITE);
}

// Example function to calculate relative direction based on angles
int calculateRelativeDirection(int currentAngle, int targetAngle) {
  int difference = targetAngle - currentAngle;
  if (difference < 0) {
    difference += 360;  // Adjust for negative differences
  }
  return difference % 360;  // Ensure the result is within 0-359 degreess
}


// Select an arrow image based on relative direction
ImageType selectArrowImage(int relativeDirection) {
  if (relativeDirection >= 348.75 || relativeDirection < 11.25) return ARROW_N;
  else if (relativeDirection >= 11.25 && relativeDirection < 33.75) return ARROW_NNE;
  else if (relativeDirection >= 33.75 && relativeDirection < 56.25) return ARROW_NE;
  else if (relativeDirection >= 56.25 && relativeDirection < 78.75) return ARROW_ENE;
  else if (relativeDirection >= 78.75 && relativeDirection < 101.25) return ARROW_E;
  else if (relativeDirection >= 101.25 && relativeDirection < 123.75) return ARROW_ESE;
  else if (relativeDirection >= 123.75 && relativeDirection < 146.25) return ARROW_SE;
  else if (relativeDirection >= 146.25 && relativeDirection < 168.75) return ARROW_SSE;
  else if (relativeDirection >= 168.75 && relativeDirection < 191.25) return ARROW_S;
  else if (relativeDirection >= 191.25 && relativeDirection < 213.75) return ARROW_SSW;
  else if (relativeDirection >= 213.75 && relativeDirection < 236.25) return ARROW_SW;
  else if (relativeDirection >= 236.25 && relativeDirection < 258.75) return ARROW_WSW;
  else if (relativeDirection >= 258.75 && relativeDirection < 281.25) return ARROW_W;
  else if (relativeDirection >= 281.25 && relativeDirection < 303.75) return ARROW_WNW;
  else if (relativeDirection >= 303.75 && relativeDirection < 326.25) return ARROW_NW;
  else if (relativeDirection >= 326.25 && relativeDirection < 348.75) return ARROW_NNW;
  else return ARROW_N;  // Default case, although logically unnecessary due to the first condition
}

// Non blocking delay to read sensors
bool nonBlockingDelay(unsigned long ms) {
  static unsigned long lastCheck = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - lastCheck >= ms) {
    lastCheck = currentMillis;
    return true;
  }
  return false;
}

// Simple function to get distance to lat/lon, is used to determine whether GPS module is in range of a stop
double getDistanceTo(double lat, double lon) {
  return TinyGPSPlus::distanceBetween(currentLat, currentLon, lat, lon);
}

String getCardinalTo(double lat, double lon) {
  double courseTo = TinyGPSPlus::courseTo(currentLat, currentLon, lat, lon);
  String cardinal = TinyGPSPlus::cardinal(courseTo);
  return cardinal;
}

int getCourseTo(double lat, double lon) {
  return TinyGPSPlus::courseTo(currentLat, currentLon, lat, lon);
}

// Read the compass sensor and return the compassDirection in cardinal points
int readCompass() {                        // is string if we're asking for cardinal.
  Wire.beginTransmission(CMPS12_ADDRESS);  // Starts communication with CMPS12
  Wire.write(ANGLE_8);                     // Sends the register we wish to start reading from
  Wire.endTransmission();

  // Request 5 bytes from the CMPS12
  // this will give us the 8 bit bearing,
  // both bytes of the 16 bit bearing, pitch and roll
  Wire.requestFrom(CMPS12_ADDRESS, 5);

  while (Wire.available() < 5)
    ;  // Wait for all bytes to come back

  angle8 = Wire.read();  // Read back the 5 bytes
  high_byte = Wire.read();
  low_byte = Wire.read();
  pitch = Wire.read();
  roll = Wire.read();

  angle16 = high_byte;  // Calculate 16 bit angle
  angle16 <<= 8;
  angle16 += low_byte;

  int currentAngle = angle16 / 10;
  return currentAngle;
}

void fadeOut() {
  for (int i = 255; i >= 0; i -= 5) {
    analogWrite(BACKLIGHT_PIN, i);
    delay(10);
  }
  screenOn = false;
}

void fadeIn() {
  for (int i = 0; i <= 255; i += 5) {
    analogWrite(BACKLIGHT_PIN, i);
    delay(10);
  }
  screenOn = true;
}
