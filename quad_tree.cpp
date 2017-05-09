//
// Created by Jenny Spurlock on 5/4/17.
//

#include <vector>
#include <fstream>
#include "quad_tree.h"

/**
 * Constructs a new quad tree with a base level
 */
QuadTree::QuadTree() {
    // Initialize the quad tree node structure
    QuadTreeNode::Initialize();
    // create a new root node at level 3
    root = QuadTreeNode::EmptyQuadTree(kStartLevels);
    // init generation count;
    num_generations = 0;
    // Set our display origin to zero for now
    origin_x = 0;
    origin_y = 0;
}

/**
 * Destructor
 */
QuadTree::~QuadTree() {
    // Shutdown the quad tree nodes, freeing all node memory
    QuadTreeNode::Shutdown();
}

/**
 * Initialize this quad tree before it's actually been run
 * @param input in the form of {x, y}
 * @param num_rows the number of input pairs
 */
void QuadTree::SetCellsAlive(std::vector<std::pair<int64_t, int64_t>> input) {
     for (std::pair<int64_t, int64_t> pair : input) {
         SetCellAlive(pair.first, pair.second);
     }
}

/**
 * Initialize this quad tree before it's actually been run
 * @param input in the form of {x, y}
 * @param num_rows the number of input pairs
 */
void QuadTree::SetCellsAlive(int64_t input[][2], size_t num_rows) {
    // set the cell values for out input
    for (size_t i = 0; i < num_rows; ++i) {
        SetCellAlive(input[i][0], input[i][1]);
    }
}

/**
 * Step this quad tree one generation forward using Conway's Game of Life Rules
 * and a hashed tree node algorithm
 *
 * HashLife References:
 * https://en.wikipedia.org/wiki/Hashlife
 */
