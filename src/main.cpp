#include <Arduino.h>

// =====================================================================
// Practice 11 - Ambient light measurement with LDR + 4-digit 7-segment
// =====================================================================
//
// Wiring (kit "TDC-3628"):
//   CB ánh sáng "AO" (2-pin)      -> PC0-A0            (ADC input)
//   LED (Led đơn L1, or similar)  -> PB4-12             (dark indicator)
//   Led 7 đoạn header "A..H"      -> segments (see below)
//   Led 7 đoạn header "Q1..Q4"    -> digit enable (see below)
//
// The 7-segment module on this kit is wired DIRECTLY (no shift
// register / BCD driver chip) - 8 segment lines + 4 digit-select
// lines = 12 wires total. PD0/PD1 are avoided because they are the
// UART Rx/Tx pins used for Serial.
// =====================================================================

// ---- LDR analog input ----
const uint8_t LDR_PIN = A0;      // PC0-A0

// ---- Dark-indicator LED/buzzer ----
const uint8_t LED_PIN = 12;      // PB4-12

// ---- 7-segment segment pins: a,b,c,d,e,f on PORTD, g,dp on PORTC ----
const uint8_t SEG_A  = 2;   // PD2
const uint8_t SEG_B  = 3;   // PD3
const uint8_t SEG_C  = 4;   // PD4
const uint8_t SEG_D  = 5;   // PD5
const uint8_t SEG_E  = 6;   // PD6
const uint8_t SEG_F  = 7;   // PD7
const uint8_t SEG_G  = A1;  // PC1
const uint8_t SEG_DP = A2;  // PC2
const uint8_t SEG_PINS[8] = {SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F, SEG_G, SEG_DP};

// ---- Digit-enable pins (Q1..Q4 transistors) ----
const uint8_t DIGIT_PINS[4] = {8, 9, 10, 11}; // PB0-8, PB1-9, PB2-10, PB3-11

// If the display shows garbled/inverted segments or the wrong digit
// lights up, flip one or both of these two constants:
const bool SEGMENT_ON = HIGH;  // logic level that lights a segment
const bool DIGIT_ON   = HIGH;  // logic level that enables a digit

// Segment patterns for digits 0-9: order matches SEG_PINS {a,b,c,d,e,f,g,dp}
const bool DIGIT_TABLE[10][8] = {
  {1,1,1,1,1,1,0,0}, // 0
  {0,1,1,0,0,0,0,0}, // 1
  {1,1,0,1,1,0,1,0}, // 2
  {1,1,1,1,0,0,1,0}, // 3
  {0,1,1,0,0,1,1,0}, // 4
  {1,0,1,1,0,1,1,0}, // 5
  {1,0,1,1,1,1,1,0}, // 6
  {1,1,1,0,0,0,0,0}, // 7
  {1,1,1,1,1,1,1,0}, // 8
  {1,1,1,1,0,1,1,0}, // 9
};

// ---- Light threshold: TUNE this after watching the ADC values ----
// printed over Serial in bright vs. covered/dark conditions.
int DARK_THRESHOLD = 500;

// ---- Timing (non-blocking so the display never stops multiplexing) ----
unsigned long lastDigitSwitch = 0;
const unsigned long DIGIT_INTERVAL_MS = 3;   // ~3 ms/digit -> flicker-free
uint8_t activeDigit = 0;

unsigned long lastSample = 0;
const unsigned long SAMPLE_INTERVAL_MS = 500;

int adcValue = 0;
int displayValue = 0; // 0-1023, fits exactly in 4 digits

void showDigit(uint8_t digitIndex, uint8_t value) {
  // Blank all digits first to avoid ghosting between digits
  for (uint8_t i = 0; i < 4; i++) {
    digitalWrite(DIGIT_PINS[i], !DIGIT_ON);
  }
  // Load the segment pattern for this digit's value
  for (uint8_t s = 0; s < 8; s++) {
    bool on = DIGIT_TABLE[value][s];
    digitalWrite(SEG_PINS[s], on ? SEGMENT_ON : !SEGMENT_ON);
  }
  // Enable only this digit
  digitalWrite(DIGIT_PINS[digitIndex], DIGIT_ON);
}

void setup() {
  Serial.begin(9600);

  pinMode(LDR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  for (uint8_t i = 0; i < 8; i++) {
    pinMode(SEG_PINS[i], OUTPUT);
    digitalWrite(SEG_PINS[i], !SEGMENT_ON);
  }
  for (uint8_t i = 0; i < 4; i++) {
    pinMode(DIGIT_PINS[i], OUTPUT);
    digitalWrite(DIGIT_PINS[i], !DIGIT_ON);
  }

  Serial.println(F("READY - Light intensity monitor started"));
}

void loop() {
  unsigned long now = millis();

  // ---- 1) Sample the ADC, drive the LED, report over UART ----
  if (now - lastSample >= SAMPLE_INTERVAL_MS) {
    lastSample = now;

    adcValue = analogRead(LDR_PIN);   // 0-1023
    displayValue = adcValue;

    bool isDark = (adcValue < DARK_THRESHOLD);
    digitalWrite(LED_PIN, isDark ? HIGH : LOW);

    Serial.print(F("ADC: "));
    Serial.print(adcValue);
    Serial.print(F("  STATUS: "));
    Serial.println(isDark ? F("DARK - LED ON") : F("BRIGHT - LED OFF"));
  }

  // ---- 2) Keep multiplexing the 4-digit display (must run continuously) ----
  if (now - lastDigitSwitch >= DIGIT_INTERVAL_MS) {
    lastDigitSwitch = now;

    int v = displayValue;
    uint8_t digits[4];
    digits[0] = (v / 1000) % 10;
    digits[1] = (v / 100)  % 10;
    digits[2] = (v / 10)   % 10;
    digits[3] = v % 10;

    showDigit(activeDigit, digits[activeDigit]);
    activeDigit = (activeDigit + 1) % 4;
  }
}