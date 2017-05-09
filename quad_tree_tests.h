//
// Created by Jenny Spurlock on 5/8/17.
//
#include <iostream>
#ifndef GOL_QUAD_TREE_TESTS_H
#define GOL_QUAD_TREE_TESTS_H


class QuadTreeTests {

    public:
        /**
         * Read in an RLE pattern and perform the simulation for the specified number of generations starting at a specific origin
         * @param pattern_file_name
         * @param num_generations
         * @param origin_x
         * @param origin_y
         */
        static void RunRLEPatternTest(const char* pattern_file_name, int num_generations, int64_t origin_x = 0, int64_t origin_y = 0);

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
