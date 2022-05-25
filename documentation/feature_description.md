# Functionality and features
This document lists features and their requirements. It is meant to be used both as documentation and as a template for test protocols.

## Base features
1. Power up motors on start
	- The humming of the motors indicates that the machine has successfully started
2. Persistent configuration
	- Winders previously configured with Blynk will load the Blynk Wifi Config
    - On machine-start the last known configuration is loaded and used if valid
	- On change (in the webfrontend) the current configuration is saved
3. Standalone access point
	- If not disabled the machine creates its own Access Point (AP) allowing for an independent operation of the machine
	- If no external wifi is available / specified, an access point is created (even if previously disabled), preventing the user from locking himself out
	- Connecting to the access point will automatically redirect the user to the machines webfrontend (Captive Portal)
	- The Frontend can either be reached via winder.local (ESP runs a DNS Server) or via the Winder IP (in AP Mode 192.168.4.1)
4. Autoupdater
	- While in Standby, the machine will periodically check for updates when connected to an external wifi, and install them on the next machine-start if possible
	- The configuration is rolled over by saving it to a different storage partition that is not overwritten by the update
5. Load Detection
	- When Winding the Spool motor automatically speeds up or slows down so the filament constantly stays at tension
	- When a function with position accuracy is needed the Ferrari Motor performs an sensorless auto home against the motor coupler (if not previously homed)
