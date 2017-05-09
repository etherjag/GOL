#include <iostream>
#include "quad_tree_tests.h"

static int64_t MinPowerOf2(int powerOf2);
static int64_t MaxPowerOf2(int powerOf2);

int main() {

    // The following tests use RLE files to test our signed 64 bit boundaries
    // We can load in RLE patterns and place them wherever we like because we clamp the patterns
    // to be within the signed 64 bit range
    // The format is:
    // @param file_name
    // @param num_generations
    // @param origin_x x coordinate (signed 64 bit int, upper left)
    // @param origin_y y coordinate (signed 64 bit int, upper left)

    // Weekender set to the INT64 MIN boundary on Y
    QuadTreeTests::RunRLEPatternTest("../patterns/weekender.rle", 1000, 0, INT64_MIN);

    // Siesta
    QuadTreeTests::RunRLEPatternTest("../patterns/siesta.rle", 1000, INT64_MIN, INT64_MIN);

    // A few block oscillators
    QuadTreeTests::RunRLEPatternTest("../patterns/mickeymouse.rle", 1, INT64_MAX, INT64_MAX);
    return 0;
}

/**
 * Calculate the minimum signed power of 2 to help us test boundaries
 * Value is -2^(n-1)
 * @param power_of_2
 * @return
 */
static int64_t MinPowerOf2(int power_of_2) {
    return -(INT64_C(1) << (power_of_2 - 1));
}

/**
 * Calculate the maximum signed power of 2 to help us test boundaries
 * Value is 2^(n-1)-1
 * @param power_of_2
 * @return
 */
static int64_t MaxPowerOf2(int power_of_2) {
    return (INT64_C(1) << (power_of_2 - 1)) - 1;
}