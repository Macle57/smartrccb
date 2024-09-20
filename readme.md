This code reads esp32 io pin and POST it to an api as json.
Also if threshold is reached, relay is turned off.

To run this code via platformio, you will have to download the platformio extension for VSCode. (or clion but don't do that to yourself)

Once downloaded, go to extension, open this project as existing project with platformio and hit the upload button located bottom bar towards left.

Or optionally you can use the Arduino IDE and copy paste the code main.cpp to ur arduino ide. Just make sure to comment out the 
#include <Arduino.h> line.
And if you are using Arduino IDE you will have to manually install the ArduinoJson library. (Platformio handles dependency automatically)

Optionally u can also install pio command line tool and use it to upload the code via
curl -O https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py && python get-platformio.py
python get-platformio.py
pio project init
pio run -e esp32dev -t upload
