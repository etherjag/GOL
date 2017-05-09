//
// Created by Jenny Spurlock on 5/8/17.
//
#include <iostream>
#ifndef GOL_QUAD_TREE_TESTS_H
#define GOL_QUAD_TREE_TESTS_H

/*
 * There are a series of pattern and stress tests here that can be run with QuadTreeTests::RunRLEPatternTest.
 * We test patterns to make sure that:
 * 1) The algorithm is working.
 * 2) Patterns can grow beyond the initial 64 bit integer input restriction, which we test by placing at the corners
 *
 * Stress tests are run with the QuadTreeTests::RunMegaRandomMaxBoundariesTest. This lets us perform the worst case
 * scenario for this algorithm, which is random, dense data.
 * 1) User can provide the number of random cells to be initialized
 * 2) User can provide the x min and max boundary, and x min and max boundary, to generate random cells in
 */
class QuadTreeTests {

    public:
        /**
         * This test function doesn't work well as random cells often just die in the next generation and the field is too large
         * @param num_nodes
         * @param num_generations
         */
        static void RunMegaRandomMaxBoundariesTest(int64_t num_nodes, int64_t num_generations, int64_t min_x, int64_t max_x, int64_t min_y, int64_t max_y, bool draw_result = true);

        /**
         * Read in an RLE pattern and perform the simulation for the specified number of generations starting at a specific origin
         * @param pattern_file_name
         * @param num_generations
         * @param origin_x
         * @param origin_y
         */
        static void RunRLEPatternTest(const char* pattern_file_name, int num_generations, int64_t origin_x = 0, int64_t origin_y = 0, bool draw_result = true);

    private:
        /**
         * Read an RLE pattern from a file and output a vector of coordinate pairs
         * Read more about the file format here:
         * http://www.conwaylife.com/w/index.php?title=Run_Length_Encoded
         *
         * @param file_name file name of the rle file
         * @return a vector of coordinate pairs
         */
        static std::vector<std::pair<int64_t, int64_t>> ReadRLEPattern(const char *file_name, int64_t origin_x = 0, int64_t origin_y = 0);
};


#endif //GOL_QUAD_TREE_TESTS_H
