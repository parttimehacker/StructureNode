# StructureNode

Sensor used to report movement and environment factors impacting a family home.

------------------------------------------------------------------------

## Description

This **C++** and hardware application provides the following functionality:

StructureNode is a low-power, BLE-based environmental monitoring node designed for residential attic and structural health tracking. Built for DIY developers, it measures temperature, humidity, pressure, dew point, battery status, and motion, then broadcasts the data using a compact custom BLE advertising payload. The device supports hardware-selectable location IDs via DIP switches, deep sleep for extended battery life, and seamless integration with a Raspberry Pi + Django backend (StructureHub) for logging, analysis, and risk visualization. Ideal for breadboard prototyping and long-term home deployment.

------------------------------------------------------------------------

## Badges

[![License:
MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
![Language](https://img.shields.io/badge/Language-C++-blue)
![Platform](https://img.shields.io/badge/Platform-Arduino-blue)

------------------------------------------------------------------------

## Table of Contents

-   [General Information](#general-information)
-   [Technologies Used](#technologies-used)
-   [Features](#features)
-   [Architecture](#architecture)
-   [Setup](#setup)
-   [Usage](#usage)
-   [Configuration](#configuration)
-   [Running as a Service (Linux)](#running-as-a-service-linux)
-   [Project Status](#project-status)
-   [Room for Improvement](#room-for-improvement)
-   [Roadmap](#roadmap)
-   [Acknowledgements](#acknowledgements)
-   [License](#license)
-   [Contact](#contact)

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

## Power Budget Breakdown

StructureNode is engineered for long-term Li-ion battery operation with minimal maintenance.

Below is a typical duty-cycle model for a 15-minute wake interval:

### Assumptions
- 3.7V Li-ion battery
- 2000 mAh capacity (typical 18650 cell)
- 15-minute wake interval (96 wake events/day)
- PIR wake events rare

### Current Consumption Model
Deep Sleep
- nRF52840 deep sleep: ~10–20 µA
- Sensor in standby: ~1–5 µA
- Total sleep current: ~25 µA

Sleep time per cycle: ~899 seconds
(15 minutes − ~1 second active time)

### Active Measurement + BLE Advertise
Typical breakdown:
- Wake + sensor read: ~50 ms
- Dew point computation: negligible
- BLE advertising burst (multiple packets): ~200–500 ms
- Total active window: ~1 second

Active current (radio on): ~8–15 mA average

### Daily Average Current Calculation (Approximate)
Sleep:
```
25 µA × 23.75 hours ≈ 0.59 mAh/day
```
Active:
```
15 mA × (96 seconds/day total active) ≈ 0.40 mAh/day
```
Total estimated daily consumption:
```
≈ 1.0 mAh/day
```

### Estimated Battey Life
For a 2000 mAh battery:
```
2000 mAh / 1 mAh/day ≈ 2000 days
≈ 5.5 years (theoretical)
```
- Real-world adjustment (temperature, regulator losses, battery aging, leakage, PIR events):
- Practical expectation: 2–4 years.

###What Dominates the Power Budget?
- BLE radio bursts
- Active sensor measurement window
- Any voltage regulator inefficiency
- Unintended wake loops (firmware bugs)

> Deep sleep current is typically not the dominant factor if implemented correctly.

------------------------------------------------------------------------

### Design Tradeoffs

| Feature                 | Power Impact | Decision    |
|-------------------------|--------------|-------------|
| WIFI                    | High         | Rejected    |
| BLE connection mode     | Moderate     | Rejected    |
| BLE advertising         | Low          | Selected    |
| Edge ML / heavy compute | High         | Rejected    |   
| 15-minute interval      | Low          | Selected    |
| PIR interrupt           | Rate Impact  | Selected    |


------------------------------------------------------------------------



### 1. Clone the Repository

``` bash
git clone https://github.com/yourusername/projectname.git
cd projectname
```

### 2. Create a Virtual Environment

``` bash
python3 -m venv venv
source venv/bin/activate
```

### 3. Install Dependencies

``` bash
pip install -r requirements.txt
```

------------------------------------------------------------------------

## Usage

``` bash
python3 main.py --option value
```

Example arguments:

-   `--group`
-   `--display`
-   `--debug`
-   `--config`

------------------------------------------------------------------------

## Configuration

Example environment variables:

    MQTT_HOST=localhost
    MQTT_PORT=1883
    DEBUG=True

------------------------------------------------------------------------

## Running as a Service (Linux)

Example systemd service file:

    [Unit]
    Description=Project Name Service
    After=network.target

    [Service]
    User=youruser
    WorkingDirectory=/path/to/project
    ExecStart=/path/to/venv/bin/python main.py
    Restart=always

    [Install]
    WantedBy=multi-user.target

Enable service:

``` bash
sudo systemctl daemon-reload
sudo systemctl enable projectname
sudo systemctl start projectname
```

------------------------------------------------------------------------

## Project Status

![Status](https://progress-bar.dev/75/?title=progress)

-   Development
-   Stable
-   Production
-   Experimental

------------------------------------------------------------------------

## Room for Improvement

-   Improve modularization
-   Add unit tests
-   Add Docker support
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

This project is licensed under the MIT License.

------------------------------------------------------------------------

## Contact

Created by **Your Name**

GitHub: https://github.com/yourusername\
Website: https://yourwebsite.com\
Email: optional
