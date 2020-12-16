#include <EEPROM.h>

#include "EMA.h"

#define EEPROM_ADDR 0x77

#define READ_INTERVAL 1
#define SERIAL_INTERVAL 100

#define LONG_EMA_LENGTH 60000
#define SHORT_EMA_LENGTH 50

#define PIN_PUMP_SW 2
#define PIN_PUMP_MOTO 3
#define PIN_RELEASE_SW 4
#define PIN_RELEASE_ACT 5
#define PIN_VIB_MOTO LED_BUILTIN

#define MAX_ANALOG 1024
#define DEFAULT_CUTOFF_THRESHOLD 10

#define MIN_CUTOFF_MILLIS 5000

uint16_t cutoffThreshold = 10;

EMAFloat longEMA;
EMAFloat shortEMA;

int currentReading;

int pumpValue = 0;
int releaseActValue = LOW;
int motorValue = 0;
int motorValueOld = UINT8_MAX;

unsigned long lastSerial = 0;
unsigned long lastRead = 0;
unsigned long lastCut = 0;

int readCurrent() {
    return analogRead(A0);
}

void writeAct() {
    analogWrite(PIN_PUMP_MOTO, pumpValue);
    digitalWrite(PIN_RELEASE_ACT, releaseActValue);
    analogWrite(PIN_VIB_MOTO, motorValue);
}

void persistConf() {
    EEPROM.put(EEPROM_ADDR, cutoffThreshold);
    Serial.println(String("$CL EE,TH,") + EEPROM_ADDR + String(",") + cutoffThreshold);
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(PIN_PUMP_SW, INPUT_PULLUP);
    pinMode(PIN_RELEASE_SW, INPUT_PULLUP);
    pinMode(PIN_PUMP_MOTO, OUTPUT);
    pinMode(PIN_RELEASE_ACT, OUTPUT);

    motorValue = motorValueOld;

    writeAct();

    // firstRead
    currentReading = readCurrent();
    lastRead = millis();
    longEMA = EMAFloat(LONG_EMA_LENGTH, currentReading);
    shortEMA = EMAFloat(SHORT_EMA_LENGTH, currentReading);

    Serial.begin(115200);
    Serial.setTimeout(1);  // #A1234Z
    lastSerial = millis();

    EEPROM.get(EEPROM_ADDR, cutoffThreshold);
    if (cutoffThreshold > MAX_ANALOG || cutoffThreshold < 0) {
        cutoffThreshold = DEFAULT_CUTOFF_THRESHOLD;
        persistConf();
    }
}

void loop() {
    // Commit change
    writeAct();

    unsigned long currentTs = millis();
    // Sersor
    if (currentTs - lastRead >= READ_INTERVAL) {
        currentReading = readCurrent();
        longEMA.update(currentReading, currentTs - lastRead);
        shortEMA.update(currentReading, currentTs - lastRead);
        lastRead = currentTs;
    }

    // Serial
    if (currentTs - lastSerial >= SERIAL_INTERVAL) {
        Serial.println(
            String("$DT ") + currentTs + String(",") + lastRead + String(",") + cutoffThreshold + String(",") +
            currentReading + String(",") + shortEMA.get() + String(",") + longEMA.get() + String(",") +
            pumpValue + String(",") + releaseActValue + String(",") + motorValue);
        lastSerial = currentTs;
    }
    if (Serial.available()) {
        int cmd = Serial.read();
        // Threshold
        if (cmd == 'w') {
            if (cutoffThreshold < MAX_ANALOG) {
                cutoffThreshold++;
            }
            Serial.println(String("$CT TH,1,") + cutoffThreshold);
        }
        if (cmd == 's') {
            if (cutoffThreshold > 0) {
                cutoffThreshold--;
            }
            Serial.println(String("$CT TH,0,") + cutoffThreshold);
        }
        // pump
        if (cmd == 'e') {
            pumpValue = UINT8_MAX;
            Serial.println(String("$CT PU,1,") + pumpValue);
        }
        if (cmd == 'd') {
            pumpValue = 0;
            Serial.println(String("$CT PU,0,") + pumpValue);
        }
        // release
        if (cmd == 'r') {
            releaseActValue = LOW;
            Serial.println(String("$CT RL,1,") + releaseActValue);
        }
        if (cmd == 'f') {
            releaseActValue = HIGH;
            Serial.println(String("$CT RL,0,") + releaseActValue);
        }
        // vib
        if (cmd == 'q') {
            if (motorValue < UINT8_MAX) {
                motorValue++;
            }
            Serial.println(String("$CT MO,1,") + motorValue);
        }
        if (cmd == 'a') {
            if (motorValue > 0) {
                motorValue--;
                motorValueOld = motorValue;
            }
            Serial.println(String("$CT MO,0,") + motorValue);
        }
        if (cmd == '`') {
            Serial.println(String("$CT EE,1,") + cutoffThreshold);
            persistConf();
        }
    }

    // Motor
    if (shortEMA.get() - longEMA.get() > cutoffThreshold) {
        if (motorValue != 0) {
            motorValueOld = motorValue;
            motorValue = 0;
            lastCut = millis();
            Serial.println(String("$CL MO,0,") + millis() + String(",") + lastCut + String(",") + shortEMA.get() + String(",") + longEMA.get() + String(",") + currentReading);
        }
    } else {
        // Restart
        if (millis() - lastCut > MIN_CUTOFF_MILLIS && motorValue == 0) {
            motorValue = motorValueOld;
            Serial.println(String("$CL MO,1,") + millis() + String(",") + lastCut + String(",") + shortEMA.get() + String(",") + longEMA.get() + String(",") + currentReading);
        }
    }

    // // Pump
    // if (!digitalRead(PIN_PUMP_SW)) {
    //     pumpValue = UINT8_MAX;
    // } else {
    //     pumpValue = 0;
    // }

    // // Release
    // if (!digitalRead(PIN_RELEASE_SW)) {
    //     releaseActValue = HIGH;
    // } else {
    //     releaseActValue = LOW;
    // }
}