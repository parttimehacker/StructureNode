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
