# Assembly-instructions
A step-by-step guide for assembling and testing the winder and its components.

# General notes on 3D-printing parts
**TODO: Document better?**
- Use black high-quality PLA (**TODO: document settings for printer**)
- Remove fine plastic-strings with a lighter to make it look cleaner, avoiding actual heating of the printed part as much as possible
- Check for printing-errors that could cause the component to break upon use
- Some holes are very tight and need to be heated for assembly, such as inserting a ball-bearing

# Hardware components
## Frame
- Parts
    - **TODO**
- Assembly
    1. **TODO**
- Test / Quality assurance
    1. **TODO**
## Puller
- Parts
    - **TODO**
- Assembly
    1. **TODO**
- Test / Quality assurance
    1. **TODO**
## Puller
- Parts
    - **TODO**
- Assembly
    1. **TODO**
- Test / Quality assurance
    1. **TODO**
## Ferrari
- Parts
    - **TODO**
- Assembly
    1. **TODO**
- Test / Quality assurance
    1. **TODO**
## Ferrari-frame
- Parts
    - **TODO**
- Assembly
    1. **TODO**
- Test / Quality assurance
    1. **TODO**
## Spool
- Parts
    - **TODO**
- Assembly
    1. **TODO**
- Test / Quality assurance
    1. **TODO**
## ESP-Holder
- Parts
    - **TODO**
- Assembly
    1. **TODO**
- Test / Quality assurance
    1. **TODO**
## Rubber Feet
- Parts
    - **TODO**
- Assembly
    1. **TODO**
- Test / Quality assurance
    1. **TODO**
## Laser-Holder
- Parts
    - **TODO**
- Assembly
    1. **TODO**
- Test / Quality assurance
    1. **TODO**


# Electronical components
## TMC-cables
Connects the tmc-drivers on the cnc-shield.
- Assembly / Testing
    1. Connect and test the cables with the cable-tester (**TODO: see image**)
    2. Cut off unneeded fourth cable-jack
    3. Document functionality/bugs with a sticker
- Possible bugs
    - Broken cables, indicated by leds not lighting up on the cable-tester
## TMC-driver
Driver for controlling a stepper-motor.
- Assembly / Testing
    1. Add the cooling element on the top (please all in the same direction)
    2. Cut off unneeded bottom pins (**TODO: see image**)
    3. Insert the TMC into a working setup (preferrably as a ferrari to check load-detection on homing)
    4. Document functionality/bugs with a sticker
- Possible bugs
    - Faulty load-detection (ferrari not stopping when reaching the end)
## CNC-shield
Shield for handeling the tmc-drivers for the stepper-motors.
- Assembly / Testing
    1. Put Jumper at ``EN/GND``
    2. Insert 3 TMC-driver into the slots (**TODO: see image**)
    3. Connect the tmc-drivers and cnc-shield using the tmc-cables (**TODO: see image**)
    4. Connect the cnc-shield with the stepper-motors (**TODO: see image**)
    5. Put cnc-shield on the esp-board
    6. Connect powersupply of cnc-shield with esp-board (black = ``-``, white = ``+``)
    7. Power up the esp-board and test full functionality of all motors
    8. Document functionality/bugs with a sticker
- Possible bugs
    - Not working at all
    - Single drivers not working
## Stepper-cables
Cables connecting the stepper-motors and cnc-shield.
- Assembly / Testing
    1. Use a multimeter to test the connections (**TODO: Is the creation of a tester overkill?**)
    2. Document functionality/bugs with a sticker
- Possible bugs
    - Broken cables
## Stepper-motors
Motors used to move spool, ferrari and puller.
- Assembly / Testing
    1. **TODO**
- Possible bugs
    1. **TODO**
