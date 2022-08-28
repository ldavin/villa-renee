#include <Arduino.h>
#include <LowPower.h>
#include <RHReliableDatagram.h>
#include <RH_RF69.h>
#include "encryption_key.h"

#define RadioGatewayNodeId 1
#define RadioNodeId 3
#define RadioMinPower -2
#define RadioMaxPower 10


#ifdef BoardIsMoteino

#include <SPIFlash.h>

#define InterruptPin PIND3

RH_RF69 driver;
SPIFlash flash(SS_FLASHMEM, 0xEF30);
#endif

#ifdef BoardIsFeather

#define InterruptPin PIND0

RH_RF69 driver(8, 7);
#endif

RHReliableDatagram radio(driver, RadioNodeId);
volatile long pulseReceived = 0;
long sendErrorsCount = 0;
long sendErrorsCountAtCurrentRadioPower = 0;
long sendSuccessCountAtCurrentRadioPower = 0;
int radioPower = RadioMinPower;

void blinkFor(int durationMillis);

void readPulse();

void disableFlashChip();

void sendData();

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(InterruptPin, INPUT_PULLUP);

    radio.init();
    radio.setRetries(5);
    driver.setFrequency(868.0);
    driver.setTxPower(radioPower, true);
    driver.setEncryptionKey(encryptionKey);
    driver.sleep();

    disableFlashChip();

    int interruptNum = digitalPinToInterrupt(InterruptPin);
    attachInterrupt(interruptNum, readPulse, RISING);

    for (int i = 0; i < 10; ++i) {
        blinkFor(20);
    }
}

void loop() {
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);

    if (pulseReceived > 0) {
        sendData();
    }
}

void readPulse() {
    pulseReceived++;
}

void sendData() {
    noInterrupts();
    long pulseReceivedCopy = pulseReceived;
    pulseReceived = 0;
    interrupts();

    String dataString;
    dataString.reserve(10);
    dataString.concat(pulseReceivedCopy);
    dataString.concat(",");
    dataString.concat(sendErrorsCount);
    dataString.concat(",");
    dataString.concat(radioPower);
    dataString.concat(".");

    char data[dataString.length()];
    dataString.toCharArray(data, sizeof(data));

    if (radio.sendtoWait((uint8_t *) data, sizeof(data), RadioGatewayNodeId)) {
        sendErrorsCount = 0;
        blinkFor(50);

        sendSuccessCountAtCurrentRadioPower++;
        if (sendSuccessCountAtCurrentRadioPower >= 50) {
            sendSuccessCountAtCurrentRadioPower = 0;
            sendErrorsCountAtCurrentRadioPower = 0;
            if (radioPower > RadioMinPower) {
                radioPower--;
                driver.setTxPower(radioPower, true);
                for (int i = 0; i < 3; ++i) {
                    blinkFor(20);
                }
            }
        }
    } else {
        noInterrupts();
        pulseReceived += pulseReceivedCopy;
        interrupts();
        sendErrorsCount++;
        for (int i = 0; i < 3; ++i) {
            blinkFor(20);
        }

        sendErrorsCountAtCurrentRadioPower++;
        sendSuccessCountAtCurrentRadioPower = 0;
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

void blinkFor(int durationMillis) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(durationMillis);
    digitalWrite(LED_BUILTIN, LOW);
    delay(durationMillis);
}