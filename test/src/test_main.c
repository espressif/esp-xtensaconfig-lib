#include "unity.h"

extern void test_dynconfig(void);

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_dynconfig);
    return UNITY_END();
}