## ESP
Microcontroller, which is the core of our winder
- Assembly / Testing
    1. Solder on power-wires on the bottom side, removing excess wires-ends to avoid short circuits (black = ``-``, white = ``+``, **TODO: see image**)
    2. Connect to USB-port, writing image an making notes of console-output (mac-adress)
        - ``Visual Studio Code>PlatformIO>Project Tasks>src>Platform>Upload Filesystem Image``
        - ``Visual Studio Code>PlatformIO>Project Tasks>Default>General>Upload and Monitor All``
    3. Test basic functionality (Availability of accesspoint)
    4. Remove from USB-Port and put a sticker on the back, writing down the aforementioned mac-adress
    5. Again, test the basic functionality only using the main power supply without a USB-connection (Availability of accesspoint)
    6. Unpower and connect with fully set up cnc-shield that is already connected to the stepper-motors (don't forget about connecting the power to the cables we soldered on earlier)
    7. Power and do a full functionality test (**TODO: translate fully**)
        - **TODO**
        - Webfrontend(View:Control): Start Puller und Geschwindigkeit variieren (Ferrari sollte Homen, Puller sollte sich in richtige Richtung(!) mit richtiger Geschwindigkeit drehen)
        - Webfrontend(View:Control): Start Winding und Geschwindigkeit variieren (Ferrari sollte zwischen den Endpunkten oszillieren, Spool sollte in richtige Richtung drehen. Die Geschwindigkeit von Puller sollte konstant, von Spool und Ferrari sich dem Widerstand anpassen)
        - Falls möglich Testdurchlauf mit echtem Filament bei 0.5mpm, 3mpm, 6mpm, 9mpm und max. Geschwindigkeit
        - Webfrontend(View:Control): Unwind und Geschwindigkeit variieren (Ferrari sollte Homen, puller sollte sich in umgedrehter Richtung bei richtiger Geschwindigkeit drehen).
        - Webfrontend(View:Control) Off / Power: Hartes An- und Ausschalten der Motoren
        - Webfrontend(View:Configure) Spool Calibration anpassen (Sollte sowohl im Standby als auch beim Winding passen, sollte nach Neustart erhalten bleiben)
        - Webfrontend(View:Configure) Anpassung des Device Name und Access Point Name (Testen durch anschließenden Neustart)
        - Webfrontend(View:Connect) Sollte umliegende Wlans anzeigen
        - ...
        - **TODO**: Include testing of the laser-calibration of the new ferrari
        - **TODO**: Include all functions
        - **TODO**: Make sure to save actually useful endpositions for the ferrari-calibration
    8. Document test-results on the sticker
    9. When working: Add board to update-database
        - Power with both supply voltage
        - Connect via USB and monitor the output (``Visual Studio Code>Project Tasks>Default>General>Monitor All``)
        - Add/Update entry in database (**TODO: document**)
        - Reset the winder as needed (updates will download whenever but only install on restart)
        - Check for success in database (**TODO: document**) and webfrontend(bottom of View:Configure)
- Possible bugs
    - No wifi-accesspoint is created without USB-power, tested by starting without USB-connection
    - Brownout, **TODO: describe**
    - Broken USB-Port, does not power when only connected to USB
    - Broken stepdown, **TODO: describe**
    - No upload, **TODO: describe**

## Final assembly
**TODO: Document**
1. Install the assembled puller-frame
2. Install the assembled ferrari and ferrari-frame
    - Check whether the ferrari can run smoothly in both directions
3. Install the motor-cables
4. Install the rubber-feet
    - Ensure that the winder can't wiggle on a flat surface
5. Install the esp
    1. Install the esp-holder
    2. Write down the mac-adress of the esp, possibly adding a temporary label on the winder-frame
    3. Install the ESP and connect it to the motor-cables
    4. Close the case
6. Do a full test-run
    1. Check all firm connection of all connectors and cables
    2. See full esp-test
    3. Respool a spool at min speed, 3mpm, 6mpm, 9mpm and max. speed
        - Check for automatic load-adjust
        - Check for result-quality
    4. Check for smooth motor-operation
        - Vibrations
        - Weird sounds
7. Clean the winder from any leftover dust and dirt
8. On finish(sale):
    1. Remove temporary labels and remove glue-residue
    2. Add a box with attachments and accessories such as power supply and laser-holder
    3. Put into packaging
