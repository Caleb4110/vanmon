# Vanmon
Vanmon is a monoring tool for a 12v battery system. It is designed to monitor the battery voltage and current,
and to provide a web interface for viewing the data. It is still a work in progress.

## Features
- Monitors battery voltage and current
- Provides a web interface for viewing the data
- Calculates time until battery is empty

To use, you connect an INA219 sensor to each appliance in the van and create instances of an ina219 module for each sensor.
The ESP32 will then read each sensor and display the data on a web interface. It can also output the data to any i2c device,
such as a display.

## Setup
### Pre-requisites
- ESP32 board
- INA219 sensor module
- esp-idf library installed on your computer
- esp-idf-lib library installed on your computer

### Installation
- Clone the repository
- cd into the repository
- run get_idf to initialise the esp-idf environment
- run idf.py menuconfig to configure the project
    - set the I2C address of the INA219 sensors
- run idf.py -p PORT flash monitor to upload the code to the ESP32 and monitor the output

## Usage
Once running, and you are connected to the ESP32's access point (VanMon),
you should be able to access the web interface at http://<ESP32_IP_ADDRESS>/
