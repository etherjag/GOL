//
// Created by Jenny Spurlock on 5/8/17.
//

#ifndef GOL_QUADTREENODE_H
#define GOL_QUADTREENODE_H

#include <cstdio>
#include <cstdint>
#include <iostream>
#include <unordered_map>
#include "quad_tree_config.h"

#if (ENABLE_BIG_INT)
#include <gmpxx.h>
#endif

/**
 * Quad Tree Node class
 * This class is based on the HashLife algorithm, invented by Bill Gosper, only it doesn't do a full hashlife calculation a power of 2 forward.
 * Instead, it calculates the next generation one step at a time
 *
 * It isn't meant to be instantiated with constructors, so those are private. Instead you should use the following methods:
 * Initialize();
 * Shutdown();
 * CreateEmptyTree();
 * SetCell();
 *
 * HashLife References:
 * https://en.wikipedia.org/wiki/Hashlife
 * http://www.drdobbs.com/jvm/an-algorithm-for-compressing-space-and-t/184406478
 * http://golly.sourceforge.net/
 * http://conwaylife.com/
 */
#if (ENABLE_INFINITE_LEVELS)
typedef  mpz_class level_type;
#else
typedef  int level_type;
#endif

class QuadTreeNode {

    friend class QuadTree;

    public:

        /**
         * Static initialize because we have some work to do, like initialize a multi precision
         * power of 2 table
         */
        static void Initialize();

        /**
         * Static shutdown because we have some work to do, like freeing all of our nodes that were
         * created because they are canonical
         */
        static void Shutdown();


        /**
         * Build an empty quadtree recursively at the specified level
         * @param level this level represents the power of 2 dimensions of this quad tree, which is square
         * @return a new empty quad tree at the specified level
         */
        static QuadTreeNode* EmptyQuadTree(level_type level);

        /**
        * Copy Constructor
        * @param other the other node to be copied from
        */
        QuadTreeNode (const QuadTreeNode& other);

        /**
         * Operator ==
         * @param other the other node to be compared
         * @return if two nodes are equal
         */
        bool operator==(const QuadTreeNode &other) const;

        /**
         * Expand this node by one level (a power of 2) so that we can effectively process inner squares
         * and not hit boundaries
         * @return a new node one level higher
         */
        QuadTreeNode* Expand();

        /**
         * Can we compact this node any smaller? We call this after evolving the root
         * Currently, we can't call this on sub-nodes that are evolved because we always assume
         * pointers to regions are valid
         * @return a new root (possibly) compacted one level smaller
         */
        QuadTreeNode* Compact();

        /**
         * The fun part. Evolve this node and all of its subchildren one generation forward based on Conway's Game of Life Rule
         * Reference: https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life
         * @return a new node pointing to a QuadTree structure evolved one generation forwared
         */
        QuadTreeNode* Evolve();

        /**
         * Turn a cell alive
         * This is ONLY called before we run any simulation and is bounded by signed 64 bit integers
         * input is bounded by signed 64 bit integer input [-2^63, 2^63-1]
         * @param x x coordinate for this simulation (signed 64 bit integer input [-2^63, 2^63-1])
         * @param y y coordinate for this simulation (signed 64 bit integer input [-2^63, 2^63-1])
         * @return a canonical, alive node
         */
        QuadTreeNode* SetCellAlive(int64_t x, int64_t y);

    #if (ENABLE_BIG_INT)
        /**
         * Build a display list of coordinates sorted by x and y
         * This will returns a list or cells that are currently alive and their current coordinates
         * @param origin_x multi precision starting coordinate of this quadtree
         * @param origin_y multi precision starting coordinate of this quadtree
         * @list vector of coordinates to return
         * todo: optimize
         */
        void BuildDisplayList(mpz_class origin_x, mpz_class origin_y, std::vector<std::pair<mpz_class, mpz_class>> &list);
    #else
        /**
         * Build a display list of coordinates sorted by x and y
         * This will returns a list or cells that are currently alive and their current coordinates
         * @param origin_x signed 64 bit starting coordinate of this quadtree
         * @param origin_y signed 64 bit starting coordinate of this quadtree
         * @list vector of coordinates to return
         * todo: optimize
         */
        void BuildDisplayList(int64_t origin_x, int64_t origin_y, std::vector<std::pair<int64_t, int64_t>> &list);
    #endif


