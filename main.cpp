#include <iostream>
#include "quad_tree_tests.h"

/**
 * This is the main entry point to run and test the cellular automata quad tree that processes input
 * and runs Conway's Game of Life rules on the cells.
 *
 * There are a series of pattern and stress tests here that can be run with QuadTreeTests::RunRLEPatternTest.
 * We test patterns to make sure that:
 * 1) The algorithm is working.
 * 2) Patterns can grow beyond the initial 64 bit integer input restriction
 *
 * Stress tests are run with the QuadTreeTests::RunMegaRandomMaxBoundariesTest. This lets us perform the worst case
 * scenario for this algorithm, which is random, dense data.
 * 1) User can provide the number of random cells to be initialized
 * 2) User can provide the x min and max boundary, and x min and max boundary, to generate random cells in
 */

/**
 * Helper functions to calculate signed integer boundary points for our quad tree
 * @param powerOf2
 * @return calculated values
 */
static int64_t MinPowerOf2(int powerOf2);
static int64_t MaxPowerOf2(int powerOf2);


int main(int argc, char* argv[]) {
    // The following tests use RLE files to test our signed 64 bit boundaries
    // We can load in RLE patterns and place them wherever we like because we clamp the patterns
    // to be within the signed 64 bit range
    // The format is:
    // @param file_name
    // @param num_generations
    // @param origin_x x coordinate (signed 64 bit int, upper left)
    // @param origin_y y coordinate (signed 64 bit int, upper left)
    // @param draw_result specify to render the result to the console or print a list of display coordinates

    // Todo: Remember to change the "../patterns" directory if running debug from a non one level sub-directory

    // Glider: lets put it at our signed 64 bit boundary and see what happens!
    QuadTreeTests::RunRLEPatternTest("../patterns/glider.rle", 100000, INT64_MAX, INT64_MAX, true);
    return 0;

    // Weekender set to the INT64 MIN boundary on Y (it moves upwards)
    QuadTreeTests::RunRLEPatternTest("../patterns/weekender.rle", 1000, 0, INT64_MIN, true);

    // Weekender set to the INT64 MIN boundary on Y (it moves upwards)
    QuadTreeTests::RunRLEPatternTest("../patterns/puffer1.rle", 1000, INT64_MAX, INT64_MAX, true);

    // Siesta - 2 cycle oscillator
    QuadTreeTests::RunRLEPatternTest("../patterns/siesta.rle", 1000, INT64_MIN, INT64_MIN, true);

    // Mickey Mouse - Test a still life at a quad tree boundary
    QuadTreeTests::RunRLEPatternTest("../patterns/mickeymouse.rle", 1, MinPowerOf2(16), MaxPowerOf2(16), true);

    // Loaf - still life
    QuadTreeTests::RunRLEPatternTest("../patterns/loaf.rle", 100, INT64_MAX, INT64_MAX, true);

    // Queen Bee
    QuadTreeTests::RunRLEPatternTest("../patterns/queenbee.rle", 21, INT64_MIN, INT64_MIN, true);

    // Queen Bee Stable - This pattern becomes stable at 191
    QuadTreeTests::RunRLEPatternTest("../patterns/queenbee.rle", 191, INT64_MIN, INT64_MIN, true);

    // Edna, A methuselah with lifespan 31,192.
    // This algorithm doesn't handle giant methuselah's particularly well
    // (The first 500 generations of Edna get processed in under a second though)
    QuadTreeTests::RunRLEPatternTest("../patterns/edna.rle", 500, INT64_MIN, INT64_MIN, false);

    // STRESS TEST: Generate cells in a signed 64 bit range with boundaries and generations clamped
    QuadTreeTests::RunMegaRandomMaxBoundariesTest(1000, 100, MinPowerOf2(6), MaxPowerOf2(6), MinPowerOf2(6), MaxPowerOf2(6), false);

    // STRESS TEST: 200-300k cells initialized
    QuadTreeTests::RunMegaRandomMaxBoundariesTest(MaxPowerOf2(12) * MaxPowerOf2(10), 0, MinPowerOf2(9), MaxPowerOf2(9), MinPowerOf2(9), MaxPowerOf2(9), false);

    // STRESS TEST: Generate millions of random cells. This is the worst case scenario
    // This tries to set about 4 million cells and evolves to 4 million nodes, and can use up to 2GB of memory (!!!)
    // This is commented out because it's slow
    //QuadTreeTests::RunMegaRandomMaxBoundariesTest(MaxPowerOf2(12) * MaxPowerOf2(12), 1, MinPowerOf2(12), MaxPowerOf2(12), MinPowerOf2(12), MaxPowerOf2(12), false);

    return 0;
}

/**
 * Calculate the minimum signed power of 2 to help us test quad tree boundaries
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