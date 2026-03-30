/**
 *  memory/node.h
 *
 *  The node on which this instance resides.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <stddef.h>

#include "memory/node.h"
#include "memory/page.h"
#include "memory/pointer.h"
#include "memory/pso.h"

/**
 * @brief Make a pointer to a paged-space object.
 * 
 * @param node The index of the node on which the object is curated;
 * @param page The memory page in which the object resides;
 * @param offset The offset, in words, within that page, of the object.
 * @return struct pso_pointer a pointer referencing the specified object.
 */
struct pso_pointer make_pointer( uint32_t node, uint16_t page, uint16_t offset) {
    return (struct pso_pointer){ node, page, offset};
}

/**
 * @brief returns the in-memory address of the object indicated by this 
 * pointer. TODO: Yhe reason I'm doing it this way is because I'm not
 * certain reference counter updates work right it we work with 'the object'
 * rather than 'the address of the object'. I really ought to have a 
 * conversation with someone who understands this bloody language.
 * 
 * @param pointer a pso_pointer which references an object.
 * @return struct pso2* the actual address in memory of that object.
 */
struct pso2* pointer_to_object( struct pso_pointer pointer) {
    struct pso2* result = NULL;

    if ( pointer.node == node_index) {
    	union page* pg = pages[pointer.page];
        result = (struct pso2*) &pg->words[pointer.offset];
    }
    // TODO: else if we have a copy of the object in cache, return that;
    // else request a copy of the object from the node which curates it.

    return result;
}

