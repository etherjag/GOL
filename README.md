# GOL

This project implements John Conway's famous Game Of Life using a recursive, quad-tree structure with memoized, canonical nodes to create cellular automata objects that are either alive or dead and evolve one generation forwared at a time (inspired by HashLife). The prompt given for this exercise was very open-ended, so I chose to try and implement an "infinite" game of life, meaning that given enough memory and processing power this game can continue beyond the initial 64 bit signed coordinates and have infinite quad-tree levels. I used the recursive quad-tree structure to achieve this along with a multi-precision library called GMP to store signed integers larged than the initial 64 bit signed range.

I was not able to get around to actually drawing a board in OpenGL or other, so I use the console to display coordinates and if small enough, an ascii representation of the board is printed out

### Prerequisites
* CLion 2017.1.1 to build and run the code
* GMP Library 6.1.2

### Installing

First, install GMP Version 6.1.2. I'm on a mac and I have homebrew installed, so I just used:

```
brew install gmp
```

Next, install CLion version 2017.1.1 here:

```
https://www.jetbrains.com/clion/
```
## Creating a quad tree and populating it with input
You can manually create a quad tree and population it with input with the following methods:
```
/**
 * Initialize this quad tree before it's actually been run
 * @param input in the form of {x, y}
 * @param num_rows the number of input pairs
*/
void SetCellsAlive(std::vector<std::pair<int64_t, int64_t>> input);
```
```
/**
 * Initialize this quad tree before it's actually been run
 * @param input in the form of {x, y}
 * @param num_rows the number of input pairs
 */
void SetCellsAlive(int64_t input[][2], size_t num_rows);
```
To step the tree a generation forward:
```
QuadTree quad_tree;
// populate with input
quad_tree.SetCellsAlive(input);
// step the simulation
quad_tree.Step();
```
To print debug info/stats:
```
quad_tree.PrintVerbose();
quad_tree.PrintStats();
quad_tree.PrintDisplayCoordinates();
```
## Running the tests

There are two kinds of tests you can run, patterns and densely population randomized nodes created within a specifed boundary system. For patterns, I support loading *.rle files (I IGNORE the rule part of that), which I have also included in the /patterns directory to use for testing.

### Running a *.rle pattern

Using an *rle file to specify a pattern, we can read it in and run a test for n generations, while placing the pattern at an (x, y) coordinate within the signed 64 bit integer range. The pattern is loaded with (x, y) as its upper left corner, so if we detect the pattern will go beyond the signed 64 bit integer range, we move it back so that its bounding box touches the edge.

Here we test the weekender pattern (which moves upwards) by placing it at the negative int64 boundary on y, run it for 1000 generations, and set it to draw to the console
```
QuadTreeTests::RunRLEPatternTest("../patterns/weekender.rle", 1000, 0, INT64_MIN, true);
```

Other examples:
```
// Siesta - 2 cycle oscillator
QuadTreeTests::RunRLEPatternTest("../patterns/siesta.rle", 1000, INT64_MIN, INT64_MIN, true);

// Mickey Mouse - Test a still life at a quad tree boundary
QuadTreeTests::RunRLEPatternTest("../patterns/mickeymouse.rle", 1, MinPowerOf2(16), MaxPowerOf2(16), true);

// Loaf - still life
QuadTreeTests::RunRLEPatternTest("../patterns/loaf.rle", 100, INT64_MAX, INT64_MAX, true);

// Queen Bee at the origin
QuadTreeTests::RunRLEPatternTest("../patterns/queenbee.rle", 21, 0, 0, true);

// Queen Bee Stable - This pattern becomes stable at 191
QuadTreeTests::RunRLEPatternTest("../patterns/queenbee.rle", 191, INT64_MIN, INT64_MIN, true);

// Edna, A methuselah with lifespan 31,192.
// This algorithm doesn't handle giant methuselah's particularly well
// (The first 500 generations of Edna get processed in under a second though)
QuadTreeTests::RunRLEPatternTest("../patterns/edna.rle", 500, INT64_MIN, INT64_MIN, false);
```

