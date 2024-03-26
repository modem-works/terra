/*
The i2c scanner code checks for connected i2c devices. 
Copyright (c) 2018 Luis Llamas (www.luisllamas.es) Licensed under the Apache License, Version 2.0.
*/

#include "I2CScanner.h"
#include <Wire.h>

I2CScanner scanner;

//if you use ESP8266-01 with not default SDA and SCL pins, define these 2 lines, else delete them
// #define SDA_PIN 6
// #define SCL_PIN 7

void setup() {
  //uncomment the next line if you use custom sda and scl pins for example with ESP8266-01 (sda = 4, scl = 5)
  // Wire.begin(SDA_PIN, SCL_PIN);

  Serial.begin(9600);
  while (!Serial) {};

  scanner.Init();
}

void loop() {
  scanner.Scan();
  delay(5000);
}
