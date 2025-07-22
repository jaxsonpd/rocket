# Rocket Controller Hardware

## Key components

The rocket controller uses the following hardware:

Controller - STM32F103xx

Accelerometer - LSM6DS3TR-C

Magnetometer - LIS2MDLTR

Barometer - BMP588

Flash - W25Q128JV519

## Serial Bus Allocation

In order to maximise data throughput the serial buses are split across the devices. The allocation is:

- SPI 1 - Accelerometer
- SPI 2 - Flash Memory/SD Card
- I2C 1 - Magnetometer with a connection to master out of accelerometer
- I2C 2 - Barometer 