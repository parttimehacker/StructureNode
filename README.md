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

-   Modular Python design
-   Logging support
-   Command-line arguments
-   Service-friendly architecture
-   Hardware or API integration
-   Clean shutdown handling

------------------------------------------------------------------------

## Architecture

Describe the system structure:

-   Data collection layer
-   Processing layer
-   Messaging layer (MQTT / REST)
-   Storage layer (SQLite / Database)
-   Display or Web interface (optional)

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
