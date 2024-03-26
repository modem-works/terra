/* Working Example for Beitan BN 880 GPS module, using TinyGPSPlus Library by Mikal Hart
Last edit: 23 Feb 2024.

If this is the first time using the GPS module for a while, it requires up to 30 
minutes to start up and find a satellite. 

GPS location is most effective when the module is outside, or near an open window.

The blue light on the GPS indicates data transmission. 
The red light indicates a satellite has been found. */

#include <TinyGPSPlus.h>

// Baud rate of the GPS module
static const uint32_t GPSBaud = 9600;

// The TinyGPSPlus object
TinyGPSPlus gps;

void setup() {
  // Initiate Arduino Serial for debugging
  Serial.begin(115200);
  // Initiate GPS Module Serial
  Serial1.begin(GPSBaud);

  Serial.print(F("Testing TinyGPSPlus library v. "));
  Serial.println(TinyGPSPlus::libraryVersion());
  Serial.println(F("by Mikal Hart"));
  Serial.println();
  Serial.println(F("Sats HDOP  Latitude   Longitude   Fix  Date       Time     Date Alt    Course Speed Card  Distance Course Card  Chars Sentences Checksum"));
  Serial.println(F("           (deg)      (deg)       Age                      Age  (m)    --- from GPS ----  ---- to Destination  ----  RX    RX        Fail"));
  Serial.println(F("----------------------------------------------------------------------------------------------------------------------------------------"));
}

void loop() {
  // Test Latitude and Longitude
  static const double DESTINATION_LAT =52.3731313, DESTINATION_LON = 4.8768349;

  Serial.print("Satellite value: ");
  printInt(gps.satellites.value(), gps.satellites.isValid(), 5);
  Serial.println(" ");

  Serial.print("GPS hdop: ");
  printFloat(gps.hdop.hdop(), gps.hdop.isValid(), 6, 1);
  Serial.println(" ");

  Serial.print("Current lat and Long: ");
  printFloat(gps.location.lat(), gps.location.isValid(), 11, 6);
  printFloat(gps.location.lng(), gps.location.isValid(), 12, 6);
  Serial.println(" ");

  Serial.print("Location age: ");
  printInt(gps.location.age(), gps.location.isValid(), 5);
  Serial.println(" ");

  Serial.print("GPS date + time: ");
  printDateTime(gps.date, gps.time);
  Serial.println(" ");

  Serial.print("Altitude: ");
  printFloat(gps.altitude.meters(), gps.altitude.isValid(), 7, 2);
  Serial.println(" ");

  Serial.print("Course deg: ");
  printFloat(gps.course.deg(), gps.course.isValid(), 7, 2);
  Serial.println(" ");

  Serial.print("Speed kmph value: ");
  printFloat(gps.speed.kmph(), gps.speed.isValid(), 6, 2);
  Serial.println(" ");

  Serial.print("GPS course: ");
  printStr(gps.course.isValid() ? TinyGPSPlus::cardinal(gps.course.deg()) : "*** ", 6);
  Serial.println(" ");

  unsigned long distanceKmToDest =
    (unsigned long)TinyGPSPlus::distanceBetween(
      gps.location.lat(),
      gps.location.lng(),
      DESTINATION_LAT,
      DESTINATION_LON)
    / 1000;
  Serial.print("Distance KM to Destination: ");
  printInt(distanceKmToDest, gps.location.isValid(), 9);
  Serial.println(" ");

  double courseToDest =
    TinyGPSPlus::courseTo(
      gps.location.lat(),
      gps.location.lng(),
      DESTINATION_LAT,
      DESTINATION_LON);

  Serial.print("Course to Destination: ");
  printFloat(courseToDest, gps.location.isValid(), 7, 2);
  Serial.println(" ");

  const char *cardinalToDest = TinyGPSPlus::cardinal(courseToDest);

  Serial.print("Cardinal to Destination: ");
  printStr(gps.location.isValid() ? cardinalToDest : "*** ", 6);
  printInt(gps.charsProcessed(), true, 6);
  printInt(gps.sentencesWithFix(), true, 10);
  printInt(gps.failedChecksum(), true, 9);
  Serial.println();

  smartDelay(1000);

  if (millis() > 5000 && gps.charsProcessed() < 10)
    Serial.println(F("No GPS data received: check wiring"));
}

// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (Serial1.available())
      gps.encode(Serial1.read());
  } while (millis() - start < ms);
}

static void printFloat(float val, bool valid, int len, int prec) {
  if (!valid) {
    while (len-- > 1)
      Serial.print('*');
    Serial.print(' ');
  } else {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1);  // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3
                           : vi >= 10  ? 2
                                       : 1;
    for (int i = flen; i < len; ++i)
      Serial.print(' ');
  }
  smartDelay(0);
}

static void printInt(unsigned long val, bool valid, int len) {
  char sz[32] = "*****************";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i = strlen(sz); i < len; ++i)
    sz[i] = ' ';
  if (len > 0)
    sz[len - 1] = ' ';
  Serial.print(sz);
  smartDelay(0);
}

static void printDateTime(TinyGPSDate &d, TinyGPSTime &t) {
  if (!d.isValid()) {
    Serial.print(F("********** "));
  } else {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
    Serial.print(sz);
  }

  if (!t.isValid()) {
    Serial.print(F("******** "));
  } else {
    char sz[32];
    sprintf(sz, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
    Serial.print(sz);
  }

  printInt(d.age(), d.isValid(), 5);
  smartDelay(0);
}

static void printStr(const char *str, int len) {
  int slen = strlen(str);
  for (int i = 0; i < len; ++i)
    Serial.print(i < slen ? str[i] : ' ');
  smartDelay(0);
}
