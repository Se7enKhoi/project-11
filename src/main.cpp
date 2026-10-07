#include <Arduino.h>




const uint8_t LDR_PIN = A0;      // PC0-A0


const uint8_t LED_PIN = 12;      // PB4-12


const uint8_t SEG_A  = 2;   // PD2
const uint8_t SEG_B  = 3;   // PD3
const uint8_t SEG_C  = 4;   // PD4
const uint8_t SEG_D  = 5;   // PD5
const uint8_t SEG_E  = 6;   // PD6
const uint8_t SEG_F  = 7;   // PD7
const uint8_t SEG_G  = A1;  // PC1
const uint8_t SEG_DP = A2;  // PC2
const uint8_t SEG_PINS[8] = {SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F, SEG_G, SEG_DP};

const uint8_t DIGIT_PINS[4] = {8, 9, 10, 11}; 

const bool SEGMENT_ON = HIGH;  
const bool DIGIT_ON   = HIGH;  


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


int DARK_THRESHOLD = 500;


unsigned long lastDigitSwitch = 0;
const unsigned long DIGIT_INTERVAL_MS = 3;  
uint8_t activeDigit = 0;

unsigned long lastSample = 0;
const unsigned long SAMPLE_INTERVAL_MS = 500;

int adcValue = 0;
int displayValue = 0; 

void showDigit(uint8_t digitIndex, uint8_t value) {
  
  for (uint8_t i = 0; i < 4; i++) {
    digitalWrite(DIGIT_PINS[i], !DIGIT_ON);
  }

  for (uint8_t s = 0; s < 8; s++) {
    bool on = DIGIT_TABLE[value][s];
    digitalWrite(SEG_PINS[s], on ? SEGMENT_ON : !SEGMENT_ON);
  }
 
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


  if (now - lastSample >= SAMPLE_INTERVAL_MS) {
    lastSample = now;

    adcValue = analogRead(LDR_PIN);  
    displayValue = adcValue;

    bool isDark = (adcValue < DARK_THRESHOLD);
    digitalWrite(LED_PIN, isDark ? HIGH : LOW);

    Serial.print(F("ADC: "));
    Serial.print(adcValue);
    Serial.print(F("  STATUS: "));
    Serial.println(isDark ? F("DARK - LED ON") : F("BRIGHT - LED OFF"));
  }

  
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
