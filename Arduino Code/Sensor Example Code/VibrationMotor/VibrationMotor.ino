/* Working Example for DRV2605 vibration motor driver, using Adafruit's DRV2605 Library.

Check out page 55 of the datasheet for the full list of effects
https://cdn-shop.adafruit.com/datasheets/DRV2605.pdf

Last edit: 23 Feb 2024.
*/

#include <Wire.h>
#include "Adafruit_DRV2605.h"

Adafruit_DRV2605 drv;
int effectNumber = 58;

void setup() {
  Serial.begin(9600);
  Serial.println("Adafruit DRV2605 Basic test");
  if (!drv.begin()) {
    Serial.println("Could not find DRV2605");
    while (1) delay(10);
  }

  drv.selectLibrary(1);
  // I2C trigger by sending 'go' command
  // default, internal trigger when sending GO command
  drv.setMode(DRV2605_MODE_INTTRIG);
}


void loop() {
  // set the effect to play
  drv.setWaveform(0, effectNumber);  // play effect - for effect list check table here: https://learn.adafruit.com/assets/72593
  drv.setWaveform(1, 0);             // end waveform

  // play the effect!
  drv.go();

  // wait a bit
  delay(500);
}
