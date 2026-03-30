/**
 *  memory/memory.c
 *
 *  The memory management subsystem.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <stdbool.h>
#include <stdio.h>

#include "memory/memory.h"
#include "memory/node.h"
#include "memory/pointer.h"

/**
 * @brief Freelists for each size class.
 */
struct pso_pointer freelists[MAX_SIZE_CLASS];

/**
 * @brief Flag to prevent re-initialisation.
 */
bool memory_initialised = false;

/**
 * @brief Initialise the memory allocation system.
 *
 * Essentially, just set up the freelists; allocating pages will then happen
 * automatically as objects are requested.
 * 
 * @param node the index number of the node we are initialising.
 * @return int 
 */
struct pso_pointer initialise_memory( uint32_t node ) {
    if (memory_initialised) {
        // TODO: throw an exception
    } else {
        for (uint8_t i = 0; i <= MAX_SIZE_CLASS; i++) {
            freelists[i] = nil;
        }
        memory_initialised = true;
    }

    return t;
}
