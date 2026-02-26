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
