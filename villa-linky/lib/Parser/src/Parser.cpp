//
// Created by Louis Davin on 19/06/2022.
//

#include <string.h>
#include <stdlib.h>
#include "Parser.h"

#define lineStart 0x0A
#define separator 0x20

Parser::Parser() {
    reset();
}

unsigned long Parser::getWhReading() const {
    return whReading;
}

unsigned long Parser::getApparentPower() const {
    return apparentPower;
}

void Parser::feed(char newChar) {
    if (waitingForLineStart && newChar != lineStart) return;

    if (newChar == lineStart) {
        waitingForLineStart = false;
        readingLabel = true;
        labelIndex = 0;
        memset(labelBuffer, 0, sizeof(labelBuffer));
        return;
    }

    if (newChar == separator) {
        if (readingLabel) {
            readingLabel = false;
            valueIndex = 0;
            memset(valueBuffer, 0, sizeof(valueBuffer));

            if (strcmp(labelBuffer, "BASE") == 0) {
                readingWhReadingValue = true;
            } else if (strcmp(labelBuffer, "PAPP") == 0) {
                readingApparentPowerValue = true;
            } else {
                waitingForLineStart = true;
            }
        } else if (readingWhReadingValue || readingApparentPowerValue) {
            if (readingWhReadingValue) {
                readingWhReadingValue = false;
                whReading = atol(valueBuffer);
                whReadingWasParsed = true;
            }
            if (readingApparentPowerValue) {
                readingApparentPowerValue = false;
                apparentPower = atol(valueBuffer);
                apparentPowerWasParsed = true;
            }
            waitingForLineStart = true;
        }

        return;
    }

    if (readingLabel) {
        labelBuffer[labelIndex] = newChar;
        labelIndex++;
    } else if (readingWhReadingValue || readingApparentPowerValue) {
        valueBuffer[valueIndex] = newChar;
        valueIndex++;
    }
}

void Parser::reset() {
    whReading = 0;
    whReadingWasParsed = false;
    apparentPower = 0;
    apparentPowerWasParsed = false;
    waitingForLineStart = true;
    readingLabel = false;
    readingWhReadingValue = false;
    readingApparentPowerValue = false;
}

bool Parser::dataIsAvailable() const {
    return whReadingWasParsed && apparentPowerWasParsed;
}
