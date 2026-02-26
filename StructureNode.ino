/*
StructureNode is a low-power, BLE-based environmental monitoring node designed
for residential attic and structural health tracking.

Built for DIY developers, it measures temperature, humidity, pressure, dew point,
battery status, and motion, then broadcasts the data using a compact custom BLE
advertising payload.

The device supports hardware-selectable location IDs via DIP switches, a sleep
period for battery life, and integration with a Raspberry Pi + Django backend
(StructureHub) for logging, analysis, and risk visualization.

Ideal for breadboard prototyping and long-term home deployment.
*/

#include <Arduino.h>
#include <Wire.h>
#include <math.h>

// ============================================================
// MUST BE ABOVE ALL #includes (Arduino auto-prototype workaround)
// ============================================================

enum SensorLocation : uint8_t {
  LOCATION_ATTIC = 0,
  LOCATION_CRAWLSPACE = 1,
  LOCATION_BASEMENT = 2,
  LOCATION_OTHER = 3
};

/*
  StructurePayload V4
  Notes:
  - Packed so it can be placed directly into BLE manufacturer data.
  - All fixed-point fields are scaled to avoid floats on the wire.
  - Endianness is whatever the MCU uses (little-endian on nRF52).
    Your decoder should treat this as a binary struct of known layout.
*/
struct __attribute__((packed)) StructurePayload {
  uint16_t protocol;         // 0x0004
  uint8_t  location;         // 0..3 from DIP (SensorLocation)
  int16_t  tempC_x100;       // Temperature (°C * 100), signed
  uint16_t humPct_x100;      // Relative Humidity (% * 100)
  uint16_t press_hPa_x10;    // Pressure (hPa * 10) => 1 decimal place
  uint16_t batt_mV;          // Battery millivolts
  uint16_t flags;            // Bitfield of which measurements are valid
  uint16_t seq;              // Sequence counter (wraps)
  uint16_t motion0;          // Motion counter 0 (wraps)
  uint16_t motion1;          // Motion counter 1 (wraps)
  uint8_t  battPct;          // Battery percent 0-100
  uint8_t  rsv0;             // Reserved (0)
  uint16_t uptimeMin;        // Uptime minutes (wraps)
  int16_t  dewPointC_x100;   // Dew point (°C * 100), signed
};

// ============================================================
// Includes
// ============================================================

#include <Arduino.h>
#include <Wire.h>
#include <math.h>

#include <bluefruit.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

#define SEALEVELPRESSURE_HPA (1013.25)

// BME680 instance (I2C)
Adafruit_BME680 bme;

// ============================================================
// Debug toggles (turn off for production / power savings)
// ============================================================

#define SN_DEBUG             1   // Master debug switch
#define SN_DEBUG_DIP         1   // DIP read prints
#define SN_DEBUG_PAYLOAD_HEX 1   // Raw payload hex dump

#if SN_DEBUG
  #define DBG_BEGIN(...)   Serial.begin(__VA_ARGS__)
  #define DBG_PRINT(...)   Serial.print(__VA_ARGS__)
  #define DBG_PRINTLN(...) Serial.println(__VA_ARGS__)
  #define DBG_PRINTF(...)  Serial.printf(__VA_ARGS__)
#else
  #define DBG_BEGIN(...)   do {} while (0)
  #define DBG_PRINT(...)   do {} while (0)
  #define DBG_PRINTLN(...) do {} while (0)
  #define DBG_PRINTF(...)  do {} while (0)
#endif

// ============================================================
// Human-friendly helpers
// ============================================================

static const char* locationToString(uint8_t loc) {
  switch (loc) {
    case LOCATION_ATTIC:      return "Attic";
    case LOCATION_CRAWLSPACE: return "Crawlspace";
    case LOCATION_BASEMENT:   return "Basement";
    case LOCATION_OTHER:      return "Other";
    default:                  return "Other";
  }
}

