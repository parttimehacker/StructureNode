# StructureNode

Sensor used to report movement and environment factors impacting a family home.

------------------------------------------------------------------------

## Description

This **C++** and hardware application provides the following functionality:

StructureNode is a low-power, BLE-based environmental monitoring node designed for residential attic and structural health tracking. Built for DIY developers, it measures temperature, humidity, pressure, dew point, battery status, and motion, then broadcasts the data using a compact custom BLE advertising payload. The device supports hardware-selectable location IDs via DIP switches, deep sleep for extended battery life, and seamless integration with a Raspberry Pi + Django backend (StructureHub) for logging, analysis, and risk visualization. Ideal for breadboard prototyping and long-term home deployment.

------------------------------------------------------------------------

## Badges

[![License:
MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)\
![Language](https://img.shields.io/badge/Language-C++-blue)
![Arduino Badge](https://img.shields.io/badge/Arduino-00878F?logo=arduino&logoColor=fff&style=for-the-badge)

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

## Architecture

StructureNode follows a lightweight edge-node → gateway → web dashboard model optimized for low power and long-term home deployment.

1️⃣ Edge Sensor Node (Battery Powered)
- Microcontroller: Feather nRF52840 / ESP32-class board
- Environmental Sensor (I2C): temperature, humidity, pressure
- PIR motion sensor (digital interrupt)
- DIP switch for hardware-based location ID
- Custom BLE advertising payload (no pairing required)
- Deep sleep between broadcasts

The node wakes on:
- Timed interval (e.g., every 15 minutes)
- PIR interrupt (rare event)

It measures, computes dew point and battery percentage, packages a compact payload, advertises via BLE, then returns to sleep.

2️⃣ BLE Gateway (Always-On Listener)

- Raspberry Pi Zero 2 W or Pi 4
- Python BLE worker (Bleak-based scanner)
- Filters by manufacturer ID / payload format
- Parses and validates payload
- Writes readings to SQLite

Designed to:

- Drop duplicate advertisements
- Smooth burst transmissions
- Maintain lightweight CPU usage

3️⃣ Django Server (StructureHub / AtticGuard)

- Django + Apache + mod_wsgi
- SQLite backend
- REST-style endpoints (/api/last, /api/history)
- Live dashboard with risk cards
- Trend detection and anomaly analysis

Provides:

- Moisture risk assessment
- Dew point monitoring
- Motion activity tracking
- Battery health monitoring
- Historical charts and timeline views

------------------------------------------------------------------------

## Design Principles
- Maximize battery life
- Stateless broadcast (no BLE connections required)
- Simple home-scale deployment
- Breadboard-first hardware design
- Server-side intelligence layer
- Modular evolution (V1 → V3 → V4 payloads)

------------------------------------------------------------------------

## System Engineering Rationale

StructureNode is intentionally designed around BLE advertising and a gateway architecture rather than WiFi-connected edge nodes. The decisions are driven by power efficiency, reliability, and simplicity.

### Why BLE Instead of WiFi?
Power Consumption
- WiFi radios consume significantly more power, especially during association and DHCP negotiation.
- BLE advertising requires no connection establishment.
- Advertising packets can be transmitted in milliseconds.
- The MCU returns to deep sleep almost immediately.

> Result: Orders of magnitude lower average current draw.

### No Infrastructure Dependency at the Edge
- WiFi nodes require SSID configuration and credential storage.
- Router changes break deployments.
- BLE advertising nodes are infrastructure-independent.
- The gateway handles networking complexity.

> Result: Edge nodes remain simple and resilient.

### Separation of Concerns
- Edge node: measurement + broadcast only.
- Gateway: scan, validate, deduplicate.
- Server: persistence + intelligence + UI.

> Result: Clean system layering and easier debugging.

### Why Stateless BLE Advertising?
No Pairing or Connection Required
- No bonding.
- No GATT services to maintain.
- No reconnection logic.
- No persistent session state.

> Result: Reduced firmware complexity and fewer failure modes.

### Gateway-Controlled Reliability
- BLE advertisements are broadcast repeatedly.
- Gateway smooths bursts and drops duplicates.
- Occasional packet loss is acceptable in 15-minute environmental sampling.

> Result: Probabilistic reliability without protocol overhead.

### Home-Scale Optimization
This system is designed for:
- Few nodes (1–10 sensors)
- Low update frequency (10–15 minutes typical)
- Very small payload sizes

> BLE advertising is optimal for this scale. WiFi would be unnecessary overhead.

### Why Compute Dew Point on the Edge?
- Minimizes server-side dependency.
- Preserves derived values in historical logs.
- Enables risk evaluation even if backend logic evolves.

> Result: Forward-compatible data model.

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
