Die firmware.bin Datei enthält ein Minimalsetup von Blynk, um das Update von Blynk Windern testen zu können. Die Firmware Datei kann mittels folgendem Kommando geflashed werden:

`c:\users\qitech\.platformio\penv\scripts\python.exe C:\Users\Qitech\.platformio\packages\tool-esptoolpy\esptool.py --chip esp32 --port "COM5" --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dout --flash_freq 40m --flash_size detect 0x10000 examples\BlynkFirmwareBuild\firmware.bin`
