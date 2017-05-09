//
// Created by Jenny Spurlock on 5/8/17.
//
#include <vector>
#include <assert.h>
#include "quad_tree_node.h"

// canonical map of all of our nodes
std::unordered_map<QuadTreeNode*, QuadTreeNode*, QuadTreeNode::HashFunction, QuadTreeNode::EqualFunction> QuadTreeNode::node_map;

// helper variables to let us know how many nodes have been created total
int64_t QuadTreeNode::num_nodes_created = 0;

#if (ENABLE_BIG_INT)
// precalculated multi precision powers of two
mpz_class QuadTreeNode::mpz_pow2_table[LEVEL_MAX];
#endif

/**
 * Static initialize because we have some work to do, like initialize a multi precision
 * power of 2 table
 */
void QuadTreeNode::Initialize() {
#if (ENABLE_BIG_INT)
    QuadTreeNode::InitializePow2Table();
#endif
}

/**
 * Static shutdown because we have some work to do, like freeing all of our nodes that were
 * created because they are canonical
 */
void QuadTreeNode::Shutdown() {
    // clean up all nodes in our hash table
    for (auto it = QuadTreeNode::node_map.begin(); it != QuadTreeNode::node_map.end(); ++it) {
        delete (it->first);
    }
    // clear out the map
    node_map.clear();
}

/**
 * Build an empty quadtree recursively at the specified level
 * @param level this level represents the power of 2 dimensions of this quad tree, which is square
 * @return a new empty quad tree at the specified level
 */
QuadTreeNode* QuadTreeNode::EmptyQuadTree(level_type level) {
    // if i'm a leaf, then return an interned node with alive set to false
    if (level == 0) {
        return Canonical(0);
    }
    // handle non-leaf nodes
    QuadTreeNode* emptyNode = EmptyQuadTree(level - 1);
    return Canonical(emptyNode, emptyNode, emptyNode, emptyNode, level);
}


/**
 * Copy Constructor
 * @param other the other node to be copied from
 */
QuadTreeNode::QuadTreeNode (const QuadTreeNode& other) {
    nw = other.nw;
    ne = other.ne;
    sw = other.sw;
    se = other.se;
    calc = other.calc;
    alive = other.alive;
    level = other.level;
#if (ENABLE_BIG_INT)
    mpz_init_set(population, other.population);
#else
    population = other.population;
#endif
}

/**
 * Operator ==
 * @param other the other node to be compared
 * @return if two nodes are equal
 */
bool QuadTreeNode::operator==(const QuadTreeNode &other) const {
    if (level != other.level) {
        return false;
    } else if (level == 0) {
        return alive == other.alive;
    } else {
        return nw == other.nw
               && ne == other.ne
               && sw == other.sw
               && se == other.se;
    }
}

/**
 * Expand this node by one level (a power of 2) so that we can effectively process inner squares
 * and not hit boundaries
 * @return a new node one level higher
 */
QuadTreeNode* QuadTreeNode::Expand() {
    // We're expanding by a factor of 2, so let's create a level above so that we have empty regions to calculate the life rule
    QuadTreeNode* emptyRegion = EmptyQuadTree(level - 1);
    QuadTreeNode* newNW = Canonical(emptyRegion, emptyRegion, emptyRegion, this->nw, level);
    QuadTreeNode* newNE = Canonical(emptyRegion, emptyRegion, this->ne, emptyRegion, level);
    QuadTreeNode* newSW = Canonical(emptyRegion, this->sw, emptyRegion, emptyRegion, level);
    QuadTreeNode* newSE = Canonical(this->se, emptyRegion, emptyRegion, emptyRegion, level);
    return Canonical(newNW, newNE, newSW, newSE, level + 1);
}

/**
 * Can we compact this node any smaller? We call this after evolving the root
 * Currently, we can't call this on sub-nodes that are evolved because we always assume
 * pointers to regions are valid
 * @return a new root (possibly) compacted one level smaller
 */
QuadTreeNode* QuadTreeNode::Compact() {
    QuadTreeNode* root = this;
    // pop off levels we dont need
    level_type level = root->level;
    while (level >= 3) {
        QuadTreeNode *empty_tree = QuadTreeNode::EmptyQuadTree(level - 2);
        if (root->nw->nw == empty_tree && root->nw->ne == empty_tree && root->nw->sw == empty_tree
             && root->ne->nw == empty_tree && root->ne->ne == empty_tree && root->ne->se == empty_tree
             && root->sw->nw == empty_tree && root->sw->sw == empty_tree && root->sw->se == empty_tree
             && root->se->ne == empty_tree && root->se->sw == empty_tree && root->se->se == empty_tree
        ) {
            level--;
            root = Canonical(root->nw->se, root->ne->sw, root->sw->ne, root->se->nw, level);
        } else {
            break;
        }
    }
    return root;
}

