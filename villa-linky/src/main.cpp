#include <Arduino.h>
#include <RHReliableDatagram.h>
#include <RH_RF69.h>
#include <Parser.h>
#include "encryption_key.h"

#define startFrame 0x02
#define endFrame 0x03

#define RadioGatewayNodeId 1
#define RadioNodeId 2
#define RadioMinPower -2
#define RadioMaxPower 10

#ifdef BoardIsMoteino

#include <LowPower.h>
#include <SPIFlash.h>

#define ReadPin A7
#define AcquisitionSerial Serial

RH_RF69 driver;
SPIFlash flash(SS_FLASHMEM, 0xEF30);
#endif

#ifdef BoardIsFeather

#include <Adafruit_SleepyDog.h>

#define ReadPin A5
#define AcquisitionSerial Serial1

RH_RF69 driver(8, 7);
#endif

RHReliableDatagram radio(driver, RadioNodeId);
Parser parser;
long sendErrorsCount = 0;
long sendErrorsCountAtCurrentRadioPower = 0;
int radioPower = RadioMinPower;

void blinkFor(int durationMillis);

void sendData(unsigned long whReading, unsigned long apparentPower);

bool superCapacitorIsChargedEnough();

void powerDown();

void disableFlashChip();

void flushSerial();

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    AcquisitionSerial.begin(1200);
    pinMode(ReadPin, INPUT);

    radio.init();
    driver.setFrequency(868.0);
    driver.setTxPower(radioPower, true);
    driver.setEncryptionKey(encryptionKey);
    driver.sleep();

    disableFlashChip();

    for (int i = 0; i < 10; ++i) {
        blinkFor(20);
    }
}

void loop() {
    if (!superCapacitorIsChargedEnough()) {
        powerDown();
        blinkFor(1000);
        flushSerial();
        return;
    }

    char charIn = 0;

    while (charIn != startFrame) {
        // We skip the 8th bit per spec
        charIn = AcquisitionSerial.read() & 0x7F;
    }

    // Start of frame capture
    while (charIn != endFrame) {
        if (AcquisitionSerial.available()) {
            charIn = AcquisitionSerial.read() & 0x7F;
            parser.feed(charIn);
        }
    }

    if (parser.dataIsAvailable()) {
        unsigned long whReading = parser.getWhReading();
        unsigned long apparentPower = parser.getApparentPower();
        sendData(whReading, apparentPower);
        flushSerial();
    }
    parser.reset();

    // End of complete frame capture
}

bool superCapacitorIsChargedEnough() {
    int read1 = analogRead(ReadPin);
    delay(1);
    int read2 = analogRead(ReadPin);
    delay(1);
    int read3 = analogRead(ReadPin);
    int read = (read1 + read2 + read3) / 3;
    // Input of a voltage divider bridge on a 3.3V board
    float voltage = read * 2 * 3.3f / 1023;
    return voltage >= 3.7;
}

void sendData(unsigned long whReading, unsigned long apparentPower) {
    String dataString;
    dataString.reserve(20);
    dataString.concat(whReading);
    dataString.concat(",");
    dataString.concat(apparentPower);
    dataString.concat(",");
    dataString.concat(sendErrorsCount);

    char data[dataString.length()];
    dataString.toCharArray(data, sizeof(data));

    if (radio.sendtoWait((uint8_t *) data, sizeof(data), RadioGatewayNodeId)) {
        sendErrorsCount = 0;
        blinkFor(50);
    } else {
        sendErrorsCount++;
        for (int i = 0; i < 3; ++i) {
            blinkFor(20);
        }

        sendErrorsCountAtCurrentRadioPower++;
        if (sendErrorsCountAtCurrentRadioPower >= 5) {
            sendErrorsCountAtCurrentRadioPower = 0;
            if (radioPower < RadioMaxPower) {
                radioPower++;
                driver.setTxPower(radioPower, true);
                for (int i = 0; i < 3; ++i) {
                    blinkFor(20);
                }
            }
        }
    }
    driver.sleep();
}

void disableFlashChip() {
#ifdef BoardIsMoteino
    if (flash.initialize()) {
        flash.sleep();
    }
#endif
}

void powerDown() {
#ifdef BoardIsMoteino
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
#endif
#ifdef BoardIsFeather
    Watchdog.sleep(8000);
#endif
}

void blinkFor(int durationMillis) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(durationMillis);
    digitalWrite(LED_BUILTIN, LOW);
    delay(durationMillis);
}

void flushSerial() {
    while (AcquisitionSerial.available() > 0) {
        AcquisitionSerial.read();
    }
}
