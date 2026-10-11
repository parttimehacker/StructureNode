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

### What Dominates the Power Budget?
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


