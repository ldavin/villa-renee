//
// Created by Louis Davin on 19/06/2022.
//

#ifndef VILLALINKY_PARSER_H
#define VILLALINKY_PARSER_H


class Parser {
private:
    unsigned long whReading;
    unsigned int apparentPower;
    bool whReadingWasParsed;
    bool apparentPowerWasParsed;
    bool waitingForLineStart;
    bool readingLabel;
    bool readingWhReadingValue;
    bool readingApparentPowerValue;
    char labelBuffer[32];
    char valueBuffer[32];
    int labelIndex;
    int valueIndex;

public:
    Parser();

    unsigned long getWhReading() const;

    unsigned long getApparentPower() const;

    void feed(char newChar);

    void reset();

    bool dataIsAvailable() const;
};


#endif //VILLALINKY_PARSER_H
