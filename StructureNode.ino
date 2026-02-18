/*

StructureNode is a low-power, BLE-based environmental monitoring node designed 
for residential attic and structural health tracking. 

Built for DIY developers, it measures temperature, humidity, pressure, dew point, 
battery status, and motion, then broadcasts the data using a compact custom BLE 
advertising payload. 

The device supports hardware-selectable location IDs via DIP switches, deep 
sleep for extended battery life, and seamless integration with a Raspberry 
Pi + Django backend (StructureHub) for logging, analysis, and risk visualization. 

Ideal for breadboard prototyping and long-term home deployment.

*/

// ============================================================
// MUST BE ABOVE ALL #includes (Arduino auto-prototype workaround)
// ============================================================

enum SensorLocation : unsigned char {
  LOCATION_ATTIC      = 0,
  LOCATION_CRAWLSPACE = 1,
  LOCATION_BASEMENT   = 2,
  LOCATION_OTHER      = 3
};

struct __attribute__((packed)) StructurePayloadV4 {
  unsigned short protocol;        // 0x0004
  unsigned char  location;        // 0..3 from DIP (SensorLocation)
  short          tempC_x100;      // signed
  unsigned short humPct_x100;     // % * 100
  unsigned short press_hPa_x10;   // hPa * 10
  unsigned short batt_mV;         // mV
  unsigned short flags;           // bitfield
  unsigned short seq;             // counter
  unsigned short motion0;         // wraps
  unsigned short motion1;         // wraps
  unsigned char  battPct;         // 0-100
  unsigned char  rsv0;            // reserved (0)
  unsigned short uptimeMin;       // wraps
  short          dewPointC_x100;  // signed
};

// ============================================================
// Includes
// ============================================================

#include <Arduino.h>
#include <Wire.h>
#include <math.h>

#include <bluefruit.h>

#include <Adafruit_SHT31.h>
#include <Adafruit_BMP280.h>

// ============================================================
// Strings / helpers
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

static const char*    kDeviceName          = "StructureNode";
static const uint16_t kMfgCompanyID        = 0xFFFF;   // your chosen company ID
static const uint16_t kProtocolV4          = 0x0004;

static const uint32_t kAdvertiseBurstMs    = 1200;
static const uint32_t kSleepMs             = 15UL * 60UL * 1000UL;

static const uint32_t kDipOverrideWindowMs = 5000;
static const uint16_t kDipPollMs           = 50;
static const uint16_t kDipStableMs         = 250;

// DIP pins (2-bit location)
static const int kDipBit0Pin = 9; // D9 -> bit0 (LSB)
static const int kDipBit1Pin = 10; // D10 -> bit1 (MSB)

// Motion pins - NEED FIX

static const int kMotion0Pin = 5;
static const int kMotion1Pin = 6;

// Battery read - NEED FIX

static const int   kBatteryPin          = A6;
static const float kAdcRefVolts         = 3.3f;
static const float kBatteryDividerRatio = 2.0f; // VBAT/2 -> A6

// LED pattern - NEED FIX

static const int kStatusLedPin   = LED_BUILTIN;
static const uint16_t kBlinkOnMs = 180;
static const uint16_t kBlinkOffMs = 180;
static const uint16_t kGroupPauseMs = 700;

// Flags
static const uint16_t FLAG_HTS_OK  = 1 << 0;
static const uint16_t FLAG_LPS_OK  = 1 << 1;
static const uint16_t FLAG_BATT_OK = 1 << 2;

// ============================================================
// Globals
// ============================================================

static uint16_t gSeq = 0;
static uint8_t  gLocation = LOCATION_OTHER;

volatile uint16_t gMotion0Count = 0;
volatile uint16_t gMotion1Count = 0;

// Sensors
static bool gHtsOk = false;
static bool gLpsOk = false;

static Adafruit_SHT31  gSht31 = Adafruit_SHT31();
static Adafruit_BMP280 gBmp;

// ============================================================
// DIP read
// ============================================================
// INPUT_PULLUP: switch ON ties to GND => pin LOW => bit=1

static uint8_t readLocationFromDipPins(int& rawD7, int& rawD8) {
  rawD7 = digitalRead(kDipBit0Pin);
  rawD8 = digitalRead(kDipBit1Pin);

    Serial.println("---- DIP READ ----");
    Serial.print("D7 raw: "); Serial.print(rawD7);
    Serial.print(" -> bit0: "); Serial.println((rawD7 == LOW) ? 1 : 0);
    Serial.print("D8 raw: "); Serial.print(rawD8);
    Serial.print(" -> bit1: "); Serial.println((rawD8 == LOW) ? 1 : 0);


  uint8_t b0 = (rawD7 == LOW) ? 1 : 0;
  uint8_t b1 = (rawD8 == LOW) ? 1 : 0;
  b0 = 1;

    Serial.print("bit0: "); Serial.println(b0);
    Serial.print("bit1: "); Serial.println(b1);

  uint8_t loc = (uint8_t)((b1 << 1) | b0);
  if (loc > 3) loc = LOCATION_OTHER;
  return loc;
}

static uint8_t readLocationFromDip(bool printDebug) {
  pinMode(kDipBit0Pin, INPUT_PULLUP);
  pinMode(kDipBit1Pin, INPUT_PULLUP);
  delay(10);

  int raw7 = HIGH, raw8 = HIGH;
  uint8_t loc = readLocationFromDipPins(raw7, raw8);

  if (true) {
    Serial.println("---- DIP READ ----");
    Serial.print("D7 raw: "); Serial.print(raw7);
    Serial.print(" -> bit0: "); Serial.println((raw7 == LOW) ? 1 : 0);
    Serial.print("D8 raw: "); Serial.print(raw8);
    Serial.print(" -> bit1: "); Serial.println((raw8 == LOW) ? 1 : 0);
    Serial.print("Location: "); Serial.print(loc);
    Serial.print(" ("); Serial.print(locationToString(loc)); Serial.println(")");
    Serial.println("------------------");
  }
  return loc;
}


// ============================================================
// Arduino setup/loop
// ============================================================

void setup() {
  Serial.begin(115200);
  delay(10000);

  Serial.println();
  Serial.println("StructureNode Booting...");

  // Ensure DIP pullups are on
  pinMode(kDipBit0Pin, INPUT_PULLUP);
  pinMode(kDipBit1Pin, INPUT_PULLUP);

  // Print initial DIP read + allow 5s override with LED pattern
  gLocation = readLocationFromDip(true);
  dipOverrideWindow();

  initSensors();
  initMotionInterrupts();

  Bluefruit.begin();
  Bluefruit.setName(kDeviceName);
  Bluefruit.setTxPower(0);

  Serial.println("BLE Ready.");
}

void loop() {
  // put your main code here, to run repeatedly:

}
