# Weather Station

Arduino weather station for final assignment Technology for Interaction, part of the minor Research in Emerging Technologies at the University of Applied Sciences.

# Goal

The goal of this project is to create a small device that can be placed near the front door of your home.
Whenever you pass by it (or press the button), it will fetch the weather from an API. The code for the API
is available [here](https://github.com/peshmerge/wstation). The device will then notify the user in case
of certain weather conditions (for example rain, snow, sunny, etc).

# Getting Started

The devices we use:

- WeMos D1 (Arduino-compatible board with wifi module)
- 8x8 LED matrix for displaying an icon
- Button
- Small speaker

To get the software running:

- Clone the repository.
- Download the [`ArduinoJson.h`](https://github.com/bblanchon/ArduinoJson/releases) library, and place it in the `weather_station` directory.
- Connect the WeMos D1 to your computer with a micro-USB -> USB cable
- Upload the code

Instructions on setting up the WeMos D1 for the Arduino IDE can be found [in the WeMos documentation](https://wiki.wemos.cc/tutorials:get_started:get_started_in_arduino)
