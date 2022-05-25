# About: Jarvis Winder
This project covers the code for the Jarvis Winder, which is a machine for winding filaments used for 3D-printing. The projects and its parts are property of QiTech Industries and must not be altered, copied or distributed without written permission.
## Purpose
The winder is used to wind freshly extruded filament on spools and rewind filament from one spool to another. The machine can be controlled via webinterface, which can be accessed with mobile devices, too. The winder can either log into an existing wifi or create its own on the fly.
## Documents
For details on usage and specifications refer to the following manuals and documents:
- TODO: User-Manual
- TODO: Marketing-flyer (or whatever)
- ...

# Development
## Setup
Steps for setting up your dev-environment
- Install and set up Visual Studio Code
    - Extensions ( ``Ctrl+Shift+X`` )
        - C/C++
        - PlatformIO IDE
- Checkout the code from git
- On the left side click on the 'PlatformIO'-Button
    - Compile only: ``project tasks>esp32dev>general>build``
    - Compile and run on microcontroller: ``project tasks>esp32dev>general>upload and monitor``
## Code structure
Do note that the following description might be outdated due to the everchanging nature of code. When in doubt result the code and don't be shy about bothering whoever is responsible to actually update documentation when committing new code.
- PlatformIO is used to manage the microcontroller related bits
    - check the ``platformio.ini`` for details
- Homebrew modules / libraries
    - BaseController
    - Logger
    - Validator
## Hardware
TODO: Basic information and specs on used modules / microcontrollers for the developer
- Stepper: TODO
- Board: TODO

# Formalities
## Licenses
Here is a list of licenses for the libraries and components being used:
| Library | Info | License |
|---|---|---|
| ArduinoJson.h | https://arduinojson.org/ | MIT |
| timer.h | https://github.com/contrem/arduino-timer ? | BSD |
| timerManager.h | TODO | TODO |
| ESPAsyncWebServer.h | https://github.com/me-no-dev/ESPAsyncWebServer | TODO |
| LITTLEFS.h | TODO | TODO |
| TMCStepper.h | TODO | TODO |
| FastAccelStepper.h | TODO | TODO |

TODO: Only comment actual libraries, not just some random header-files I found