/**
 * The fun part. Evolve this node and all of its sub children one generation forward based on Conway's Game of Life Rule
 * Reference: https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life
 * @return a new node pointing to a QuadTree structure evolved one generation forwared
 */
QuadTreeNode* QuadTreeNode::Evolve() {
    if (calc == 0) {
#if (ENABLE_BIG_INT)
        if (mpz_cmp_si(population, 0) == 0) {
#else
        if (population == 0) {
#endif
            calc = nw;
        } else if (level == 2) {
            calc = EvolveLevel2();
        } else {
            calc = EvolveLevelN();
        }
    }
    return calc;
}

/**
 * Turn a cell alive
 * This is ONLY called before we run any simulation and is bounded by signed 64 bit integers
 * input is bounded by signed 64 bit integer input [-2^63, 2^63-1]
 * @param x x coordinate for this simulation (signed 64 bit integer input [-2^63, 2^63-1])
 * @param y y coordinate for this simulation (signed 64 bit integer input [-2^63, 2^63-1])
 * @return
 */
QuadTreeNode* QuadTreeNode::SetCellAlive(int64_t x, int64_t y) {
    if (level == 0) {
        // Return a level 0 alive cell
        return Canonical(1);
    }
#if (ENABLE_INFINITE_LEVELS)
    int64_t offset = (level == 1) ? 0 : INT64_C(1) << (level.get_si() - 2);
#else
    int64_t offset = (level == 1) ? 0 : INT64_C(1) << (level - 2);
#endif
    // Check west quadrants
    if (x < 0) {
        // Northwest
        if (y < 0) {
            return Canonical(nw->SetCellAlive(x + offset, y + offset), ne, sw, se, level);
        }
        // Southwest
        else {
            return Canonical(nw, ne, sw->SetCellAlive(x + offset, y - offset), se, level);
        }
    }
        // Check east quadrants
    else {
        // Northeast
        if (y < 0) {
            return Canonical(nw, ne->SetCellAlive(x - offset, y + offset), sw, se, level);
        }
        // Southeast
        else {
            return Canonical(nw, ne, sw, se->SetCellAlive(x - offset, y - offset), level);
        }
    }
}

#if (ENABLE_BIG_INT)
/**
 * Build a display list of coordinates sorted by x and y
 * This will returns a list or cells that are currently alive and their current coordinates
 * @param origin_x multi precision starting coordinate of this quadtree
 * @param origin_y multi precision starting coordinate of this quadtree
 * @list vector of coordinates to return
 * todo: optimize
 */
void QuadTreeNode::BuildDisplayList(mpz_class origin_x, mpz_class origin_y, std::vector<std::pair<mpz_class, mpz_class>>& list) {
    //std::cout << origin_x << ", " << origin_y << std::endl;
    if (level == 0) {
        if (alive) {
            list.push_back(std::make_pair(origin_x, origin_y));
        }
    } else {
        mpz_class offset = 0;
        if (level > 1) {
            if ((level - 2) < LEVEL_MAX) {
                // we're still within signed int boundaries for level
#if (ENABLE_INFINITE_LEVELS)
                offset = mpz_pow2_table[level.get_si() - 2];
#else
                offset = mpz_pow2_table[level - 2];
#endif
            } else {
                // God, this is slow, but we've run out of precalculated multi-precision powers of 2 because
                // our level must be ginormous. We take care with boundaries here.
                mpz_class max = (level - 2) - (LEVEL_MAX - 1);
                mpz_class i = 0;
                for (; i < max; ++i) {
                    offset = offset * 2;
                }
            }
        }
        //std::cout << offset << std::endl;
        if (mpz_cmp_si(nw->population, 0) != 0) {
            //std::cout << "nw" << level << " " << nw->population << " " << origin_x << " " << origin_y << " " << offset << std::endl;
            if (level == 1) {
                nw->BuildDisplayList(origin_x - 1, origin_y - 1, list);
            } else {
                nw->BuildDisplayList(origin_x - offset, origin_y - offset, list);
            }
        }
        if (mpz_cmp_si(ne->population, 0) != 0) {
            //std::cout << "ne" << level << " " << origin_x << " " << origin_y << " " << offset << std::endl;
            if (level == 1) {
                ne->BuildDisplayList(origin_x, origin_y-1, list);
            } else {
                ne->BuildDisplayList(origin_x + offset, origin_y - offset, list);
            }
        }
        if (mpz_cmp_si(sw->population, 0) != 0) {
            //std::cout << "sw" << level << " " << origin_x << " " << origin_y << " " << offset << std::endl;
            if (level == 1) {
                sw->BuildDisplayList(origin_x - 1, origin_y, list);
            } else {
                sw->BuildDisplayList(origin_x - offset, origin_y + offset, list);
            }
        }
        if (mpz_cmp_si(se->population, 0) != 0) {
            //std::cout << "se" << level <<" " << origin_x << " " << origin_y << " " << offset << std::endl;
            if (level == 1) {
                se->BuildDisplayList(origin_x, origin_y, list);
            } else {
                se->BuildDisplayList(origin_x + offset, origin_y + offset, list);
            }
        }
    }
}

#else
/**
 * Build a display list of coordinates sorted by x and y
 * This will returns a list or cells that are currently alive and their current coordinates
 * @param origin_x signed 64 bit starting coordinate of this quadtree
 * @param origin_y signed 64 bit starting coordinate of this quadtree
 * @list vector of coordinates to return
 * todo: optimize
 */
void QuadTreeNode::BuildDisplayList(int64_t origin_x, int64_t origin_y, std::vector<std::pair<int64_t, int64_t>>& list) {
 #if (ENABLE_DEBUG_PRINT)
        std::cout << originX << ", " << originY << std::endl;
 #endif
    if (level == 0) {
        if (alive) {
            list.push_back(std::make_pair(origin_x, origin_y));
        }
    } else {
        int64_t offset = (level == 1) ? 0 : INT64_C(1) << (level - 2);

 #if (ENABLE_DEBUG_PRINT)
        std::cout << offset << std::endl;
 #endif
        if (nw->population > 0) {
 #if (ENABLE_DEBUG_PRINT)
            std::cout << "nw" << level << " " << originX << " " << originY << " " << offset << std::endl;
 #endif
            if (level == 1) {
                nw->BuildDisplayList(origin_x - 1, origin_y - 1, list);
            } else {
                nw->BuildDisplayList(origin_x - offset, origin_y - offset, list);
            }
        }
        if (ne->population > 0) {
            //std::cout << "ne" << level <<" " << originX << " " << originY << " " << offset << std::endl;
            if (level == 1) {
                ne->BuildDisplayList(origin_x, origin_y - 1, list);
            } else {
                ne->BuildDisplayList(origin_x + offset, origin_y - offset, list);
            }
        }
        if (sw->population > 0) {
            //std::cout << "sw" << level <<" " << originX << " " << originY << " " << offset << std::endl;
            if (level == 1) {
                sw->BuildDisplayList(origin_x - 1, origin_y, list);
            } else {
                sw->BuildDisplayList(origin_x - offset, origin_y + offset, list);
            }
        }
        if (se->population > 0) {
            //std::cout << "se" << level <<" " << originX << " " << originY << " " << offset << std::endl;
            if (level == 1) {
                se->BuildDisplayList(origin_x, origin_y, list);
            } else {
                se->BuildDisplayList(origin_x + offset, origin_y + offset, list);
            }
        }

    }
}
#endif


/**
 * This function looks up a node with level > 0 in the hash table and returns a canonical one
 * if it exists. If it doesn't exist, it creates a new node and adds it
 * @param node quad tree node
 * @return a new, canonical node
 */
QuadTreeNode* QuadTreeNode::Canonical(QuadTreeNode* nw, QuadTreeNode* ne, QuadTreeNode* sw, QuadTreeNode* se, level_type level) {
    // Create a node on the stack to look up the canonical version
    QuadTreeNode node(nw, ne, sw, se, level);
    // Return a pointer to the canonical version
    return Canonical(&node);
}

/**
 * This function looks up a leaf node in the hash table and returns a canonical one
 * if it exists. If it doesn't exist, it creates a new node and adds it
 * @param node quad tree node
 * @return a new, canonical node
 */
QuadTreeNode* QuadTreeNode::Canonical(QuadTreeNode* node) {
    std::unordered_map<QuadTreeNode*, QuadTreeNode*>::const_iterator iter = node_map.find(node);
    // if this node isn't in the map, add it
    if (iter == node_map.end()) {
        // create a new node on the heap with the copy constructor
        QuadTreeNode* newNode = new QuadTreeNode(*node);
        // insert the node in the map
        node_map.insert(std::make_pair(newNode, newNode));
        // increment stats
        num_nodes_created++;
        return newNode;
    }
    return iter->first;
}

/**
 * This function looks up a node in the hash table and returns a canonical one
 * if it exists. If it doesn't exist, it creates a new node and adds it
 * @param node quad tree node
 * @return a new, canonical node
 */
QuadTreeNode* QuadTreeNode::Canonical(int alive) {
    // Create a node on the stack
    QuadTreeNode node(alive);
    // Return a pointer to the canonical version
    return Canonical(&node);
}

/**
 * Non-leaf node constructor
 * @param nw northwest corner node
 * @param ne southeast corner node
 * @param sw southwest corner node
 * @param se southeast corner node
 * @param level current level this node represents (2^level is the side dimension of this square, which is NxN)
 */
QuadTreeNode::QuadTreeNode(QuadTreeNode* nw, QuadTreeNode* ne, QuadTreeNode* sw, QuadTreeNode* se, level_type level) {
    this->nw = nw;
    this->ne = ne;
    this->sw = sw;
    this->se = se;
    this->calc = 0;
    this->level = level;
#if (ENABLE_BIG_INT)
    mpz_init(population);
    mpz_add(population, nw->population, ne->population);
    mpz_add(population, population, sw->population);
    mpz_add(population, population, se->population);
    alive = (population > 0) ? 1 : 0;
#else
    population = nw->population + ne->population + sw->population + se->population;
                alive = (population > 0) ? 1 : 0;
#endif
}


/**
 * Leaf node constructor (1x1 square, level 0, which is 2^0 x 2^0 in size)
 * @param alive is this cell alive?
 */
QuadTreeNode::QuadTreeNode(int alive) {
    this->alive = alive;
    nw = 0;
    ne = 0;
    sw = 0;
    se = 0;
    calc = 0;
    level = 0;
#if (ENABLE_BIG_INT)
    mpz_init_set_si(population, alive & 1);
#else
    population = (alive & 1);
#endif
}

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
QuadTreeNode* QuadTreeNode::EvolveLevel2() {
    // calculate our inner region
    // todo: accessing all of these pointers can cause cache misses/is probably slow. we can optimize this by making a level 2 node a leaf
    // node and packing all of these into a short (16 bits)
    QuadTreeNode* newNW = Canonical(RunRule(nw->se->alive,
                               nw->nw->alive + nw->ne->alive + ne->nw->alive + nw->sw->alive + ne->sw->alive +
                               sw->nw->alive + sw->ne->alive + se->nw->alive));
    QuadTreeNode* newNE = Canonical(RunRule(ne->sw->alive,
                               nw->ne->alive + ne->nw->alive + ne->ne->alive + nw->se->alive + ne->se->alive +
                               sw->ne->alive + se->nw->alive + se->ne->alive));
    QuadTreeNode* newSW = Canonical(RunRule(sw->ne->alive,
                               nw->sw->alive + nw->se->alive + ne->sw->alive + sw->nw->alive + se->nw->alive +
                               sw->sw->alive + sw->se->alive + se->sw->alive));
    QuadTreeNode* newSE = Canonical(RunRule(se->nw->alive,
                               nw->se->alive + ne->sw->alive + ne->se->alive + sw->ne->alive + se->ne->alive +
                               sw->se->alive + se->sw->alive + se->se->alive));
    return Canonical(newNW, newNE, newSW, newSE, level-1);
}


/**
 * Function to evolve a level N square, which is 2^N by 2^N in size and N is > 2
 * Level 2 squares evolve their inner squares, and a level above we combine the results
 * and keep recursing down. This algorithm works because the tree has an empty border to ensure
 * That calculations will be correct at edges
 *   +--+--+--+--+--+--+--+--+--+
 *   |                          |
 *   +   +--+--+--+--+--+--+    +
 *   |   |__|__|__|__|__|__|    |
 *   |   |  |  |  |  |  |  |    |
 *   +   +--+--+--+--+--+--+    +
 *   |   |__|__|__|__|__|__|    |
 *   |   |  |  |  |  |  |  |    |
 *   +   +--+--+--+--+--+--+    +
 *   |   |__|__|__|__|__|__|    |
 *   |   |  |  |  |  |  |  |    |
 *   +   +--+--+--+--+--+--+    +
 *   |                          |
 *   +--+--+--+--+--+--+--+--+--+
 * @return a new calculated result for a level N square, one level down
 */
QuadTreeNode* QuadTreeNode::EvolveLevelN() {
    QuadTreeNode* n00 = GetInnerNWNode();
    QuadTreeNode* n01 = GetInnerNNode();
    QuadTreeNode* n02 = GetInnerNENode();
    QuadTreeNode* n10 = GetInnerWNode();
    QuadTreeNode* n11 = GetInnerCNode();
    QuadTreeNode* n12 = GetInnerENode();
    QuadTreeNode* n20 = GetInnerSWNode();
    QuadTreeNode* n21 = GetInnerSNode();
    QuadTreeNode* n22 = GetInnerSENode();
    QuadTreeNode* newNW = Canonical(n00, n01, n10, n11, n00->level+1)->Evolve();
    QuadTreeNode* newNE = Canonical(n01, n02, n11, n12, n00->level+1)->Evolve();
    QuadTreeNode* newSW = Canonical(n10, n11, n20, n21, n00->level+1)->Evolve();
    QuadTreeNode* newSE = Canonical(n11, n12, n21, n22, n00->level+1)->Evolve();
    return Canonical(newNW, newNE, newSW, newSE, newNW->level + 1);
}

/**
 * Helper function to get inner nodes
 */
QuadTreeNode* QuadTreeNode::GetInnerNWNode() {
    return Canonical(nw->nw->se, nw->ne->sw, nw->sw->ne, nw->se->nw, level-2);
}

/**
 * Helper function to get inner nodes
 */
QuadTreeNode* QuadTreeNode::GetInnerNNode() {
    QuadTreeNode* w = nw;
    QuadTreeNode* e = ne;
    return Canonical(w->ne->se, e->nw->sw, w->se->ne, e->sw->nw, level-2);
}

/**
 * Helper function to get inner nodes
 */
QuadTreeNode* QuadTreeNode::GetInnerNENode() {
    return Canonical(ne->nw->se, ne->ne->sw, ne->sw->ne, ne->se->nw, level-2);
}

/**
 * Helper function to get inner nodes
 */
QuadTreeNode* QuadTreeNode::GetInnerWNode() {
    QuadTreeNode* n = nw;
    QuadTreeNode* s = sw;
    return Canonical(n->sw->se, n->se->sw, s->nw->ne, s->ne->nw, level-2);
}

/**
 * Helper function to get inner nodes
 */
QuadTreeNode* QuadTreeNode::GetInnerCNode() {
    return Canonical(nw->se->se, ne->sw->sw, sw->ne->ne, se->nw->nw, level-2);
}

/**
 * Helper function to get inner nodes
 */
QuadTreeNode* QuadTreeNode::GetInnerENode() {
    QuadTreeNode* n = ne;
    QuadTreeNode* s = se;
    return Canonical(n->sw->se, n->se->sw, s->nw->ne, s->ne->nw, level-2);
}

/**
 * Helper function to get inner nodes
 */
QuadTreeNode* QuadTreeNode::GetInnerSWNode() {
    return Canonical(sw->nw->se, sw->ne->sw, sw->sw->ne, sw->se->nw, level-2);
}

/**
 * Helper function to get inner nodes
 */
QuadTreeNode* QuadTreeNode::GetInnerSNode() {
    QuadTreeNode* w = sw;
    QuadTreeNode* e = se;
    return Canonical(w->ne->se, e->nw->sw, w->se->ne, e->sw->nw, level-2);
}

/**
 * Helper function to get inner nodes
 */
QuadTreeNode* QuadTreeNode::GetInnerSENode() {
    return Canonical(se->nw->se, se->ne->sw, se->sw->ne, se->se->nw, level-2);
}

/**
 * Run the game of life rule give an alive status and a neighbor count
 * @param alive current alive status
 * @param count current neighbor count
 * @return if this node is alive in the next generation
 */
int QuadTreeNode::RunRule(int alive, int count) {
    if (alive) {
        return (count == 2 || count == 3) ? 1 : 0;
    } else {
        return (count == 3) ? 1 : 0;
    }
}


#if (ENABLE_BIG_INT)
/**
 * This function looks up a node in the hash table and returns a canonical one
 * if it exists. If it doesn't exist, it creates a new node and adds it
 * @param node quad tree node
 * @return a new, canonical node
 */
void QuadTreeNode::InitializePow2Table() {
    mpz_pow2_table[0] = 1;
    for(int i = 1; i < LEVEL_MAX; i++) {
        mpz_pow2_table[i] = mpz_pow2_table[i-1] * 2;
    }
}
#endif