#include "../src/test.h"

DEFINE_TEST(simple_test) {
    assert(1 == 1);
}

int main() {
    RUN_SINGLE_TEST(simple_test);
    return 0;
}