// ============================================================
// Configuration
// ============================================================

// BLE identity
static const char*  kDeviceName    = "StructureNode";
static const uint16_t kMfgCompanyID = 0xFFFF;   // TODO: replace with your assigned ID if you have one
static const uint16_t kProtocolV4   = 0x0004;

// Timing
static const uint32_t kAdvertiseBurstMs = 1200;        // How long to advertise before stopping
static const uint32_t kSleepMs          = 15UL * 1000; // Current: 15 seconds (TODO: set to 15 min if intended)

// DIP stability / debouncing (optional future enhancement)
static const uint16_t kDipPollMs   = 50;
static const uint16_t kDipStableMs = 250;

// DIP pins (2-bit location). Using INPUT_PULLUP:
// switch ON ties pin to GND => digitalRead() == LOW => logical bit = 1.
static const int kDipBit0Pin = 0;  // D0 -> bit0 (LSB)
static const int kDipBit1Pin = 1;  // D1 -> bit1 (MSB)

// Motion pins
// TODO: confirm these are correct for your wiring / board variant
static const int kMotion0Pin = 9;
static const int kMotion1Pin = 10;

// Battery sense
// TODO: confirm divider ratio and ADC reference for your board + wiring
static const int   kBatteryPin          = A0;
static const float kAdcRefVolts         = 3.3f;
static const float kBatteryDividerRatio = 2.0f;  // VBAT/2 -> ADC

// Flags
static const uint16_t FLAG_HTS_OK  = 1 << 0; // temp + humidity valid
static const uint16_t FLAG_LPS_OK  = 1 << 1; // pressure valid
static const uint16_t FLAG_BATT_OK = 1 << 2; // battery reading valid

// ============================================================
// Globals (minimal + obvious)
// ============================================================

static uint16_t gSeq = 0;
static uint8_t  gLocation = LOCATION_OTHER;

// NOTE: These are counters only (not ISR-driven currently). If you later
// attach interrupts for motion, keep them volatile and update safely.
static uint16_t gMotion0Count = 0;
static uint16_t gMotion1Count = 0;

// Sensor status (kept for compatibility with your existing logic)
static bool gHtsOk = true;
static bool gLpsOk = true;

// ============================================================
// DIP read
// ============================================================

static uint8_t readLocationFromDip() {
  const int dip0 = digitalRead(kDipBit0Pin);
  const int dip1 = digitalRead(kDipBit1Pin);

  // INPUT_PULLUP means LOW => switch ON => bit=1
  const uint8_t b0 = (dip0 == LOW) ? 1 : 0;
  const uint8_t b1 = (dip1 == LOW) ? 1 : 0;

  uint8_t loc = (uint8_t)((b1 << 1) | b0);
  if (loc > LOCATION_OTHER) loc = LOCATION_OTHER;

#if SN_DEBUG && SN_DEBUG_DIP
  DBG_PRINTLN("---- DIP READ ----");
  DBG_PRINT("bit0: "); DBG_PRINTLN(b0);
  DBG_PRINT("bit1: "); DBG_PRINTLN(b1);
  DBG_PRINT("Dip Location = "); DBG_PRINTLN(loc);
  DBG_PRINTLN("------------------");
#endif

  return loc;
}

// ============================================================
// Utility: clamp helpers
// ============================================================

static uint16_t clampU16(int32_t v) {
  if (v < 0) return 0;
  if (v > 65535) return 65535;
  return (uint16_t)v;
}

static int16_t clampI16(int32_t v) {
  if (v < -32768) return -32768;
  if (v > 32767)  return 32767;
  return (int16_t)v;
}

// ============================================================
// Battery helpers
// ============================================================

