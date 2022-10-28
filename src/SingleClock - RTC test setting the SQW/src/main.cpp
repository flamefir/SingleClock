#include <Wire.h>
#include <Arduino.h>
#include <ErriezDS1307.h>

#define DS1307_CTRL_ID 0x68
#define INT_PIN 4
#define ledPin 2

// Create DS1307 RTC object
ErriezDS1307 rtc;

// SQW interrupt flag must be volatile
volatile bool sqwInterrupt = false;

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
ICACHE_RAM_ATTR
#endif
void sqwHandler()
{
    // Set global interrupt flag
    sqwInterrupt = true;
}

void setup() {
    // Initialize serial port
    delay(500);
    Serial.begin(115200);
    while (!Serial) {
        ;
    }
    Serial.println(F("\nErriez DS1307 SQW blink example\n"));

    // Initialize TWI
    Wire.begin();
    Wire.setClock(400000);

    // Attach to INT0 interrupt falling edge
    pinMode(INT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(INT_PIN), sqwHandler, FALLING);

    // Initialize RTC
    while (!rtc.begin()) {
        Serial.println(F("RTC not found"));
        delay(3000);
    }

    // Enable RTC clock
    if (!rtc.isRunning()) {
        rtc.clockEnable();
    }

    // Set 1Hz square wave out:
    //      SquareWaveDisable
    //      SquareWave1Hz
    //      SquareWave4096Hz
    //      SquareWave8192Hz
    //      SquareWave32768Hz
    rtc.setSquareWave(SquareWave1Hz);

    // Initialize LED
    pinMode(ledPin, OUTPUT);
}

void loop() {
  // Wait for 1Hz square wave clock interrupt
  if (sqwInterrupt) {
      // Toggle LED
      Serial.println(F("Toggle LED"));
      digitalWrite(ledPin, !digitalRead(ledPin));

      // Clear interrupt flag when interrupt has been handled
      sqwInterrupt = false;
  }
}