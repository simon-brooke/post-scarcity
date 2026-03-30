/**
 *  payloads/vector_pointer.h
 *
 *  A pointer to an object in vector space.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_payloads_vector_pointer_h
#define __psse_payloads_vector_pointer_h

#include <stdbool.h>

#include "memory/pointer.h"

/**
 * A pointer to an object in vector space.
 */
#define VECTORPOINTTAG  "VSP"
#define VECTORPOINTTV   5264214

/**
 * @brief payload of a vector pointer cell.
 */
struct vectorp_payload {
    /** the tag of the vector-space object. NOTE that the vector space object
     * should itself have the identical tag. */
    union {
        /** the tag (type) of the vector-space object this cell
         * points to, considered as bytes. */
        char bytes[TAGLENGTH];
        /** the tag considered as a number */
        uint32_t value;
    } tag;
    /** unused padding to word-align the address */
    uint32_t padding;
    /** the address of the actual vector space object */
    void *address;
};

#endif
