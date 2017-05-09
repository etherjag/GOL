//
// Created by Jenny Spurlock on 5/8/17.
//

#ifndef GOL_QUADTREECONFIG_H
#define GOL_QUADTREECONFIG_H

/**
 * This define turns on using big integers. Currently we are using The GNU Multiple Precision Arithmetic Library 6.1.2.
 * If this is turned off, we can run just fine unless we have nodes at boundaries, in which the coordinates will wrap
 * in an odd way (we haven't accounted for a bounded space with this off!)
 */
#define ENABLE_BIG_INT                          1          // enable/disable use of big integers
#define ENABLE_GARBAGE_COLLECTION               1          // enable/disable garbage collection of nodes
#define DEBUG_RENDER_SIZE_MAX                   128        // Largest board size we can render to the console

/**
 * Two garbage collection modes exist.
 * 1) Generations: Clean up nodes every x generations
 *  - A higher value could mean an exponentially higher number of nodes per generation
 *      but I wanted to experiment with this method
 * 2) Nodes: Clean up nodes once we pass a specific threshold
 *  - A higher value is faster if we are just running a simulation, although it will use more max memory.
 *      This will cause a render hitch due to the high volume if we are actually rendering.
 *  - A lower value is better if we are rendering and stepping every frame, because it won't cause a hitch
 */
#define GARBAGE_COLLECTION_MODE_GENERATIONS     (0&&ENABLE_GARBAGE_COLLECTION)         // collect garbage every N generations
#define GARBAGE_COLLECTION_MODE_NODES           (1&&ENABLE_GARBAGE_COLLECTION)         // collect garbage once we pass N nodes

#if (GARBAGE_COLLECTION_MODE_GENERATIONS)
#define GARBAGE_COLLECTION_GENERATIONS_COUNT    5000      // number of generations to collect garbage
#elif (GARBAGE_COLLECTION_MODE_NODES)
#define GARBAGE_COLLECTION_NODES_COUNT          100000    // number of nodes threshold to collect garbage
#endif

/**
 * Enable/disable debug printing
 */
#define ENABLE_DEBUG_PRINT                      0     // enable/disable debug printing (this will drastically slow everything down

#if (ENABLE_BIG_INT)
 #define LEVEL_MAX 68
#endif

#endif //GOL_QUADTREECONFIG_H