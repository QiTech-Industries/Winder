
# About: Jarvis Winder
This project covers the code for the Jarvis Winder, which is a machine for winding filaments used for 3D-printing. The projects and its parts are property of QiTech Industries and must not be altered, copied or distributed without written permission.
## Purpose
The winder is used to wind freshly extruded filament on spools and rewind filament from one spool to another. The machine can be controlled via webinterface, which can be accessed with mobile devices, too. The winder can either log into an existing wifi or create its own on the fly.

# Development
## Setup
Steps for setting up your dev-environment
- Install and set up Visual Studio Code
    - required extensions ( ``Ctrl+Shift+X`` )
        - C/C++ `ms-vscode.cpptools`
        - PlatformIO IDE `platformio.platformio-ide`
    - recommended extensions
      - Code comments with `/** + Shift + Enter` `cschlosser.doxdocgen`
      - Code spell checker `streetsidesoftware.code-spell-checker`
      - Markdown formatter `yzhang.markdown-all-in-one`
    - required settings
        - C_Cpp.clang_format_fallbackStyle: ``{ BasedOnStyle: Google, IndentWidth: 4, ColumnLimit: 140 }``
          - use `Shift + Alt + F` to format files according to style
        - files.insertFinalNewline: true
        - files.eol: \n
- Install ESP32 drivers
  - CP210 https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers
  - CH341 http://www.wch.cn/downloads/CH341SER_EXE.html
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
## Contributing
1. Open a new feature request issue on what you want to implement or state in an existing one that you want to implement/fix it
2. create a new branch from the issue based on the main branch
3. make the modifications inside your branch
4. create a PullRequest into main
5. wait for a review (and make requested changes)
6. Once your Pull Request gets approved it will be merged
## Hardware
Basic information and specs on used modules / microcontrollers for the developer
- Nema 17 (17HS19-2004S1)
- Nema 17 Planetary 1:5.18 (17HS19-1684S-PG5)
- TMC2130
- ESPduino (ESP32WROOM32)
- CNC Shield

# Formalities
## Licenses
Here is a list of licenses for the libraries and components being used:
| Library           | Info                                                 | License       |
| ----------------- | ---------------------------------------------------- | ------------- |
| ArduinoJson       | https://arduinojson.org/                             | MIT           |
| Timer             | https://github.com/brunocalou/Timer                  | MIT           |
| ESPAsyncWebServer | https://github.com/me-no-dev/ESPAsyncWebServer       | GNU-2.1       |
| LITTLEFS          | https://github.com/lorol/LITTLEFS.git                | GPL-2.0       |
| HardwareControl   | https://github.com/QiTech-Industries/HardwareControl | QiTech Custom |
| FastAccelStepper  | https://github.com/gin66/FastAccelStepper            | MIT           |
| TMCStepper        | https://github.com/teemuatlut/TMCStepper             | MIT           |
