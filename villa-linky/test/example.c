#include <unity.h>

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

void sample_test() {
    TEST_ASSERT_TRUE(1);
}

int main( int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(sample_test);
    UNITY_END();
}