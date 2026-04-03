#ESP32-S3 SLOTS

This project uses a eps32-s3 devkit and a LCM1602 screen to play slots

#WIRING
GND | GND
VCC | 5V
SDA | GPIO8
SCL | GPIO9

#Setup

1)Grab the .ino file and open it in Arduino IDE
2)After opening the sketch go to Sketch > Include Library > Manage Libraries > Add the LiquidCrystal I2C library
3)Upload the code to the esp32 and you are ready to play

#Instructions 
Pressing The Boot button spins the slot and gives you the result based on your matches you either get more money or lose what you jest bet(the bet is 10)