static uint16_t readBatteryMilliVolts(bool& ok) {
  // On nRF52 cores, analogReadResolution(12) is supported.
  analogReadResolution(12);  // 0..4095
  delay(5);                  // ADC settle after power-up / mux switching

  ok = true;

  const int raw = analogRead(kBatteryPin);
  if (raw < 0) {
    ok = false;
    return 0;
  }

  // If core ever returns 10-bit values, this keeps it sane.
  const uint16_t maxCounts = (raw > 1023) ? 4095 : 1023;

  const float vAdc = (float)raw * (kAdcRefVolts / (float)maxCounts);
  const float vBat = vAdc * kBatteryDividerRatio;

  return clampU16((int32_t)(vBat * 1000.0f + 0.5f));
}

// Simple Li-ion voltage-to-% curve (rough; you can refine per cell)
static const struct { float v; uint8_t pct; } kBattCurve[] = {
  { 4.20f, 100 },
  { 4.10f, 90  },
  { 4.00f, 80  },
  { 3.92f, 70  },
  { 3.85f, 60  },
  { 3.79f, 50  },
  { 3.74f, 40  },
  { 3.70f, 30  },
  { 3.65f, 20  },
  { 3.55f, 10  },
  { 3.40f, 5   },
  { 3.30f, 2   },
  { 3.20f, 0   },
};

static uint8_t battPercentFromMilliVolts(uint16_t mv) {
  const float v = (float)mv / 1000.0f;
  if (v >= 4.20f) return 100;
  if (v <= 3.20f) return 0;

  for (size_t i = 0; i + 1 < (sizeof(kBattCurve) / sizeof(kBattCurve[0])); i++) {
    const float vA = kBattCurve[i].v;
    const float vB = kBattCurve[i + 1].v;
    const uint8_t pA = kBattCurve[i].pct;
    const uint8_t pB = kBattCurve[i + 1].pct;

    if (v <= vA && v >= vB) {
      const float t  = (vA - v) / (vA - vB);
      const float pf = (float)pA + ((float)pB - (float)pA) * t;
      int p = (int)(pf + 0.5f);
      if (p < 0) p = 0;
      if (p > 100) p = 100;
      return (uint8_t)p;
    }
  }

  return 0;
}

// ============================================================
// Dew point helper (Magnus formula)
// ============================================================

static float dewPointC(float tempC, float rhPct) {
  // Clamp RH to avoid log(0) and crazy values
  if (rhPct < 1.0f)   rhPct = 1.0f;
  if (rhPct > 100.0f) rhPct = 100.0f;

  const float a = 17.62f;
  const float b = 243.12f;

  const float gamma = (a * tempC) / (b + tempC) + logf(rhPct / 100.0f);
  return (b * gamma) / (a - gamma);
}

// ============================================================
// Motion sampling
// ============================================================

static void sampleMotionPinsAndAccumulate() {
  // Typical PIR output is push-pull HIGH/LOW (active HIGH).
  if (digitalRead(kMotion0Pin)) gMotion0Count++;
  if (digitalRead(kMotion1Pin)) gMotion1Count++;
}

// ============================================================
// Sensor init
// ============================================================

static void initSensors() {
  Wire.begin();
  Wire.setClock(100000); // 100kHz is usually stable for breadboards/long jumpers

  // Optional internal pullups (board may already have them)
  pinMode(PIN_WIRE_SDA, INPUT_PULLUP);
  pinMode(PIN_WIRE_SCL, INPUT_PULLUP);

  // NOTE: Your comment said "Try 0x77 if this fails" but you're already using 0x77.
  // If your module is 0x76, change it here.
  if (!bme.begin(0x77)) {
    DBG_PRINTLN("Could not find BME680 at 0x77!");
    while (1) { delay(100); }
  }

  // Recommended oversampling + filter settings
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
}

// ============================================================
// BLE init + advertise helpers
// ============================================================

static void initBle() {
  Bluefruit.begin();
  Bluefruit.setName(kDeviceName);

  // NOTE: Set this once. You can still override per-advertise if desired.
  Bluefruit.setTxPower(0);

  // If you later add services, do it here.
}

