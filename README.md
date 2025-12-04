<h1 align="center">TERRA is a companion for mindful wandering</h1>

<br> <!-- Additional spacing -->
<br> <!-- Additional spacing -->

<p align="center">
  <img src="https://modemworks.com/wp-content/uploads/2024/03/terra_device01.png" alt="TERRA Device">
</p>

<br> <!-- Additional spacing -->
<br> <!-- Additional spacing -->

##  Designed with the science of AI and the wisdom of mindfulness, TERRA is the incredible, pocket-sized compass that lets you wander without getting lost. Leave your phone behind and reconnect with your surroundings.


<br> <!-- Additional spacing -->

### Features
- **Improvised Trails, Powered by AI**: Every journey begins with a prompt. TERRA’s sophisticated AI translates your intentions, available time, and precise location into a tailored trail of GPS coordinates. Users don't know where TERRA will take them; the only certainty is finding their way back to the starting point.
- **Wander Without Your Phone**: TERRA invites users to leave their smartphones behind and reconnect with their surroundings. The standalone device boasts a minimalist interface, featuring a tranquil compass needle and gentle haptic feedback to ensure users remain engaged with the present moment.  
- **Open-Source by Design**: TERRA is designed with a DIY mindset. Anyone can download the open-source software, 3D print the CAD designs, assemble the necessary hardware, and start wandering.

<br> <!-- Additional spacing -->

<p align="center">
  ⚠️ <strong>NOTICE</strong>: This project is currently in an experimental phase and is undergoing rapid developments ⚠️
</p>

<br> <!-- Additional spacing -->


### Software

The TERRA App generates coordinates or checkpoints from a starting point and an ideal trail length using the ChatGPT API.

### Running TERRA App Locally

#### Prerequisites 
- `Node.js` v21.5
- `body-parser` v1.20.2
- `cors` v2.8.5
- `express` v4.19.0
- `node-fetch` v3.3.2

To run the TERRA App locally, follow these steps:

1. **Implement ChatGPT API Key:** Obtain a [ChatGPT API key](https://platform.openai.com/) and replace ```YOUR_OPENAI_API_KEY``` with it in the ```server.js``` file.
2. **Implement Google Maps API Key:** Obtain a [Google Places API key](https://developers.google.com/maps/documentation/places/web-service/overview) and replace ```YOUR_MAPS_KEY``` with it in the ```index.html``` file.
3. **Run Locally:** To run the application, use the command:
    ```
    node server.js
    ```
4. **Load Application:** Open `index.html` in a browser to use the app for generating a route.
5. **Download config.h File:** Download the `config.h` file which contains the generated route configuration.
6. **Update Arduino Code:** Replace the `config.h` file in the Arduino code directory with the newly downloaded file.

### Hardware


The hardware for the TERRA project is based on an ESP32 device, equipped with a range of sensors for navigation and interaction.

### Components

The main components required are:

- ESP32 Feather V2 [link](https://learn.adafruit.com/adafruit-esp32-feather-v2/pinouts)
- Waveshare 1.28" Round LCD Display Module with GC9A01 Driver [link](https://www.waveshare.com/1.28inch-lcd-module.htm)
- Adafruit Push-button Power Switch [link](https://thepihut.com/products/adafruit-push-button-power-switch-breakout)
- Beitian BN 880 GPS module [link](https://store.beitian.com/products/beitian-compass-qmc5883l-amp2-6-pix4-pixhawk-gnss-gps-glonass-dual-flight-control-gps-module-bn-880q?variant=44696120295711)
- Adafruit DRV2605L Haptic Controller Breakout [link](https://learn.adafruit.com/adafruit-drv2605-haptic-controller-breakout/arduino-code)
- CMPS12 Compass [link](https://www.robot-electronics.co.uk/cmps12-tilt-compensated-magnetic-compass.html)
- Round Battery
- USB C charging port extender

### Wiring
Connect the components together in the following order:

| ESP32 Pin | Screen | Motor Driver | Compass | GPS | Push Button |
| --------- | ------ | ------------ | ------- | --- | ----------- |
| 3V        | VCC    | VIN          | VIN     | VIN |             |
| GND       | GND    | GND          | GND     | GND | GND         |
| 19 (MOSI) | DIN    |              |         |     |             |
| 4 (A5)    | RST    |              |         |     |             |
| 5 (SCK)   | CLK    |              |         |     |             |
| 12        | BL     |              |         |     |             |
| 15        | CS     |              |         |     |             |
| 32 (A7)   |        |              |         |     | OFF         |
| 33        | DC     |              |         |     |             |
| SDA       |        | SDA          | SDA     |     |             |
| SCL       |        | SCL          | SCL     |     |             |
| RX        |        |              |         | TX  |             |
| TX        |        |              |         | RX  |             |

## Arduino Code

To run the Arduino code, you will need to set up the ESP32 board and install the necessary libraries.

### Setup Steps

1. **ESP32 Arduino Setup:** Add the ESP32 board to the Arduino IDE.
    - Go to **Arduino > Preferences**.
    - Add the following URL to the **Additional Boards Manager URLs**:
      ```
      https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json
      ```
    - Select **Adafruit Feather ESP32 V2** as the board.

2. **Install Libraries:** Install the following libraries through the Library Manager in the Arduino IDE.
    - `TinyGPSPlus.h`
    - `TFT_eSPI.h`
    - `Adafruit_DRV2605.h`

3. **Upload the Code:** After installing the libraries and updating the `config.h` file with new coordinates, upload the code to the ESP32 device. The device should work without further adjustments.

4. **Debug Mode:** A `debugMode` flag is available in the code. To test the device with manual coordinates, set this flag to `true` and input coordinates into the serial monitor.

### Further Customizations

You can use your own checkpoint files by converting black and white images into a compatible format.

#### Using Your Own Checkpoint Files

- Convert static black and white, 240x240PX `.bmp` images to `.XBM` format using [this online tool](https://www.online-utility.org/image/convert/to/XBM).
- Open the XBM file in a text editor and copy the hexadecimal values included in the curly brackets.
- Replace the existing files in the `images/checkpoints` directory with your customized `.h` file.

### Contributors
Vision + Concept: Modem<br> 
Industrial Design: Panter&Tourron<br> 
Software + Hardware: Alice Stewart<br> 
Product Renderings: Côme Lart<br> 
Illustrations: Hugo Bernier<br> 
Campaign Teaser: Michael Willis

### License
TERRA is licensed under the GNU General Public License v3.0 © 2024 Modem 
