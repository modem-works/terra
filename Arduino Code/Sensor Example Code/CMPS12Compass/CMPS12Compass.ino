/* Working Example for  CMPS12 I2C Compass module. Code originally written by James Henderson, 2014.
Last edit: 23 Feb 2024.

This example code prints the sensor data (8 and 16 bit heading, magnetometer, accelerometer and gyro)

The example code has been modified to compensate for calibration with an iPhone compass.
When the sensor is lined up with a separate compass pointing North and the values do not match 0, 
the value displayed is input as the "offset" for calibration.

The cardinal directions have also been added as variables.

*/

#include <Wire.h>

#define CMPS12_ADDRESS 0x60  // Address of CMPS12 shifted right one bit for arduino wire library
#define ANGLE_8 1            // Register to read 8bit angle from

// Offset angle discovered during calibration between iPhone compass and the sensor.

// const int offset = 335;

unsigned char high_byte, low_byte, angle8;
char pitch, roll;
unsigned int angle16;

void setup() {
  Serial.begin(115200);  // Start serial port
  Wire.begin();
}

void loop() {

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

  Serial.print("roll: ");               // Display roll data
  Serial.print(roll, DEC);

  Serial.print("    pitch: ");          // Display pitch data
  Serial.print(pitch, DEC);

  Serial.print("    angle 8: ");        // Display 8bit angle
  Serial.print(angle8, DEC);

  Serial.print("    angle 16: ");     // Display 16 bit angle with decimal place
  Serial.print(angle16 / 10, DEC);
  Serial.print(".");
  Serial.print(angle16 % 10, DEC);

  // // Apply the offset and adjust for wrap-around
  // int calibratedAngle16 = (angle16 - (offset * 10) + 3600) % 3600;

  // // Display calibrated angle
  // Serial.print("    calibrated angle: ");
  // Serial.print(calibratedAngle16 / 10, DEC);
  // Serial.print(".");
  // Serial.print(calibratedAngle16 % 10, DEC);

  // Display cardinal direction 
  Serial.print("    cardinal direction: ");
  // Serial.println(getCardinalDirection(calibratedAngle16 / 10));
  Serial.println(getCardinalDirection(angle16 / 10));

  // delay for readibility
  delay (100);

}

String getCardinalDirection(int angle) {
    const String directions[] = {"N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE",
                                 "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"};

    // Divide the angle by 22.5 since each direction covers 22.5 degrees
    // Add 0.5 to handle rounding to the nearest direction
    int val = (int)((angle / 22.5) + 0.5);

    // Use modulus 16 to wrap around the array index
    return directions[val % 16];
}