void QuadTree::Step() {
    // does the root exist and have a population greater than zero?
#if (ENABLE_BIG_INT)
    if (root == 0 || mpz_cmp_si(root->population, 0) == 0) {
#else
        if (root == 0 || root->population == 0) {
#endif
        return;
    }
    // if we're under level 3 or our population one level below doesn't match 2 levels below itself,
    // we expand to ensure that there is a border along the edge to calculate the next generation
    while (
            root->level < 3
#if (ENABLE_BIG_INT)
            || mpz_cmp(root->nw->population, root->nw->se->se->population) != 0
            || mpz_cmp(root->ne->population, root->ne->sw->sw->population) != 0
            || mpz_cmp(root->sw->population, root->sw->ne->ne->population) != 0
            || mpz_cmp(root->se->population, root->se->nw->nw->population) != 0
#else
            || root->nw->population != root->nw->se->se->population
            || root->ne->population != root->ne->sw->sw->population
            || root->sw->population != root->sw->ne->ne->population
            || root->se->population != root->se->nw->nw->population
#endif
    ) {
        root = root->Expand();
    }
    // evolve a generation forward
    root = root->Evolve();
    // can we prune this level above and shrink our structure?
    root = root->Compact();
    // increment the current generation
    ++num_generations;
    // collect garbage
    CollectGarbage();
}

/**
 * Print some debug information, and if the board is small enough we print
 * it out to the console with empty cells as "_", and alive cells as "*"
 */
void QuadTree::PrintVerbose() {
    // print out some statistics
    PrintStats();
    // Print our display coordinates or render out a tiny display if our nodes are constrained small enough
    PrintDisplayCoordinates();
}



/**
 * Print run stats, including memory usage and memory
 */
void QuadTree::PrintStats() {
    size_t total_mem = (sizeof(QuadTreeNode) * QuadTreeNode::node_map.size())/1024;  // convert to kilobytes
    std::cout << "Generating stats..\n";
    std::cout << "\tOverview: Generation (" << num_generations << ") Population (" << root->population << ")" << " Tree Level (" << root->level << ")" << std::endl;
    std::cout << "\t\tCurrent # nodes: " << QuadTreeNode::node_map.size() << std::endl;
    std::cout << "\t\tCurrent Heap memory usage: " << total_mem << " KB" << std::endl;
    std::cout << "\t\tAll Time # nodes: " << QuadTreeNode::num_nodes_created << std::endl;
    std::cout << "\t\tNW Population: " << root->nw->population << std::endl;
    std::cout << "\t\tNE Population: " << root->ne->population << std::endl;
    std::cout << "\t\tSW Population: " << root->sw->population << std::endl;
    std::cout << "\t\tSE Population: " << root->se->population << std::endl;
}

/**
 * Print a list of display coordinates, or if the alive cells are tightly clustered, a console printout of the board
 */
void QuadTree::PrintDisplayCoordinates() {

#if (ENABLE_BIG_INT)
    std::cout << "Generating Display List..\n";
    // Create our display list, starting from our origin coordinates
    std::vector<std::pair<mpz_class, mpz_class>> display_list;
    root->BuildDisplayList(origin_x, origin_y, display_list);

    // Setup our display coordinate map to store alive cells
    std::unordered_map<std::pair<mpz_class, mpz_class>, bool, MpzClassHash> display_map;
    bool minMaxSet = false;
    mpz_class min_x = 0;
    mpz_class min_y = 0;
    mpz_class max_x = 0;
    mpz_class max_y = 0;

    // Let's go through our display list, record min and max coordinates, and add to our map for fast access
    for(std::pair<mpz_class, mpz_class> pair : display_list) {
        if (!minMaxSet) {
            min_x = pair.first;
            max_x = pair.first;
            min_y = pair.second;
            max_y = pair.second;
            minMaxSet = true;
        } else {
            if (pair.first < min_x) {
                min_x = pair.first;
            } else if (pair.first > max_x) {
                max_x = pair.first;
            }
            if (pair.second < min_y) {
                min_y = pair.second;
            } else if (pair.second > max_y) {
                max_y = pair.second;
            }
        }
        display_map[pair] = true;
    }
    // Print out a small render of the board, or if its too large, print out display coordinates so we can verify
    std::cout << "Drawing Boundaries min(" << min_x << ", " << min_y  << ") max(" << max_x << ", " << max_y << ").." << std::endl;
    int display_list_index = 0;
    // Are we small enough to render out to the console?
    if ((max_x - min_x < DEBUG_RENDER_SIZE_MAX) && (max_y - min_y < DEBUG_RENDER_SIZE_MAX)) {
        for (mpz_class y = min_y; y <= max_y; ++y) {
            mpz_class x;
            for (x = min_x; x <= max_x; ++x) {
                if (display_map[std::make_pair(x, y)]) {
                    std::cout << "*";
                    ++display_list_index;
                } else {

                    std::cout << "_";
                }
            }
            std::cout << std::endl;
        }
    } else {
        /*std::ofstream myfile("test.txt");

        if (myfile.is_open())
        {
            for (mpz_class y = min_y; y <= max_y; ++y) {
                mpz_class x;
                for (x = min_x; x <= max_x; ++x) {
                    if (display_map[std::make_pair(x, y)]) {
                        myfile << "*";
                        ++display_list_index;
                    } else {

                        myfile << "_";
                    }
                }
                myfile << std::endl;
            }
            myfile.close();
        }*/

        // Render size would be too large, so lets just display a list of coordinates
        // Let's go through our display list, record min and max coordinates, and add to our map for fast access
        int64_t max = 0;
        for(std::pair<mpz_class, mpz_class> pair : display_list) {
            if (max > DEBUG_PRINT_NODES_MAX) {
                std::cout << "\n ... and " << display_list.size() - DEBUG_PRINT_NODES_MAX << " more cells.\n";
                break;
            }
            std::cout << "(" << pair.first << ", " << pair.second << ") ";
            ++max;
        }
        std::cout << std::endl;
    }
#else
    // Create our display list, starting from our origin coordinates
    std::vector<std::pair<int64_t, int64_t>> display_list;
    root->BuildDisplayList(INT64_C(0), INT64_C(0), display_list);
    std::unordered_map<std::pair<int64_t, int64_t>, bool, PairHash> display_map;

    // Setup our display coordinate map to store alive cells
    int64_t min_x = INT64_MAX;
    int64_t min_y = INT64_MAX;
    int64_t max_x = INT64_MIN;
    int64_t max_y = INT64_MIN;

    // Let's go through our display list, record min and max coordinates, and add to our map for fast access
    for(std::pair<int64_t, int64_t> pair : display_list) {
        if (pair.first < min_x) {
            min_x = pair.first;
        } else if (pair.first > max_x) {
            max_x = pair.first;
        }
        if (pair.second < min_y) {
            min_y = pair.second;
        } else if (pair.second > max_y) {
            max_y = pair.second;
        }
        display_map[pair] = true;
    }
    int display_list_index = 0;
    std::cout << "============================================================\n";
    std::cout << "== Drawing min(" << min_x << ", " << min_y  << ") max(" << max_x << ", " << max_y << ")" << std::endl;
    // Are we small enough to render out to the console?
    if ((max_x - min_x < DEBUG_RENDER_SIZE_MAX) && (max_y - min_y < DEBUG_RENDER_SIZE_MAX)) {
        for (int64_t y = min_y; y <= max_y; ++y) {
            int64_t x;
            for (x = min_x; x <= max_x; ++x) {
                if (display_map[std::make_pair(x, y)]) {
                    std::cout << "*";
                    ++display_list_index;
                } else {

                    std::cout << "_";
                }
            }
            std::cout << std::endl;
        }
    } else {
        // Render size would be too large, so lets just display a list of coordinates
        // Let's go through our display list, record min and max coordinates, and add to our map for fast access
        for(std::pair<int64_t, int64_t> pair : display_list) {
            std::cout << "(" << pair.first << ", " << pair.second << ") ";
        }
        std::cout << std::endl;
    }
#endif
}

/**
 * Print out the current hashtable, mainly for debugging
 */
void QuadTree::PrintHashTable() {
    for(std::pair<QuadTreeNode*, QuadTreeNode*> pair : QuadTreeNode::node_map) {
        QuadTreeNode* node = pair.first;
        if (node->level != 0) {
            std::cout << "Node " << node->level << " " << node->nw->population << " " << node->ne->population;
            std::cout << " " << node->sw->population << " " << node->se->population << " " << node->population << std::endl;
        } else {
            std::cout << "Node " << node->level << " " << node->population << std::endl;
        }
    }
}

#if (ENABLE_GARBAGE_COLLECTION)
/**
 * Collect garbage, depending on whatever scheme is active
 * @return if garbage collection was performed
 */
bool QuadTree::CollectGarbage() {

 #if (GARBAGE_COLLECTION_MODE_GENERATIONS)
    if (num_generations % GARBAGE_COLLECTION_GENERATIONS_COUNT == 0) {
 #else //(GARBAGE_COLLECTON_MODE_NODES)
    if (QuadTreeNode::node_map.size() > GARBAGE_COLLECTION_NODES_COUNT) {
 #endif
        // this map stores nodes that are currently used
        std::unordered_map<QuadTreeNode*, QuadTreeNode*> nodesInUse;

        // recurse to figure out which nodes are in use
        CollectGarbageHelper(nodesInUse, root);
        int64_t numNodes = 0;
        for (auto it = QuadTreeNode::node_map.begin(); it != QuadTreeNode::node_map.end();) {
            if(nodesInUse.find(it->first) == nodesInUse.end()) {
                QuadTreeNode* node = it->first;
                // delete the node first
                delete node;
                // erase the node from the map, AFTER deleting
                it = QuadTreeNode::node_map.erase(it);
                ++numNodes;
            } else {
                it++;
            }
        }
        // remove nodes if they are not in the in use map
        return true;
    }
    return false;
}

/**
 * Helper for garbage collection, called recursively to iterate
 * Through a quad tree node structure
 * @param nodesInUse
 * @param curr
 */
void QuadTree::CollectGarbageHelper(std::unordered_map<QuadTreeNode *, QuadTreeNode *> &nodesInUse, QuadTreeNode *curr) {
    if (curr == 0) {
        return;
    }
    if (nodesInUse.find(curr) == nodesInUse.end()) {
        nodesInUse.insert(std::make_pair(curr, curr));

        CollectGarbageHelper(nodesInUse, curr->calc);
        CollectGarbageHelper(nodesInUse, curr->nw);
        CollectGarbageHelper(nodesInUse, curr->ne);
        CollectGarbageHelper(nodesInUse, curr->sw);
        CollectGarbageHelper(nodesInUse, curr->se);
    }
}
#endif

/*
 * This function is only called when we construct the universe and set initial values
 * input is bounded by signed 64 bit integer input [-2^63, 2^63-1]
 */
void QuadTree::SetCellAlive(int64_t x, int64_t y) {
    while(true) {
        // Calculate the minimum and maximum signed 64-bit integer for this level
        //  min will be -2^(n-1)
        //  max will be 2^(n-1) - 1
        int shift = root->level - 1;
        int64_t min = (root->level == 0) ? 0 : -(INT64_C(1) << shift);
        int64_t max = (root->level == 0) ? 0 : (INT64_C(1) << shift) - 1;

        // if we're in bounds, we're cool.
        // if not, expand the root node one level higher (power of 2 higher in size)
        if ((x >= min && x <= max && y >= min && y <= max)) {
            break;
        } else {
            root = root->Expand();
        }
    }

    root = root->SetCellAlive(x, y);
}

