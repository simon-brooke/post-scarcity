/**
 *  memory/header.h
 *
 *  Header for all page space objects
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_memory_header_h
#define __psse_memory_header_h

#include <bits/stdint-uintn.h>

#include "memory/pointer.h"

#define TAGLENGTH 3

/**
 * @brief Header for all paged space objects.
 * 
 */
struct pso_header {
    union {
        /** the tag (type) of this object,
         * considered as bytes */
        struct {
            /** mnemonic for this type; */
            char mnemonic[TAGLENGTH];
            /** size class for this object */
            uint8_t size_class;
        } tag;
        /** the tag considered as a number */
        uint32_t value;
    } tag;
    /** the count of the number of references to this object */
    uint32_t count;
    /** pointer to the access control list of this object */
    struct pso_pointer access;
};

#endif
