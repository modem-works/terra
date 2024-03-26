/* Working example to demonstrate the drawing of X BitMap (XBM) format image onto the TFT display.
Information on the X BitMap (XBM) format can be found here: https://en.wikipedia.org/wiki/X_BitMap
This is a modified code example from the TFT_eSPI library, created by Bodmer 23/04/18
Last edit: 23 Feb 2024.

This code displays an image, fades the screen up and down again before switching to a new image.

Guide to configuring images:
1. Resize your image to 240px x 240px
2. Convert to greyscale
3. Export as .bmp using a software like GIMP
4. Convert .bmp to XBM using https://www.online-utility.org/image/convert/to/XBM
5. Open XBM files in a code or text editor and copy the array (starts with 0xFF)
6. Paste the array into the template .h file, eg. "PROGMEM const unsigned char checkpoint_1[] = {}"
7. Save into a .h file within the sketch folder, eg. "checkpoint_1.h"

Make sure the names of the files are consistent across all of the code files.

*/

#include "checkpoint_1.h"     // Sketch tab header for xbm images
#include "checkpoint_2.h"  // Sketch tab header for xbm images
#include "checkpoint_3.h"    // Sketch tab header for xbm images
#include "checkpoint_4.h"    // Sketch tab header for xbm images
#include "checkpoint_5.h"    // Sketch tab header for xbm images

#include <TFT_eSPI.h>  // Hardware-specific library

int backlight = 12;
int brightness = 0;  // how bright the LED is
int fadeAmount = 5;  // how many points to fade the LED by

#define logoWidth 240   // logo width
#define logoHeight 240  // logo height

TFT_eSPI tft = TFT_eSPI();  // Invoke library

// Set X and Y coordinates where the image will be drawn
int x = 0;
int y = 0;

void setup() {
  pinMode(backlight, OUTPUT);
  tft.begin();                // Initialise the display
  tft.fillScreen(TFT_BLACK);  // Black screen fill
}

void loop() {
  fadeOut();
  tft.fillScreen(TFT_BLACK);  // Black screen fill
  tft.drawXBitmap(x, y, checkpoint_1, logoWidth, logoHeight, TFT_BLACK, TFT_WHITE);
  delay(100);
  fadeIn();

  delay(1000);

  fadeOut();
  tft.fillScreen(TFT_BLACK);  // Black screen fill
  tft.drawXBitmap(x, y, checkpoint_2, logoWidth, logoHeight, TFT_BLACK, TFT_WHITE);
  delay(100);
  fadeIn();

  delay(1000);

  fadeOut();
  tft.fillScreen(TFT_BLACK);  // Black screen fill
  tft.drawXBitmap(x, y, checkpoint_3, logoWidth, logoHeight, TFT_BLACK, TFT_WHITE);
  delay(100);
  fadeIn();

  delay(1000);

  fadeOut();
  tft.fillScreen(TFT_BLACK);  // Black screen fill
  tft.drawXBitmap(x, y, checkpoint_4, logoWidth, logoHeight, TFT_BLACK, TFT_WHITE);
  delay(100);
  fadeIn();

  delay(1000);
}

void fadeOut() {
  // fade out from max to min in increments of 5 points:
  for (int fadeValue = 255; fadeValue >= 0; fadeValue -= 5) {
    // sets the value (range from 0 to 255):
    analogWrite(backlight, fadeValue);
    // wait for 30 milliseconds to see the dimming effect
    delay(10);
  }
}

void fadeIn(){
  // fade in from min to max in increments of 5 points:
  for (int fadeValue = 0; fadeValue <= 255; fadeValue += 5) {
    // sets the value (range from 0 to 255):
    analogWrite(backlight, fadeValue);
    // wait for 30 milliseconds to see the dimming effect
    delay(10);
  }
}