### Running a randomized, bounded dense test
To see how this algorithm performs both memory-wise and computationally, we can perform randomized bounded dense tests.
We specify the number of random cells to create, the number of generations to run it for, and two sets of boundaries that define the minimum x and y coordinates, as well as the max x and y coordinates. 

```
// STRESS TEST: Generate cells in a signed 64 bit range with boundaries and generations clamped
QuadTreeTests::RunMegaRandomMaxBoundariesTest(1000, 100, MinPowerOf2(6), MaxPowerOf2(6), MinPowerOf2(6), MaxPowerOf2(6), false);

// STRESS TEST: 200-300k cells initialized
QuadTreeTests::RunMegaRandomMaxBoundariesTest(MaxPowerOf2(12) * MaxPowerOf2(10), 0, MinPowerOf2(9), MaxPowerOf2(9), MinPowerOf2(9), MaxPowerOf2(9), false);

// STRESS TEST: Generate millions of random cells. This is the worst case scenario
// This tries to set about 4 million cells and evolves to 4 million nodes, and can use up to 2GB of memory (!!!)
QuadTreeTests::RunMegaRandomMaxBoundariesTest(MaxPowerOf2(12) * MaxPowerOf2(12), 1, MinPowerOf2(12), MaxPowerOf2(12), MinPowerOf2(12), MaxPowerOf2(12), false);
```
### Optimization Options found in quad_tree_config.h
A few different options can be configured in quad_tree_config.h:

Enable quad tree center alignment (optimization)
```
/**
 * This is an optimization to align a quadtree to the center coordinate of all of the initial input
 * This saves us memory by not generating a complete structure, especially if the initial input is clustered in a region away
 * From the origin
 */
#define ENABLE_QUADTREE_CENTER_ALIGN            1&&ENABLE_BIG_INT // enable/disable aligning the quad tree to the center of input
```
Enable Infinite Levels (optimization)
```
/**
 * This define turns on supporting infinite levels. With this off, we support up to an int value of levels (which is 2^n in size)
 * but with multi-precision on and this, we store the level as a multi-precision value. We precalculate values of two up to LEVEL_MAX
 * levels, and after that, we manually do it.
 */
#define ENABLE_INFINITE_LEVELS 0&&ENABLE_BIG_INT   // enable infinite levels, which stores the current level as a multi-precision value
```

Enable garbage collection (optimization)
```
/**
 * Enable/Disable garbage collection, of which there are currently two modes:
 * 1) Collect every N generations
 * 2) Collect once past N number of nodes
 */
#define ENABLE_GARBAGE_COLLECTION               1   // disable garbage collection of nodes
```
Garbage Collection modes
```
/**
 * Two garbage collection modes exist.
 * 1) Generations: Clean up nodes every x generations
 *  - A higher value could mean an exponentially higher number of nodes per generation, but this method works better
 *      for a large board with lots of nodes
 * 2) Nodes: Clean up nodes once we pass a specific threshold
 *  - A higher value is faster if we are just running a simulation, although it will use more max memory.
 *      This will cause a render hitch due to the high volume if we are actually rendering.
 *  - A lower value is better if we are rendering and stepping every frame, because it won't cause a hitch
 *  - Possible problems: If we have a huge board, this will run every frame because of the node count
 */
#define GARBAGE_COLLECTION_MODE_GENERATIONS     (1&&ENABLE_GARBAGE_COLLECTION)         // collect garbage every N generations
#define GARBAGE_COLLECTION_MODE_NODES           (0&&ENABLE_GARBAGE_COLLECTION)         // collect garbage once we pass N nodes

/**
 * Params for garbage collection modes
 */
#if (GARBAGE_COLLECTION_MODE_GENERATIONS)
#define GARBAGE_COLLECTION_GENERATIONS_COUNT    1000      // number of generations to collect garbage
#elif (GARBAGE_COLLECTION_MODE_NODES)
#define GARBAGE_COLLECTION_NODES_COUNT          100000    // number of nodes threshold to collect garbage
#endif
```


