# Functionality and features
This document lists features and their requirements. It is meant to be used both as documentation and as a template for test protocols.

## Base features
1. Power up motors on start
	- The humming of the motors indicates that the machine has successfully started
1. Persistent configuration
	- Winders previously configured with Blynk will load the Blynk Wifi Config
    - On machine-start the last known configuration is loaded and used if valid
	- On change (in the webfrontend) the current configuration is saved
1. Standalone access point
	- If not disabled the machine creates its own Access Point (AP) allowing for an independent operation of the machine
	- If no external wifi is available / specified, an access point is created (even if previously disabled), preventing the user from locking himself out
	- Connecting to the access point will automatically redirect the user to the machines webfrontend (Captive Portal)
	- The Frontend can either be reached via winder.local (ESP runs a DNS Server) or via the Winder IP (in AP Mode 192.168.4.1)
1. Autoupdater
	- While in Standby, the machine will periodically check for updates when connected to an external wifi, and install them on the next machine-start if possible
	- The configuration is rolled over by saving it to a different storage partition that is not overwritten by the update
1. Load Detection
	- When Winding the Spool motor automatically speeds up or slows down so the filament constantly stays at tension
	- When a function with position accuracy is needed the Ferrari Motor performs an sensorless auto home against the motor coupler (if not previously homed)

## Webfrontend
Called by the user via browser
1. Menubar on the top
    - Qi-Logo in top left, linking to the main view
	- Menu-entries in top right, linking to the ``Connect``-/``Control``-/``Configure``-views
1. View: Start
	- Welcome-message and logo of stylized winder-machine
	- Link to datasheet / manual of winder
	- Link to external video tutorial (youtube)
	- Changelog (i-button in bottom left corner)
		- Show a popup with a scrollable list of updates/changes (not implemented)
	- Note and logo in bottom right, providing a reference to the company
1. View: Connect
	- Display a loading animation while waiting for a list of available wifis
	- Display a scrollable list of available wifis
		- Show name, signal strength and encryption-type/open
		- Highlight a wifi that the machine is connected to
		- On selecting a wifi in the list provide an interface for connecting to it, with an input for specifying a password.
		- Display a count of the networks found
		- Reload-Button for forcing a rebuild of the wifi-list
1. View: Control
	- View in which commands can be issued to the machine. Changing the view does NOT stop currently running operations
	- Section: Motor-status ("puller", "ferrari", "spool")
		- For each motor display:
		- Current rotations in rpm (negative values depending on direction)
		- power(on/off)
		- Alternatively (on click) a graph with the speed/load of the last minute
	- Section: Speed Adjust
		- Adjust the motor speeds immediately on change of the slider without interrupting the current operation if any
		- The speed is based on the puller-speed in filament-length wound per minute(m/min)
	- Section: Statistics (not yet implemented)
		- Displays winding-related metrics:
		- Filament wound in meters
		- Layers wound
		- Running time in s
		- Total rotations
		- Estimated weight in g
	- A control panel on the bottom of the page to issue commands to the machine, switching between operation modes
		- Only available commands are displayed, depending on the current operation mode
		- Button "Start Pulling"
			- Available in STANDBY-/OFF-mode
			- Enter PULLING-mode
			- Starts the puller and positions the ferrari on the **right** start-position, homing it if necessary
		- Button "Start Winding"
			- Available in PULLING-mode
			- Enter WIND-mode
			- Continue running the puller and start rotating the spool with automatic load-adjust. Oscillate the ferrari between the start- and end-position, automatically synchronizing its speed with the spool for continuous winding-results
		- Button "Unwind"
			- Available in all modes except UNWIND-mode
			- Enter UNWIND-mode
			- Disables spool, moves ferrari to start position and runs puller in inverted direction, thereby unwinding the spool
		- Button "Unwinding"
			- Available in UNWIND-mode
			- Enter STANDBY-mode
			- Stops unwinding and returns to STANDBY-mode
		- Button "Change spool"
			- Available in WIND-mode
			- equal to PULLING-mode
			- Reposition the ferrari to the start-position, stop the spool and continue running the puller. Meant for changing the spool while the external extruder is still running
		- Button "Off"
			- Available in all modes
			- Enter OFF-mode
			- Immediately un-power all motors (freespin), stopping all operations (emergency feature). The ferrari will need to be re-homed after this operation.
1. View: Configure
	- Configuration changes are immediately persistently saved on the machine
	- Section: Configure wifi
		- Device name
		- Access Point Name
		- (De-)Activate Access point
	- Section: Spool calibration
		- When in active operation mode(pulling, winding etc.): Use the adjusted position without interrupting the current operation
		- When in standby: move ferrari to last adjusted position
	- Section: Display technical details
		- Hardware Info
			- Board
			- Revision
		- ESP Firmware
			- Version
			- Build
			- Last Update
		- Interface
			- Version
			- Build
			- Last Update
