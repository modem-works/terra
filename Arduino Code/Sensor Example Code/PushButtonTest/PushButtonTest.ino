/*
  Working Example for testing Adafruit's momentary powerswitch.

  When a HIGH signal is sent to the powerswitch on pin 32, the power is switched OFF.
  When the button on the powerswitch is pressed, this turns the power on again.

  Last edit: 23 Feb 2024.

  This code needs to be tested when not plugged into a computer, as the power from the 
  USB will override the result of the powerswitch.
*/

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin 32 of the ESP32 as an output.
  pinMode(32, OUTPUT);

  // wait for 2 seconds before proceeding
  delay (3000);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(32, HIGH);  // turn the LED on (HIGH is the voltage level)
}
