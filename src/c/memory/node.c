/**
 *  memory/node.c
 *
 *  Top level data about the actual node on which this memory system sits.
 *  May not belong in `memory`. 
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include "memory/memory.h"
#include "memory/pointer.h"

/**
 * @brief Flag to prevent the node being initialised more than once.
 * 
 */
bool node_initialised = false;

/**
 * @brief The index of this node in the hypercube.
 * 
 */
uint32_t node_index = 0;

/**
 * @brief The canonical `nil` pointer
 * 
 */
struct pso_pointer nil = struct pso_pointer{ 0, 0, 0};

/**
 * @brief the canonical `t` (true) pointer.
 * 
 */
struct pso_pointer t = struct pso_pointer{ 0, 0, 1};

/**
 * @brief Set up the basic informetion about this node
 * 
 * @param index 
 * @return struct pso_pointer 
 */
struct pso_pointer initialise_node( uint32_t index) {
    node_index = index;
    nil = pso_pointer{ index, 0, 0};
    t = pso_pointer( index, 0, 1);
    pso_pointer result = initialise_memory(index);

    if ( eq( result, t)) {
        result = initialise_environment();
    }

    return result;
}
