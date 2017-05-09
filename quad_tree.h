//
// Created by Jenny Spurlock on 5/4/17.
//

#ifndef GOL_QUADTREE_H
#define GOL_QUADTREE_H


#include <cstdio>
#include <cstdint>
#include <iostream>
#include <unordered_map>
#include "quad_tree_node.h"
#include "quad_tree_config.h"

#if (ENABLE_BIG_INT)
#include <gmpxx.h>
#endif

/**
 * This class is used to instantiate a data structure used to store a recursive, quad tree structure for cellular automata.
 * In this instance, it is used to calculate and evolve nodes in Conway's Game of Life one generation at a time.
 * It depends on memorized, canonical nodes because it also uses a big int to allow large board setup.
 *
 * For a better understanding on how hash life works;
 * https://en.wikipedia.org/wiki/Hashlife
 * http://www.drdobbs.com/jvm/an-algorithm-for-compressing-space-and-t/184406478
 * http://golly.sourceforge.net/
 * http://conwaylife.com/
 */
class QuadTree {

    public:

        /**
         * Constructs a new quad tree with a base level
         */
        QuadTree();

        /**
         * Destructor
         */
        ~QuadTree();

        /**
         * Initialize this quad tree before it's actually been run
         * @param input in the form of {x, y}
         * @param num_rows the number of input pairs
        */
        void SetCellsAlive(std::vector<std::pair<int64_t, int64_t>> input);

        /**
         * Initialize this quad tree before it's actually been run
         * @param input in the form of {x, y}
         * @param num_rows the number of input pairs
         */
        void SetCellsAlive(int64_t input[][2], size_t num_rows);

        /**
         * Step this quad tree one generation forward using Conway's Game of Life Rules
         * and a hashed tree node algorithm
         *
         * HashLife References:
         * https://en.wikipedia.org/wiki/Hashlife
         */
        void Step();

        /**
         * Print some debug information, and if the board is small enough we print
         * it out to the console with empty cells as "_", and alive cells as "*"
         */
        void Print();

    private:

        /**
         * Simple hash function to allow a pair of pointers to be in an unordered_map
         */
        class PairHash {
            public:
                template <class T1, class T2>
                std::size_t operator () (const std::pair<T1,T2> &p) const {
                    auto h1 = std::hash<T1>{}(p.first);
                    auto h2 = std::hash<T2>{}(p.second);
                    // just multiply the second function by a prime number
                    return h1 + 11 * h2;
                }
        };

#if (ENABLE_BIG_INT)
        /**
         * Simple hash function to allow a pair of multi precision objects to be in an unordered_map
         * We don't use any multiplication because of speed
         */
        class MpzClassHash {
            public:
                template <class T1, class T2>
                std::size_t operator () (const std::pair<T1,T2> &p) const {
                    // just add them together to get a hash. they could be beyond si limits anyway and we
                    // don't want this to be very slow.
                    size_t hash = p.first.get_si() + p.second.get_si();
                    return hash;
                }
        };
#endif

    private:

        /**
         * Turn a cell alive
         * This is ONLY called before we run any simulation and is bounded by signed 64 bit integers
         * input is bounded by signed 64 bit integer input [-2^63, 2^63-1]
         * @param x x coordinate for this simulation (signed 64 bit integer input [-2^63, 2^63-1])
         * @param y y coordinate for this simulation (signed 64 bit integer input [-2^63, 2^63-1])
         * @return
         */
        void SetCellAlive(int64_t x, int64_t y);

        /**
         * Print out the current hashtable, mainly for debugging
         */
        void PrintHashTable();

#if (ENABLE_GARBAGE_COLLECTION)
        /**
         * Collect garbage, depending on whatever scheme is active
         * @return if garbage collection was performed
         */
        bool CollectGarbage();

        /**
         * Helper for garbage collection, called recursively to iterate
         * Through a quad tree node structure
         * @param nodesInUse
         * @param curr
         */
        void CollectGarbageHelper(std::unordered_map<QuadTreeNode *, QuadTreeNode *> &nodesInUse, QuadTreeNode *curr);
#endif

        /**
         * Print run stats, including memory usage and memory
         */
        void PrintStats();

        /**
         * Print a list of display coordinates, or if the alive cells are tightly clustered, a console printout of the board
         */
        void PrintDisplayCoordinates();

    private:
        // The root note of our quad tree
        QuadTreeNode* root;

        // Origin display coordinates of this quad tree
#if (ENABLE_BIG_INT)
        mpz_class origin_x;
        mpz_class origin_y;
#else
        int64_t origin_x;
        int64_t origin_y;
#endif

        // Number of generations
        int64_t num_generations;

        // define the number of levels to construct the quad tree with
        // this should never be under 3
        const int kStartLevels = 3;
};

#endif //GOL_QUADTREE_H
