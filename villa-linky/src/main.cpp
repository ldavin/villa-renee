#include <Arduino.h>
#include <RFM69_ATC.h>
#include <LowPower.h>
#include <SPIFlash.h>
#include <Parser.h>

#define AcquisitionSerial board_specific_serial
#define startFrame 0x02
#define endFrame 0x03

#define RadioFrequency RF69_868MHZ
#define RadioGatewayNodeId 1
#define RadioNodeId 2
#define RadioNetworkId 100
#define RadioKey "sampleEncryptKey"
#define RadioAtcRssi -75

Parser parser;
RFM69_ATC radio;
SPIFlash flash(SS_FLASHMEM, 0xEF30);

void blinkFor(int durationMillis);

void sendData(unsigned long whReading, unsigned long apparentPower);

bool superCapacitorIsChargedEnough();

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    AcquisitionSerial.begin(1200);
    pinMode(A7, INPUT);

    radio.initialize(RadioFrequency, RadioNodeId, RadioNetworkId);
    radio.setHighPower();
    radio.encrypt(RadioKey);
    radio.enableAutoPower(RadioAtcRssi);
    radio.sleep();

    if (flash.initialize()) {
        flash.sleep();
    }

    delay(1000);

    for (int i = 0; i < 3; ++i) {
        blinkFor(100);
    }
}

void loop() {
    if (!superCapacitorIsChargedEnough()) {
        LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
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
        parser.reset();
        blinkFor(20);
        blinkFor(20);
        blinkFor(20);
    }

    // End of complete frame capture
    blinkFor(100);
}

bool superCapacitorIsChargedEnough() {
    int read = analogRead(A7);
    // Input of a voltage divider bridge on a 3.3V board
    float voltage = read * 2 * 3.3f / 1023;
    return voltage >= 3.7;
}

void sendData(unsigned long whReading, unsigned long apparentPower) {
    // TODO Come up with a format for the radio gateway and really send the data
    radio.sendWithRetry(RadioGatewayNodeId, "Hi", 2);
}

void blinkFor(int durationMillis) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(durationMillis);
    digitalWrite(LED_BUILTIN, LOW);
    delay(durationMillis);
}
