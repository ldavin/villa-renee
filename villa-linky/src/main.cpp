#include <Arduino.h>

#define AcquisitionSerial board_specific_serial
#define startFrame 0x02
#define endFrame 0x03

void blinkFor(int durationMillis);

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    AcquisitionSerial.begin(1200);
    delay(1000);

    for (int i = 0; i < 3; ++i) {
        blinkFor(100);
        delay(100);
    }
}

void loop() {
    char charIn = 0;

    while (charIn != startFrame) {
        // We skip the 8th bit per spec
        charIn = AcquisitionSerial.read() & 0x7F;
    }

    // Start of frame capture
    while (charIn != endFrame) {
        if (AcquisitionSerial.available()) {
            charIn = AcquisitionSerial.read() & 0x7F;

            // TODO Do something with the char
        }
    }

    // End of complete frame capture
    blinkFor(200);
}

void blinkFor(int durationMillis) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(durationMillis);
    digitalWrite(LED_BUILTIN, LOW);
}
