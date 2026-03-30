/**
 *  environment/environment.c
 *
 *  Initialise a MINIMAL environment.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <stdbool.h>

#include "memory/node.h"
#include "memory/pointer.h"

/**
 * @brief Flag to prevent re-initialisation.
 */
bool environment_initialised = false;

/**
 * @brief Initialise a minimal environment, so that Lisp can be bootstrapped.
 * 
 * @param node theindex of the node we are initialising.
 * @return struct pso_pointer t on success, else an exception.
 */

struct pso_pointer initialise_environment( uint32_t node ) {
    struct pso_pointer result = t;
    if ( environment_initialised ) {
        // TODO: throw an exception "Attempt to reinitialise environment"
    } else {
        // TODO: actually initialise it.
    }

    return result;
}
