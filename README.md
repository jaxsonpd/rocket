# Rocket

This repo contains a custom model rocket designed for low altitude (>1500m) flights. It is built completely from scratch with no kits involved (apart from an off the shelf motor). With the may construction method being 3D printing. Currently the software build enviroment and TUI has been proven on the STM32F103 and work is being conducted on the flight controller PCB (schematic is mostly finished) and the CAD models for the required parts. Software will pick back up with the pcbs are manufactured.

## Rocket Specifications

The current design is a single stage D class rocket with an theoretical approximate apogee of 280m. This design uses a off the shelf PVC tube as the main body with 3D printed structures.

![Rocket Rendering](doc/pictures/open_rocket_render.png)

## Flight computer

### Software

The software stack uses the CMSIS wrapper with platform IO. This is done to allow for easy change between various controllers with the current option being the STM32F103. Currently only the TUI has been implemented with the rest of the development waiting until the hardware is finalised.

### Hardware

The current flight computer focuses on data acquisition with limited control. Its design is based around the STM32F1 or F4 series of microcontrollers. It contains the following components:

- MCU
- Accelerometer
- Magnometer/Compass
- Barometer and Temperature Sensor
- Flash Memory (For flight data)
- SD Card (For flight data)
- Pyro ports x2, to allow for in flight deploables
- Servo mounts x3, to allow for future thrust vectoring projects