Enable Big Integers. This probably needs to be on now.
```
/**
 * This define turns on using big integers. Currently we are using The GNU Multiple Precision Arithmetic Library 6.1.2.
 * If this is turned off, we can run just fine unless we have nodes at boundaries, in which the coordinates will wrap
 * in an odd way (we haven't accounted for a bounded space with this off!)
 */
#define ENABLE_BIG_INT 1   // enable/disable use of big integers. probably don't turn this off now..
```

Big Int Powers of 2 Table
We pre-process a powers of 2 table to account for up to LEVEL_MAX levels (which really means 2^level*2^level nodes).
If we get beyond this level, we manually calculate the power of 2 to ensure infinity is possible.
```
#if (ENABLE_BIG_INT)
 #define LEVEL_MAX                              256    // number of levels to calculate to determine bigint coordinates, in powers of 2
#endif
```
## Points of Discussion
### Data structures and optimizations implemented:
* Recursive, quad tree structure
* Memoized, Canonical quad tree nodes
* Simple garbage collection with 2 different modes
* *.rle pattern reading supported
* Multi-precision integers used for keeping track of population and calculating display coordinates
* Pre-process the input values and create a quad-tree at the average x, y value to minimize the number of levels we need to create. This, in particular, optimizes clusters of data that are away from the origin such as our signed 64 bit integer values. Testing out an edna pattern, this leads to a 2x speedup when placed at an int boundary as well as reducing the maximum number of nodes that get created:
```
Edna WITH optimization
Overview: Generation (10000) Population (1992) Tree Level (13)
		Current # nodes: 1461
		Current Heap memory usage: 91 KB
		All Time # nodes: 5876921
		NW Population: 80
		NE Population: 162
		SW Population: 549
		SE Population: 1201
DONE: Processed test in 26814 milliseconds

Edna WITHOUT optimization
	Overview: Generation (10000) Population (1992) Tree Level (65)
		Current # nodes: 1673
		Current Heap memory usage: 104 KB
		All Time # nodes: 11346024
		NW Population: 1992
		NE Population: 0
		SW Population: 0
		SE Population: 0
DONE: Processed test in 44347 milliseconds

```

### Improvements to be made:
* Tree construction is slow because we are continually creating new nodes and throwing away others
* Building the display list is also slow because we recurse through everything. How can we integrate this as we process the tree?
* Have a leaf node be a higher level up, that way we dont have to dereference pointers to calculate the game of life rule, such as a 2x2 leaf node
* Better garbage collection? My methods don't benefit if the input has millions of nodes.
* Node free list? Allocate nodes in chunks and use a free list
* Better hash function?
* Reduce memory usage by using leaf nodes separate from non-leafs
* Calculate the population count instead of storing it as a multi-precision int
* How would paralleism work?
* Better test framework

### Alternative Approaches:
* Modern, parallelized approach?
* Hash table of active cells?
* Store and sort bounded areas of cells, such as sorting triangles is done?
* KD tree? R tree? could any of these perform better?
* A wildly different alternative...??

Things I did not get around to:
* Drawing!!!

## Code Style

No style guide was provided to me, so I decided to use Google C++ Style guide, found here:
https://google.github.io/styleguide/cppguide.html

## Built With

* [CLion](https://www.jetbrains.com/clion/) - C++ IDE, Version 2017.1.1
* [GMP](https://gmplib.org/) - GNU's multiprecision arithmetic library, Version 6.1.2
* [C++ 11](https://en.wikipedia.org/wiki/C%2B%2B11) - C++ Version 11, mainly because that's what CLion defaults to..

## Authors

* **Jenny Spurlock** 

## License

This project is for an interview test and should not be used for any other purposes.

## Acknowledgments
 * Golly Creators and contributors (Andrew Trevorrow and Tom Rokicki, with code contributions by Tim Hutton, Dave Greene, Jason Summers, Maks Verver, Robert Munafo, Brenton Bostick and Chris Rowett) (http://golly.sourceforge.net/)
  * Game of Life Wiki, for providing .rle patterns (http://conwaylife.com/w/index.php?title=Main_Page)
  * John Conway, for inventing the Game of Life
