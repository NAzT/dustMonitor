# Introduction
This is a CO2/TVOC air quality monitor designed for keeping track of air quality indoors. 

It is build on top of a TTGO T4 V1.3 board and uses a MH-Z19B and CCS811 sensor.

![alt](https://clinetworking.files.wordpress.com/2020/01/img_20200125_223056.jpg)
![alt](https://clinetworking.files.wordpress.com/2020/01/img_20200125_221439.jpg)

# Features
It features the following:

1. Real time LCD graphs (8hr, 3hr, 1hr, 30min and 10min intervals)
2. Bluetooth support (app available)
3. TVOC measurement
4. CO2 measurement
5. Battery power
6. LCD graphs with CO2/Temp
7. Mobile graphs displaying a 24 hour range

# Resources 
The source code is available here:

Bluetooth app: https://github.com/wilyarti/AirMonitor
ESP32 firmware: https://github.com/wilyarti/dustMonitor

# Build
Flash the following firmware to the device: https://github.com/wilyarti/dustMonitor/blob/master/firmware.bin

The pin out is as follows:

MH-Z19B:
- TX Pin 33
- RX Pin 35
VCC to 5v

CCS811:
- SDA Pin 19
- SCL Pin 26
- VCC to 3.3v

# Mobile app

Mobile app here: https://github.com/wilyarti/AirMonitor


