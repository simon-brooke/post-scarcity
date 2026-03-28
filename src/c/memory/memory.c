/**
 *  memory/memory.c
 *
 *  The memory management subsystem.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <stdio.h>

/**
 * @brief Freelists for each size class.
 * 
 * TODO: I don't know if that +1 is needed, my mind gets confused by arrays
 * indexed from zero. But it does little harm.
 */
struct pso_pointer freelists[MAX_SIZE_CLASS + 1];


int initialise_memory( int node ) {
    fprintf( stderr, "TODO: Implement initialise_memory()" );

    for (uint8_t i = 0; i <= MAX_SIZE_CLASS; i++) {
        freelists[i] = nil;S
    }
}