static void advertisePayload(const StructurePayload& p) {
  // If you want a stronger burst TX power, do it right before advertising
  Bluefruit.setTxPower(8);

  // Manufacturer data format:
  // [0..1] = Company ID (little-endian)
  // [2..]  = your payload bytes
  uint8_t buf[2 + sizeof(StructurePayload)];
  buf[0] = (uint8_t)(kMfgCompanyID & 0xFF);
  buf[1] = (uint8_t)((kMfgCompanyID >> 8) & 0xFF);
  memcpy(buf + 2, &p, sizeof(p));

#if SN_DEBUG
  DBG_PRINTF("ADV mfg len=%d head: %02X %02X %02X %02X %02X %02X %02X %02X\n",
             (int)sizeof(buf),
             buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
#endif

  Bluefruit.Advertising.stop();
  Bluefruit.Advertising.clearData();
  Bluefruit.ScanResponse.clearData();

  // IMPORTANT: You previously noted addFlags() grows the payload and broke things.
  // Keep manufacturer data lean; use scan response for the name.
  Bluefruit.Advertising.addManufacturerData(buf, sizeof(buf));
  Bluefruit.ScanResponse.addName();

  // Interval: 160 * 0.625ms = 100ms
  Bluefruit.Advertising.setInterval(160, 160);

  // start(0) = advertise forever (until we stop it)
  Bluefruit.Advertising.start(0);
}

static void stopAdvertising() {
  Bluefruit.Advertising.stop();
}

// ============================================================
// Build payload
// ============================================================

static StructurePayload buildPayload() {
  StructurePayload p{};
  p.protocol  = kProtocolV4;
  p.location  = gLocation;
  p.flags     = 0;
  p.seq       = gSeq++;

  float tempC = NAN;
  float rhPct = NAN;
  float pressPa = NAN;

  // Read temp/humidity
  if (gHtsOk) {
    tempC = bme.readTemperature();
    rhPct = bme.readHumidity();

    // Preserve your prior behavior (offset calibration)
    if (!isnan(tempC)) tempC -= 2.0f;

    if (!isnan(tempC) && !isnan(rhPct)) p.flags |= FLAG_HTS_OK;
  }

  // Read pressure
  if (gLpsOk) {
    pressPa = bme.readPressure();
    if (!isnan(pressPa) && pressPa > 0.0f) {
      p.flags |= FLAG_LPS_OK;
    }
  }

  // Battery
  bool battOk = false;
  const uint16_t battmV = readBatteryMilliVolts(battOk);
  if (battOk) p.flags |= FLAG_BATT_OK;

  p.batt_mV  = battmV;
  p.battPct  = battPercentFromMilliVolts(battmV);
  p.rsv0     = 0;
  p.uptimeMin = (uint16_t)((millis() / 60000UL) & 0xFFFF);

  // Dew point
  const float dpC = (!isnan(tempC) && !isnan(rhPct)) ? dewPointC(tempC, rhPct) : NAN;

  // Fixed-point packing
  const int32_t t_x100  = isnan(tempC) ? 0 : (int32_t)(tempC * 100.0f);
  const int32_t rh_x100 = isnan(rhPct) ? 0 : (int32_t)(rhPct * 100.0f);

  // Convert Pa -> hPa*10:
  // hPa = Pa / 100
  // hPa*10 = Pa / 10
  const int32_t press_hPa_x10 = (isnan(pressPa) ? 0 : (int32_t)(pressPa / 10.0f));
  const int32_t dp_x100 = isnan(dpC) ? 0 : (int32_t)(dpC * 100.0f);

  p.tempC_x100      = clampI16(t_x100);
  p.humPct_x100     = clampU16(rh_x100);
  p.press_hPa_x10   = clampU16(press_hPa_x10);
  p.dewPointC_x100  = clampI16(dp_x100);

  // Motion accumulation (polled)
  sampleMotionPinsAndAccumulate();
  p.motion0 = gMotion0Count;
  p.motion1 = gMotion1Count;

  return p;
}

// ============================================================
// Debug prints
// ============================================================

static void dumpPayloadHex(const StructurePayload& p) {
#if SN_DEBUG && SN_DEBUG_PAYLOAD_HEX
  const uint8_t* b = reinterpret_cast<const uint8_t*>(&p);
  DBG_PRINT("RAW ");
  for (size_t i = 0; i < sizeof(StructurePayload); i++) {
    if (b[i] < 0x10) DBG_PRINT('0');
    DBG_PRINT(b[i], HEX);
    DBG_PRINT(' ');
  }
  DBG_PRINTLN();
#else
  (void)p;
#endif
}

static void printPayload(const StructurePayload& p) {
#if SN_DEBUG
  DBG_PRINT("SEQ ");  DBG_PRINT(p.seq);
  DBG_PRINT(" loc "); DBG_PRINT(locationToString(p.location));
  DBG_PRINT("(");     DBG_PRINT(p.location); DBG_PRINT(")");

  DBG_PRINT(" T ");   DBG_PRINT(p.tempC_x100 / 100.0f, 2);
  DBG_PRINT(" RH ");  DBG_PRINT(p.humPct_x100 / 100.0f, 2);
  DBG_PRINT(" P ");   DBG_PRINT(p.press_hPa_x10 / 10.0f, 1);

  DBG_PRINT(" mV ");  DBG_PRINT(p.batt_mV);
  DBG_PRINT(" pct "); DBG_PRINT(p.battPct);
  DBG_PRINT(" upm "); DBG_PRINT(p.uptimeMin);

  DBG_PRINT(" DP ");  DBG_PRINT(p.dewPointC_x100 / 100.0f, 2);
  DBG_PRINT(" M0 ");  DBG_PRINT(p.motion0);
  DBG_PRINT(" M1 ");  DBG_PRINT(p.motion1);

  DBG_PRINT(" flags 0x"); DBG_PRINT(p.flags, HEX);
  DBG_PRINT(" proto 0x"); DBG_PRINTLN(p.protocol, HEX);

  dumpPayloadHex(p);
#else
  (void)p;
#endif
}

// ============================================================
// Sleep (placeholder)
// ============================================================

static void saveBattery() {
  // NOTE: This is a simple delay. For true low-power you’d typically:
  // - stop peripherals
  // - use system OFF / deep sleep APIs
  // - or rely on a TPL5110/5111 to cut power
  delay(kSleepMs);
}

// ============================================================
// Compile-time safety checks
// ============================================================

// BLE legacy advertising payload is typically 31 bytes total.
// Manufacturer data adds 2 bytes company ID + our payload.
// Keep payload <= 29 so companyId(2) + payload(<=29) <= 31.
static_assert((2 + sizeof(StructurePayload)) <= 31,
              "Manufacturer data too large for 31-byte legacy advertising payload.");

// ============================================================
// Arduino setup/loop
// ============================================================

void setup() {
  DBG_BEGIN(115200);
  delay(1000);

  // Motion pins (active high)
  pinMode(kMotion0Pin, INPUT);
  pinMode(kMotion1Pin, INPUT);

  // DIP pins (active low due to pullups)
  pinMode(kDipBit0Pin, INPUT_PULLUP);
  pinMode(kDipBit1Pin, INPUT_PULLUP);

  // Sample motion immediately on boot (useful if a wake source held the line high)
  sampleMotionPinsAndAccumulate();

  // Read DIP-selected location
  gLocation = readLocationFromDip();

  // Sensors + BLE
  initSensors();
  initBle();

#if SN_DEBUG
  DBG_PRINTLN("StructureNode Ready.");
#endif
}

void loop() {
  const StructurePayload payload = buildPayload();

  printPayload(payload);

  advertisePayload(payload);
  delay(kAdvertiseBurstMs);
  stopAdvertising();

  saveBattery();
}