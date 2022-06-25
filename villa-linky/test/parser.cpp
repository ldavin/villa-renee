#include <unity.h>
#include <Parser.h>

const char line2[] = {0x0A, 'O', 'P', 'T', 'A', 'R', 'I', 'F', ' ', 'B', 'A', 'S', 'E', ' ', '0', 0x0D};
const char line3[] = {0x0A, 'I', 'S', 'O', 'U', 'S', 'C', ' ', '3', '0', ' ', '9', 0x0D};
const char line4[] = {0x0A, 'B', 'A', 'S', 'E', ' ', '0', '0', '9', '5', '6', '8', '3', '5', '5', ' ', '4', 0x0D};
const char line7[] = {0x0A, 'I', 'M', 'A', 'X', ' ', '0', '9', '0', ' ', 'H', 0x0D};
const char line8[] = {0x0A, 'P', 'A', 'P', 'P', ' ', '0', '0', '3', '4', '0', ' ', '(', 0x0D};

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

void new_parser_has_no_data_available() {
    Parser parser;
    TEST_ASSERT_FALSE(parser.dataIsAvailable());
    TEST_ASSERT_EQUAL(0, parser.getApparentPower());
    TEST_ASSERT_EQUAL(0, parser.getWhReading());
}

void parser_ignores_uninteresting_lines() {
    Parser parser;
    for (char c: line2) {
        parser.feed(c);
    }
    for (char c: line3) {
        parser.feed(c);
    }
    for (char c: line7) {
        parser.feed(c);
    }

    TEST_ASSERT_FALSE(parser.dataIsAvailable());
    TEST_ASSERT_EQUAL(0, parser.getApparentPower());
    TEST_ASSERT_EQUAL(0, parser.getWhReading());
}

void parser_saves_wh_reading() {
    Parser parser;
    for (char c: line4) {
        parser.feed(c);
    }

    TEST_ASSERT_EQUAL(9568355, parser.getWhReading());
}

void parser_saves_apparent_power() {
    Parser parser;
    for (char c: line8) {
        parser.feed(c);
    }

    TEST_ASSERT_EQUAL(340, parser.getApparentPower());
}

void parser_saves_both_and_marks_as_available_on_a_full_frame() {
    Parser parser;
    for (char c: line2) {
        parser.feed(c);
    }
    for (char c: line3) {
        parser.feed(c);
    }
    for (char c: line4) {
        parser.feed(c);
    }
    for (char c: line7) {
        parser.feed(c);
    }
    for (char c: line8) {
        parser.feed(c);
    }

    TEST_ASSERT_TRUE(parser.dataIsAvailable());
    TEST_ASSERT_EQUAL(9568355, parser.getWhReading());
    TEST_ASSERT_EQUAL(340, parser.getApparentPower());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(new_parser_has_no_data_available);
    RUN_TEST(parser_ignores_uninteresting_lines);
    RUN_TEST(parser_saves_wh_reading);
    RUN_TEST(parser_saves_apparent_power);
    RUN_TEST(parser_saves_both_and_marks_as_available_on_a_full_frame);
    UNITY_END();
}