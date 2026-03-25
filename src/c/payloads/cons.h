/**
 *  payloads/cons.h
 *
 *  A cons cell.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_payloads_cons_h
#define __psse_payloads_cons_h

#include "memory/pointer.h"

/**
 * An ordinary cons cell:
 */
#define CONSTAG     "CNS"

/**
 * @brief A cons cell.
 * 
 */
struct cons_payload {
  /** Contents of the Address Register, naturally. */
    struct pso_pointer car;
  /** Contents of the Decrement Register, naturally. */
    struct pso_pointer cdr;
};


#endif
