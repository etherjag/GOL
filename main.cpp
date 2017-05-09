#include <iostream>
#include "quad_tree.h"
#include <gmpxx.h>

static int64_t minPowerOf2(int powerOf2) {
    return -(INT64_C(1) << (powerOf2 - 1));
}

static int64_t maxPowerOf2(int powerOf2) {
    return (INT64_C(1) << (powerOf2 - 1)) - 1;
}


static void addGlider(QuadTree* u);
static void addWeekender(QuadTree* u);
static void addSiesta(QuadTree* u);
static void addMethuselah(QuadTree* u);
static void addOscillator(QuadTree* u);
static void addCornerOscillators(QuadTree* u);
static void addCornerGlider(QuadTree* u);

int main() {
    QuadTree *u = new QuadTree();

    //addSiesta(u);
    //addOscillator(u);
    addWeekender(u);
    //addCornerOscillators(u);
    //addCornerGlider(u);
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10000; ++i) {
        //u->print();
        u->Step();
    }
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();
    u->Print();
    std::cout << "============================================================\n";
    std::cout << "Processed in " << duration << " milliseconds" << std::endl;
    std::cout << "============================================================\n";

    // free our universe
    delete u;
    return 0;
}

static void addOscillator(QuadTree* u) {
    int64_t input[][2] = {
            {0, maxPowerOf2(64)-1},
            {-1, maxPowerOf2(64)-1},
            {1, maxPowerOf2(64)-1},
    };
    u->SetCellsAlive(input, sizeof(input) / (sizeof(int64_t) * 2));
}

static void addGlider(QuadTree* u) {
    int64_t input[][2] = {
            {1, 0}, {2,1}, {0,2}, {1,2}, {2,2}
    };
    u->SetCellsAlive(input, sizeof(input) / (sizeof(int64_t) * 2));
}

static void addWeekender(QuadTree* u) {
    int64_t input[][2] = {
            {2,0},  {15,0},
            {2,1},  {15,1},
            {1,2},  {3,2},  {14,2}, {16,2},
            {2,3},  {15,3},
            {2,4},  {15,4},
            {3,5},  {7,5}, {8,5}, {9,5}, {10,5}, {14,5},
            {7,6},  {8,6}, {9,6}, {10,6},
            {3,7},  {4,7}, {5,7}, {6,7}, {11,7}, {12,7}, {13,7}, {14,7},
            {5,9},  {12,9},
            {6,10}, {7,10}, {10,10}, {11,10},
    };
    u->SetCellsAlive(input, sizeof(input) / (sizeof(int64_t) * 2));
}

static void addSiesta(QuadTree* u) {
    int64_t input[][2] = {
            {13,0}, {14,0},
            {5,1}, {6,1}, {12,1}, {14,1},
            {5,2}, {7,2}, {12,2},
            {7,3}, {11,3}, {12,3}, {14,3},
            {5,4}, {7,4}, {8,4}, {14,4}, {15,4}, {16,4},
            {3,5}, {4,5}, {5,5}, {11,5}, {13,5}, {17,5},
            {2,6}, {6,6}, {8,6}, {14,6}, {15,6}, {16,6},
            {3,7}, {4,7}, {5,7}, {11,7}, {12,7}, {14,7},
            {5,8}, {7,8}, {8,8}, {12,8},
            {7,9}, {12,9}, {14,9},
            {5,10}, {7,10}, {13,10}, {14,10},
            {5,11}, {6,11}
    };
    u->SetCellsAlive(input, sizeof(input) / (sizeof(int64_t) * 2));
}

static void addMethuselah(QuadTree* u) {
    int64_t input[][2] = {
            {-2,-2}, {-2,-1}, {-2,2}, {-1,-2}, {-1,1}, {0,-2}, {0,1},
            {0,2}, {1,0}, {2,-2}, {2,0}, {2,1}, {2,2}
    };
    u->SetCellsAlive(input, sizeof(input) / (sizeof(int64_t) * 2));
}

static void addCornerGlider(QuadTree* u) {
    int64_t cornerBoundary = maxPowerOf2(64) - 3;   // subtract the boundary of the glider
    int64_t input[][2] = {
        {cornerBoundary+1, cornerBoundary+0},
        {cornerBoundary+2, cornerBoundary+1},
        {cornerBoundary+0, cornerBoundary+2},
        {cornerBoundary+1, cornerBoundary+2},
        {cornerBoundary+2, cornerBoundary+2}
    };
    u->SetCellsAlive(input, sizeof(input) / (sizeof(int64_t) * 2));
}

static void addCornerOscillators(QuadTree* u) {
    int64_t input[][2] = {
            // upper left signed int boundary
            {-minPowerOf2(64), -minPowerOf2(64)},
            {-minPowerOf2(64)+1, -minPowerOf2(64)},
            {-minPowerOf2(64)+2, -minPowerOf2(64)},
            // upper right signed int boundary
            {maxPowerOf2(64), -minPowerOf2(64)},
            {maxPowerOf2(64)-1, -minPowerOf2(64)},
            {maxPowerOf2(64)-2, -minPowerOf2(64)},
            // lower left signed int boundary
            {-minPowerOf2(64), maxPowerOf2(64)},
            {-minPowerOf2(64)+1, maxPowerOf2(64)},
            {-minPowerOf2(64)+2, maxPowerOf2(64)},
            // lower right signed int boundary
            {maxPowerOf2(64), maxPowerOf2(64)},
            {maxPowerOf2(64)-1, maxPowerOf2(64)},
            {maxPowerOf2(64)-2, maxPowerOf2(64)},
    };
    u->SetCellsAlive(input, sizeof(input) / (sizeof(int64_t) * 2));
}