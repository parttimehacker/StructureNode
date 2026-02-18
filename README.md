# StructureNode

Sensor used to measure and report movement and environment in different areas of a family home.

------------------------------------------------------------------------

## Description

This **C++** application provides the following functionality:

StructureNode is a low-power, BLE-based environmental monitoring node designed for residential attic and structural health tracking. Built for DIY developers, it measures temperature, humidity, pressure, dew point, battery status, and motion, then broadcasts the data using a compact custom BLE advertising payload. The device supports hardware-selectable location IDs via DIP switches, deep sleep for extended battery life, and seamless integration with a Raspberry Pi + Django backend (StructureHub) for logging, analysis, and risk visualization. Ideal for breadboard prototyping and long-term home deployment.

------------------------------------------------------------------------

## Badges

[![License:
MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)\
![Language](https://img.shields.io/badge/Language-C++-blue)

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
- Roof required replacement much earlier than expected due to temperature related damage to shingles.
-   Exploring hardware integration and low power design

------------------------------------------------------------------------

## Technologies Used

Example:

-   Python 3.x
-   Virtualenv
-   MQTT / REST API
-   Flask / Django / FastAPI (if applicable)
-   SQLite / PostgreSQL
-   Raspberry Pi / Linux
-   Docker (optional)

------------------------------------------------------------------------

## Features

🔋 Ultra-Low Power BLE Sensor Node – Battery-optimized attic/structure monitoring device designed for long sleep cycles with optional PIR wake events.
📡 Custom BLE Advertising Payload – Broadcasts temperature, humidity, pressure, dew point, battery voltage/percentage, motion flags, and location ID in a compact structured packet.
🏠 Multi-Location Support via DIP Switch – Hardware-selectable location encoding (e.g., Attic, Crawlspace, Garage) without firmware changes.
🌡 Environmental Risk Monitoring – Tracks moisture-related conditions that impact roof decking, insulation, and structural wood integrity.
💤 Deep Sleep Architecture** – Supports timed wake intervals and optional interrupt-based wake (PIR), maximizing Li-ion battery life.
🧠 Server-Ready Design** – Built to integrate with a Raspberry Pi + Django backend (StructureHub/AtticGuard) for logging, trend analysis, and risk dashboards.
🛠 Breadboard-Friendly Hardware** – Designed for DIY prototyping using Feather nRF52840 / ESP32-class boards and common I2C environmental sensors.
📊 Debug + Production Modes** – Serial diagnostics for development plus clean BLE output for unattended deployment.

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

## Setup

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
