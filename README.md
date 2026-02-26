# StructureNode

Sensor used to report movement and environment factors impacting a family home.

------------------------------------------------------------------------

## Description

This **C++** and hardware application provides the following functionality:

StructureNode is a low-power, BLE-based environmental monitoring node designed for residential attic and structural health tracking. Built for DIY developers, it measures temperature, humidity, pressure, dew point, battery status, and motion, then broadcasts the data using a compact custom BLE advertising payload. The device supports hardware-selectable location IDs via DIP switches, deep sleep for extended battery life, and seamless integration with a Raspberry Pi + Django backend (StructureHub) for logging, analysis, and risk visualization. Ideal for breadboard prototyping and long-term home deployment.

------------------------------------------------------------------------

## Badges

![Platform](https://img.shields.io/badge/Platform-Arduino-green))&nbsp; &nbsp;
![Language](https://img.shields.io/badge/Language-C++-green)&nbsp; &nbsp;
![Domain](https://img.shields.io/badge/Domain-IOT-yellow)&nbsp; &nbsp;
![Connectivity](https://img.shields.io/badge/Connectivity-Bluetooth-blue)&nbsp; &nbsp;

------------------------------------------------------------------------

## Table of Contents

-   [General Information](#general-information)
-   [Technologies Used](#technologies-used)
-   [Features](#features)
-   [Installation](#installation)
-   [Project Status](#project-status)
-   [Room for Improvement](#room-for-improvement)
-   [Roadmap](#roadmap)
-   [Acknowledgements](#acknowledgements)
-   [License](#license)
-   [Contact](#contact)

Supporting documents in docs directory

- Architecture
- Design goals and approach
- Power management strategy

------------------------------------------------------------------------

## General Information

### What problem does this project solve?

Attics, crawlespacess and othe isolated locations in a family home are susceptible to mosture damage. 

### What is the purpose?

StructureNode is used to monitor the status of these location and alert the homeowner to potential problems.

### Why was it built?
- My roof required replacement much earlier than expected due to temperature related damage to shingles with resulting water damage.
-  Exploring hardware integration and low power design

------------------------------------------------------------------------

## Technologies Used

- Arduino IDE supporting C/C++
-   Seeed Studio XIAO nRF52840 host
-   HiLetgo TPL5110 Timer Breakout Module
-   PARALLAX PIR Motion Sensor
-   Adafruit BME680 - Temperature, Humidity, Pressure and Gas Sensor

------------------------------------------------------------------------

## Features

- Ultra-Low Power BLE Sensor Node – Battery-optimized attic/structure monitoring device designed for long sleep cycles with optional PIR wake events.
- Custom BLE Advertising Payload – Broadcasts temperature, humidity, pressure, dew point, battery voltage/percentage, motion flags, and location ID in a compact structured packet.
- Multi-Location Support via DIP Switch – Hardware-selectable location encoding (e.g., Attic, Crawlspace, Garage) without firmware changes.
- Environmental Risk Monitoring – Tracks moisture-related conditions that impact roof decking, insulation, and structural wood integrity.
- Deep Sleep Architecture** – Supports timed wake intervals and optional interrupt-based wake (PIR), maximizing Li-ion battery life.
- Server-Ready Design** – Built to integrate with a Raspberry Pi + Django backend (StructureHub/AtticGuard) for logging, trend analysis, and risk dashboards.
- Breadboard-Friendly Hardware** – Designed for DIY prototyping using Feather nRF52840 / ESP32-class boards and common I2C environmental sensors.
- ebug + Production Modes** – Serial diagnostics for development plus clean BLE output for unattended deployment.

------------------------------------------------------------------------

## Installation

- Clone the Repository
- Launch your development IDE
- Open the StructureNode.ino file
- Import the Studio XIAO nRF52840 processor library
- Import the following application specific libraries
```
#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include <bluefruit.h>
#include <Adafruit_SHT31.h>
#include <Adafruit_BMP280.h>
```
- Compile and build
- Open monitor to verify loading

------------------------------------------------------------------------

## Configuration

The major components include the following:

-   Seeed Studio XIAO nRF52840 processor
-   HiLetgo TPL5110 Timer Breakout Module
-   PARALLAX PIR Motion Sensor
-   Adafruit BME680 environment sensor
-   Pololu 3.3v StepUp / StepDown volage regulator
-   3.7 Lipo Battery

Electronic parts needed:

- Resistors and capacitors
- 2N3904 adn 2N3906 transistors
- Breadboard (developemnt)
- Protoboard (production)

------------------------------------------------------------------------

## Project Status

![Status](https://progress-bar.dev/75/?title=progress)

- In development and testing
- Converting breadboard to protoboard
- Building enclosure

------------------------------------------------------------------------

## Room for Improvement

-   Improve power savings
-   Add unit tests
-   Improve documentation

------------------------------------------------------------------------

## Roadmap

-   [ ] Add test coverage
-   [ ] Add structured logging
-   [ ] Add REST API
-   [ ] Add dashboard
-   [ ] Add Docker support
-   [ ] Publish to PyPI

------------------------------------------------------------------------

## Acknowledgements

-   Open-source libraries used
-   Community contributors
-   Inspiration sources

------------------------------------------------------------------------

## License

This project is licensed under the MIT License.&nbsp; &nbsp;[![License:MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

------------------------------------------------------------------------

## Contact

- Created by **Dave Wilson**
- GitHub: https://github.com/parttimehacker
- Website: https://theairportguy.net
- Email: parttimehacker@gmail.com
