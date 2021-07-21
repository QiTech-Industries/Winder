# Winder
Filament winding machine powererd by ESPdruino
##


# Board Schematics
## Arduino + CNC Shield
![Arduino-CNC-Shield-Scematics_](https://user-images.githubusercontent.com/44653977/126171971-0116264c-c214-4cdb-9ea3-75b781c61fb7.jpg)
## ESPdruino
![ESPdruino-Schematics](https://user-images.githubusercontent.com/44653977/126172108-59261051-43c0-4276-8642-24a51f24ba7b.jpeg)
## TMC21130 pin out
![TMC2130-Pinout-with-Legend-1](https://user-images.githubusercontent.com/61654753/126180279-9baa4cb1-23f9-4813-917e-d8515cacb58d.jpg)

# Port Mappings
| ESPdruino | Adruino | CNC Shield |
|-----------|---------|------------|
| 0         |         |            |
| 5V        | 5V      |            |
| RST       | RES     | RST        |
| 3,3V      | 3,3V    | 3,3V       |
| 5V        | 5V      | 5V         |
| GND       | GND     | GND1       |
| GND       | GND     | GND2       |
| VN        | VIN     | V_IN       |
| 2         | 0       | AD0        |
| 4         | 1       | AD1        |
| 36        | 2       | AD2        |
| 34        | 3       | AD3        |
| 38        | 4       | AD4        |
| 39        | 5       | AD5        |
| SCL       | SCL     | SCLtopright|
| SDA       | SDA     | SDAtopright|
| RST       | AREF    |            |
| GND       | GND     | GND        |
| 18        | 13      | SpinDir    |
| 19        | 12      | SpinEnable |
| 23        | 11      | Z-EndStop  |
| 5         | 10      | Y-EndStop  |
| 13        | 9       | X-EndStop  |
| 12        | 8       | EN         |
| 14        | 7       | Z-DIR      |
| 27        | 6       | Y-DIR      |
| 16        | 5       | X-DIR      |
| 17        | 4       | Z-STEP     |
| 25        | 3       | Y-STEP     |
| 26        | 2       | X-STEP     |
|           | TX->1   | D1         |
|           | TX<-0   | D0         |
