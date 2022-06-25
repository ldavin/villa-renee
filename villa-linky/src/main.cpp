#include <Arduino.h>
#include <RHReliableDatagram.h>
#include <RH_RF69.h>
#include <Parser.h>

#define startFrame 0x02
#define endFrame 0x03

#define RadioGatewayNodeId 1
#define RadioNodeId 2
#define RadioNetworkId 100
#define RadioKey "sampleEncryptKey"

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

void blinkFor(int durationMillis);

void sendData(unsigned long whReading, unsigned long apparentPower);

bool superCapacitorIsChargedEnough();

void powerDown();

void disableFlashChip();

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    AcquisitionSerial.begin(1200);
    pinMode(ReadPin, INPUT);

    radio.init();
    driver.setFrequency(868.0);
    driver.setTxPower(5, true);
    driver.setEncryptionKey(); // TODO
    driver.sleep();

    disableFlashChip();

    delay(1000);

    for (int i = 0; i < 10; ++i) {
        blinkFor(20);
    }
}

void loop() {
    if (!superCapacitorIsChargedEnough()) {
        powerDown();
        blinkFor(1000);
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
        blinkFor(100);
        sendData(whReading, apparentPower);
    } else {
        blinkFor(20);
        blinkFor(20);
        blinkFor(20);
    }
    parser.reset();

    // End of complete frame capture
    blinkFor(100);
}

bool superCapacitorIsChargedEnough() {
    int read = analogRead(ReadPin);
    // Input of a voltage divider bridge on a 3.3V board
    float voltage = read * 2 * 3.3f / 1023;
    return voltage >= 3.7;
}

void sendData(unsigned long whReading, unsigned long apparentPower) {
    // TODO Come up with a format for the radio gateway and really send the data
    uint8_t data[] = "Hi!";
    if (radio.sendtoWait(data, sizeof(data), RadioGatewayNodeId)) {
        blinkFor(500);
    } else {
        for (int i = 0; i < 3; ++i) {
            blinkFor(100);
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