    private:
        /**
         * Hash function to make nodes canonical
         * For a level 0 node, the hash will just be the population (1 or 0)
         * For higher level nodes, we just hash the pointers with some primes
         */
        struct HashFunction {
            public:
                size_t operator() (QuadTreeNode* node) const {
                    if (node->level == 0) {
                        return node->population > 0 ? 1 : 0;
                    } else {
                        size_t hashCalc =
                                (size_t) node->nw +
                                3 * (size_t) node->ne +
                                3 * (size_t) node->sw +
                                3 * (size_t) node->se;
                        return hashCalc;
                    }
                }
        };

        /**
         * Function to test if two node pointers are equal
         * We early out if they are not the same level, and again for level 0 we just
         * compare if both nodes are alive or not. For other level nodes, we compare the pointers
         */
        struct EqualFunction {
            public:
                bool operator()(QuadTreeNode* node1, QuadTreeNode* node2) const{
                    if (node1->level != node2->level) {
                        return false;
                    }
                    if (node1->level == 0) {
                        return (node1->alive == node2->alive);
                    }
                    return (node1->nw == node2->nw)
                           && (node1->ne == node2->ne)
                           && (node1->sw == node2->sw)
                           && (node1->se == node2->se);
                }
        };

    private:

        /**
         * This function looks up a node with level > 0 in the hash table and returns a canonical one
         * if it exists. If it doesn't exist, it creates a new node and adds it
         * @param node quad tree node
         * @return a new, canonical node
         */
        static QuadTreeNode* Canonical(QuadTreeNode* nw, QuadTreeNode* ne, QuadTreeNode* sw, QuadTreeNode* se, level_type level);

        /**
         * This function looks up a leaf node in the hash table and returns a canonical one
         * if it exists. If it doesn't exist, it creates a new node and adds it
         * @param node quad tree node
         * @return a new, canonical node
         */
        static QuadTreeNode* Canonical(int alive);

        /**
         * This function looks up a node in the hash table and returns a canonical one
         * if it exists. If it doesn't exist, it creates a new node and adds it
         * @param node quad tree node
         * @return a new, canonical node
         */
        static QuadTreeNode* Canonical(QuadTreeNode* node);

        /**
         * Private non-leaf node constructor
         * @param nw northwest corner node
         * @param ne southeast corner node
         * @param sw southwest corner node
         * @param se southeast corner node
         * @param level current level this node represents (2^level is the side dimension of this square, which is NxN)
         */
        QuadTreeNode(QuadTreeNode* nw, QuadTreeNode* ne, QuadTreeNode* sw, QuadTreeNode* se, level_type level);

        /**
         * Private Leaf node constructor (1x1 square, level 0, which is 2^0 x 2^0 in size)
         * @param alive is this cell alive?
         */
        QuadTreeNode(int alive);

        /**
         * Function to evolve a level 2 square, which is 2^2 by 2^2 in size
         * Level 2 squares calculate and apply the game of life rule to their four inner squares
         * to calculate the next generation. We don't do borders because those are taking care of in the recursive
         * level above, which calculates  overlapping inner squares
         *   +--+--+--+--+
         *   |__|__|__|__|
         *   |  |  |  |  |
         *   +--|--|--|--+
         *   |__|__|__|__|
         *   |  |  |  |  |
         *   +--+--+--+--+
         * @return a new calculated result for a level 2 square
         */
        QuadTreeNode* EvolveLevel2();

