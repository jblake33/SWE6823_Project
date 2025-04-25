# SWE6823 Project
Source code archive for my "Scheduled pet feeder" embedded systems project. 

## About The Project
This was created as a semester-long project for the class SWE6823 - Embedded Systems at Kennesaw State University, under the supervision of Prof. Michael Franklin.

This solo project was proposed and completed on my own, with feedback from Prof. Franklin. The project incorporates relevant embedded systems knowledge - sensors, actuators, I/O, multitasking, and more - with the result being a working prototype of a scheduled pet feeder. The prototype was built using an Arduino Uno Rev3 and various hardware components found online. It features a user interface with two menus for viewing/changing the food schedule and managing system-related options, and uses buttons for input. It uses a real-time clock to keep track of time, and a DC motor to swing a door open/shut to dispense food below. 

The prototype fulfilled all planned requirements, was created within schedule, and was presented in-person. 

## Images

| Image | Description |
| ----- | ----------- |
| <img src="/images/20250425_094717.jpg" height="300" width="400"> | Home screen |
| <img src="/images/20250425_095337.jpg" height="300" width="400"> | System Info screen |
| <img src="/images/20250425_094835.jpg" height="300" width="400"> | View Times screen |
| <img src="/images/20250425_110720.jpg" height="300" width="400"> | Set System Time screen |
| <img src="/images/20250425_095217.jpg" height="300" width="400"> | System, with a jar below the food door |
| <img src="/images/20250425_094445.jpg" height="300" width="400"> | Hardware components (from left to right): Food door contraption, microcontroller, breadboard, and display |
| <img src="/images/20250425_110612.jpg" height="300" width="400"> | Photoresistor sensor and LED to detect jams |

## Hardware And Software Used

Hardware used:
| Hardware | Description |
| -------- | ----------- |
| Arduino Uno Rev3 | Microcontroller, based on ATmega328 single-chip |
| Arduino Motor Shield Rev3 | Daughterboard on the Rev3 to connect/operate motor |
| DS3231 | Real-time clock module |
| EK1450 | DC motor |
| LCD2004 | LCD display |

Also used: buttons, resistors, LEDs, breadboard, photoresistor, external power supply, and wires.

Created with Arduino IDE and various libraries: LiquidCrystal_I2C, I2C_RTC, Wire.h. Code is compiled with avr-g++, which can be installed with Arduino IDE.

## Thanks
Thank you to Prof. Franklin for the lectures and helpful information, this project taught me a lot!

