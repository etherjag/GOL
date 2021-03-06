//
// Created by Jenny Spurlock on 5/8/17.
//
#include <vector>
#include <fstream>
#include <random>
#include "quad_tree_tests.h"
#include "quad_tree.h"

/**
 * Read in an RLE pattern and perform the simulation for the specified number of generations starting at a specific origin
 * @param pattern_file_name
 * @param num_generations
 * @param origin_x
 * @param origin_y
 */
void QuadTreeTests::RunRLEPatternTest(const char* pattern_file_name, int num_generations, int64_t origin_x, int64_t origin_y, bool draw_result) {
    std::cout << "======================================================================================\n";
    std::cout << "Running Pattern Test: " << pattern_file_name << " " << "Generations: " << num_generations << " Location: (" << origin_x << ", " << origin_y << ")" << std::endl;
    std::cout << "======================================================================================\n";

    std::cout << "Reading in RLE pattern ..\n";
    std::vector<std::pair<int64_t, int64_t>> pattern_coords = ReadRLEPattern(pattern_file_name, origin_x, origin_y);
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    if (pattern_coords.size() > 0) {
        // Initialize the quad tree
        QuadTree quad_tree;
        std::cout << "Initializing quad tree with cells..\n";
        quad_tree.SetCellsAlive(pattern_coords);

        // Run the test
        std::cout << "Evolving for " << num_generations << " generations..\n";
        for (int x = 0; x < num_generations; ++x) {
            quad_tree.Step();
        }

        // print statistics
        quad_tree.PrintStats();

        // Ended, print out some stuff!
        if (draw_result) {
            std::cout << "Building display list...\n";
            std::chrono::high_resolution_clock::time_point display1 = std::chrono::high_resolution_clock::now();
            quad_tree.PrintDisplayCoordinates();
            std::chrono::high_resolution_clock::time_point display2 = std::chrono::high_resolution_clock::now();
            auto display_duration = std::chrono::duration_cast<std::chrono::milliseconds>(display2 - display1).count();
            std::cout << "Processed and printed display list in " << display_duration << " milliseconds" << std::endl;
        }
    } else {
        std::cout << "Unable to load pattern: " << pattern_file_name << std::endl;
    }
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();
    std::cout << "DONE: Processed test in " << duration << " milliseconds" << std::endl << std::endl;
}

/**
 * Read an RLE pattern from a file and output a vector of coordinate pairs
 * Read more about the file format here:
 * http://www.conwaylife.com/w/index.php?title=Run_Length_Encoded
 *
 * @param file_name file name of the rle file
 * @return a vector of coordinate pairs
 */
std::vector<std::pair<int64_t, int64_t>> QuadTreeTests::ReadRLEPattern(const char *file_name, int64_t origin_x, int64_t origin_y) {

    std::vector<std::pair<int64_t, int64_t>> pattern_coords;

    std::ifstream input(file_name);
    int64_t x = origin_x;
    int64_t y = origin_y;
    int param_arg = 0;
    for (std::string line; std::getline(input, line); ) {
        // Ignore these, these are comment lines
        if (line.at(0) == '#') {
            continue;
        } else if(line.at(0) == 'x') {
            std::string s = line;
            std::string delimiter = ", ";
            std::string coord_delimiter = " = ";

            size_t pos = 0;
            std::string token;
            std::string x_str = s.substr(0, s.find(delimiter));
            std::string y_str = s.substr(s.find(delimiter)+delimiter.size());
            y_str = y_str.substr(0, s.find(delimiter));

            int64_t x_bound = atol(x_str.substr(x_str.find(coord_delimiter) + coord_delimiter.size()).c_str());
            int64_t y_bound = atol(y_str.substr(y_str.find(coord_delimiter) + coord_delimiter.size()).c_str());

            // clamp boundaries to 64 bit signed integer max
            if ((x_bound + origin_x - 1) < origin_x) {
                x = origin_x = INT64_MAX - x_bound;
            }
            if ((y_bound + origin_y - 1) < origin_y) {
                y = origin_y = INT64_MAX - y_bound;
            }
            continue;
        }
        for (int i=0; i<line.length(); i++) {
            char c = line.at(i);
            int param = (param_arg == 0 ? 1 : param_arg);
            if (c == ' ') {
                continue;
            } else if (c == 'b') {
                x += param;
                param_arg = 0;
            } else if (c == 'o') {
                while (param-- > 0) {
                    //std::cout << x << " " << y << std::endl;
                    pattern_coords.push_back(std::make_pair(x, y));
                    x++;
                }
                param_arg = 0;
            } else if (c == '$') {
                y += param;
                x = origin_x;
                param_arg = 0;
            } else if ('0' <= c && c <= '9') {
                param_arg = 10 * param_arg + c - '0';
            } else if (c == '!') {
                return pattern_coords;
            }
        }
    }
    input.close();

    return pattern_coords;
}


/**
 * This test function doesn't work well as random cells often just die in the next generation and the field is too large
 * @param num_nodes
 * @param num_generations
 */
void QuadTreeTests::RunMegaRandomMaxBoundariesTest(int64_t num_nodes, int64_t num_generations, int64_t min_x, int64_t max_x, int64_t min_y, int64_t max_y, bool draw_result) {
    std::cout << "======================================================================================\n";
    std::cout << "Running MegaRandomMaxBoundariesTest -> Random Nodes: " << num_nodes << " Generations: " << num_generations << std::endl;
    std::cout << "======================================================================================\n";

    std::cout << "Initializing with random nodes..\n";

    // Apparently this is how we do random signed 64 bit integers?
    std::vector<std::pair<int64_t, int64_t>> pattern_coords;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int64_t> gen_random_x(min_x, max_x);
    std::uniform_int_distribution<int64_t> gen_random_y(min_y, max_y);

    for (int64_t i = 0; i < num_nodes; ++i) {
        pattern_coords.push_back(std::make_pair(gen_random_x(gen), gen_random_y(gen)));
    }

    std::cout << "Initializing quad tree with generated nodes..\n";
    // Initialize the quad tree
    QuadTree quad_tree;
    quad_tree.SetCellsAlive(pattern_coords);

    // Run the test
    std::cout << "Evolving for " << num_generations << " generations..\n";
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    for (int x = 0; x < num_generations; ++x) {
        quad_tree.Step();
    }
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    std::cout << "Processed generations in " << duration << " milliseconds" << std::endl;

    // print statistics
    quad_tree.PrintStats();

    // Ended, print out some stuff!
    if (draw_result) {
        std::cout << "Building display list...\n";
        std::chrono::high_resolution_clock::time_point display1 = std::chrono::high_resolution_clock::now();
        quad_tree.PrintDisplayCoordinates();
        std::chrono::high_resolution_clock::time_point display2 = std::chrono::high_resolution_clock::now();
        auto display_duration = std::chrono::duration_cast<std::chrono::milliseconds>(display2 - display1).count();
        std::cout << "DONE: Processed and printed display list in " << display_duration << " milliseconds" << std::endl << std::endl;
    } else {
        std::cout << "DONE." << std::endl << std::endl;
    }
}