        /**
         * Function to evolve a level N square, which is 2^N by 2^N in size and N is > 2
         * Level 2 squares evolve their inner squares, and a level above we combine the results
         * and keep recursing down. This algorithm works because the tree has an empty border to ensure
         * That calculations will be correct at edges
         *   +--+--+--+--+--+--+--+--+--+
         *   |                          |
         *   |   +--+--+--+--+--+--+    |
         *   |   | NW  |  N  | NE  |    |
         *   |   |     |     |     |    |
         *   |   +--+--+--+--+--+--+    |
         *   |   | W   |  C  | E   |    |
         *   |   |     |     |     |    |
         *   |   +--+--+--+--+--+--+    |
         *   |   | SW  |  S  | SE  |    |
         *   |   |     |     |     |    |
         *   |   +--+--+--+--+--+--+    |
         *   |                          |
         *   +--+--+--+--+--+--+--+--+--+
         * @return a new calculated result for a level N square, one level down
         */
        QuadTreeNode* EvolveLevelN();


        /**
         * Run the game of life rule give an alive status and a neighbor count
         * @param alive current alive status
         * @param count current neighbor count
         * @return if this node is alive in the next generation
         */
        int RunRule(int alive, int count);

        /**
         * Helper functions to get inner nodes
         *   +--+--+--+--+--+--+--+--+--+
         *   |                          |
         *   |   +--+--+--+--+--+--+    |
         *   |   | NW  |  N  | NE  |    |
         *   |   |     |     |     |    |
         *   |   +--+--+--+--+--+--+    |
         *   |   | W   |  C  | E   |    |
         *   |   |     |     |     |    |
         *   |   +--+--+--+--+--+--+    |
         *   |   | SW  |  S  | SE  |    |
         *   |   |     |     |     |    |
         *   |   +--+--+--+--+--+--+    |
         *   |                          |
         *   +--+--+--+--+--+--+--+--+--+
         * @return
         */
        QuadTreeNode* GetInnerNWNode();
        QuadTreeNode* GetInnerNNode();
        QuadTreeNode* GetInnerNENode();
        QuadTreeNode* GetInnerWNode();
        QuadTreeNode* GetInnerCNode();
        QuadTreeNode* GetInnerENode();
        QuadTreeNode* GetInnerSWNode();
        QuadTreeNode* GetInnerSNode();
        QuadTreeNode* GetInnerSENode();

#if (ENABLE_BIG_INT)
        /**
         * This function calculates powers of two up to LEVEL_MAX for multi-precision integers
         * Because this table has a defined limit, we really can't ever reach infinity if we choose to use it
         */
        static void InitializePow2Table();
#endif

    private:

        // Northwest node
        QuadTreeNode* nw;

        // Northeast node
        QuadTreeNode* ne;

        // Southwest node
        QuadTreeNode* sw;

        // Southeast node
        QuadTreeNode* se;

        // Memoization: store the result of this node being evolved one generation forward
        QuadTreeNode* calc;

        // Indicates if this node is alive. We store this here because it's faster to check this
        // than to compare a multi precision value if bigint is enabled
        int8_t alive;

        // Level of this node, meaning this node is 2^level x 2^level in size, with coordinates
        // ranging from [-2^(n-1), 2^(n-1)-1]
        level_type level;

    #if (ENABLE_BIG_INT)
        // population count of this node. We use a multiprecision integer because we can have a multiprecision boundary size
        // todo: this is huge overkill for every node.
        mpz_t population;
    #else
        // population count of this node. This really won't work if big int is off and the number
        // of nodes exceeds a signed 64 bit int, which it can because our coordinates are between [-2^63, 2^63-1]
        // and every row could be full
        int64_t population;
    #endif

        // canonical map of all of our nodes
        static std::unordered_map<QuadTreeNode*, QuadTreeNode*, HashFunction, EqualFunction> node_map;

        // helper variables to let us know how many nodes have been created total
        static int64_t num_nodes_created;

    #if (ENABLE_BIG_INT)
        // precalculated multi precision powers of two
        static mpz_class mpz_pow2_table[LEVEL_MAX];
    #endif
};


#endif //GOL_QUADTREENODE_H
