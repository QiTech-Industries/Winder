## Design Decisions
Why does the ferrari start at the right side when winding?
- more comfortable to feed in filament
- hole on spool inside is better visible on the right side
- filament can be pushed away from user not pulled towards him

Why does starting to Wind require two button presses?
- filament diameter is adjusted by pulling speed **not nozzle diameter**
- diameter has to be adjusted first before it makes sense to wind on spools

Why is the update server not using https?
- https needs certificate validation
- certificate needs to be stored on the device itself
- certificates refresh every 3 months so stored certificate would need to be updated every 3 months inside Winder Storage

Why is the configuration stored in a .conf file, not JSON?
- JSON Serialization is slow and needs RAM
- ESP is memory constrained
- RTOS watchdog could be triggered if task takes too long

Why are we using LittleFS and not just SPIFFS?
- LittleFS has folder support
- LittleFS should be faster than SPIFFS
- LittleFS is going to replace SPIFFS officially so we are already prepared

Why is the configuration stored in the file system not EEPROM?
- EEPROM has a limited lifetime (read/write cycles)
- reads and writes are slower than to file system
- Storage size is very limited

Why are some functions provided by the HardwareControl Library?
- meant to be used within other projects too

Why is there always a .h and a .cpp version of a file?
- .h files contain all documentation and the api structure (classes, methods, variables)
- .cpp filed contain the implementation of the actual functions
- this is how c++ works :)

Why are we using PlatformIO and not Arduino IDE?
- PlatformIO is much more versatile and provides functions like:
	- proper code highlighting
	- multiple build environments
	- version control of libraries
	- faster build times
	- unit testing capabilities
	- can be used inside the Editor everyone loves
	- ...
