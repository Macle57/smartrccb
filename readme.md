# ESP32 JSON POST and Relay Control

This code reads an ESP32 IO pin and sends the data via a POST request to an API in JSON format. Additionally, if a specified threshold is reached, the relay is turned off.

## Running the Code

### Using PlatformIO

To run this code using PlatformIO, you'll need to download the PlatformIO extension for VSCode (or CLion, but VSCode is recommended).

Once installed, open this project as an existing PlatformIO project and click the upload button located in the bottom bar towards the left.

### Using Arduino IDE

Alternatively, you can use the Arduino IDE. Copy and paste the code from `main.cpp` into your Arduino IDE. Be sure to comment out the following line if using Arduino IDE:

```cpp
#include <Arduino.h>
```
You will also need to manually install the ArduinoJson library if using the Arduino IDE (PlatformIO handles dependencies automatically).

Command Line (Optional)
If you prefer, you can install the PlatformIO CLI tool and upload the code via the following commands:

curl -O https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py && python get-platformio.py
pio project init
pio run -e esp32dev -t upload