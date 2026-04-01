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
struct pso_pointer make_pointer( uint32_t node, uint16_t page,
                                 uint16_t offset ) {
    return ( struct pso_pointer ) { node, page, offset };
}

/**
 * @brief returns the in-memory address of the object indicated by this 
 * pointer `p`.
 *
 * NOTE THAT: It's impossible, with our calling conventions, to pass an
 * exception back from this function. Consequently, if anything goes wrong
 * we return NULL. The caller *should* check for that and throw an exception.
 *
 * NOTE THAT: The return signature of these functions is pso2, because it is
 * safe to cast any paged space object to a pso2, but safe to cast an object
 * of a smaller size class to a larger one. If you know what size class you
 * want, you should prefer `pointer_to_object_of_size_class()`, q.v.
 *
 * TODO: The reason I'm doing it this way is because I'm not
 * certain reference counter updates work right it we work with 'the object'
 * rather than 'the address of the object'. I really ought to have a 
 * conversation with someone who understands this bloody language.
 * 
 * @param p a pso_pointer which references an object.
 *
 * @return the actual address in memory of that object, or NULL if `p` is
 * invalid.
 */
struct pso2 *pointer_to_object( struct pso_pointer p ) {
    struct pso2 *result = NULL;

    if ( p.node == node_index ) {
    	if (p.page < get_pages_allocated() && p.offset < (PAGE_BYTES / 8)) {
    		// TODO: that's not really a safe test of whether this is a valid pointer.
			union page *pg = pages[p.page];
			result = ( struct pso2 * ) &pg->words[p.offset];
    	}
    }
    // TODO: else if we have a copy of the object in cache, return that;
    // else request a copy of the object from the node which curates it.

    return result;
}

/**
 * @brief returns the memory address of the object indicated by this pointer
 * `p`, if it is of this `size_class`.
 *
 * NOTE THAT: It's impossible, with our calling conventions, to pass an
 * exception back from this function. Consequently, if anything goes wrong
 * we return NULL. The caller *should* check for that and throw an exception.
 *
 * NOTE THAT: The return signature of these functions is pso2, because it is
 * safe to cast any paged space object to a pso2, but safe to cast an object
 * of a smaller size class to a larger one. You should check that the object
 * returned has the size class you expect.
 *
 * @param p a pointer to an object;
 * @param size_class a size class.
 *
 * @return the memory address of the object, provided it is a valid object and
 * 		of the specified size class, else NULL.
 */
struct pso2 * pointer_to_object_of_size_class( struct pso_pointer p, uint8_t size_class) {
	struct pso2 * result = pointer_to_object( p);

	if (result->header.tag.bytes.size_class != size_class) {
		result = NULL;
	}

	return result;
}

/**
 * @brief returns the memory address of the object indicated by this pointer
 * `p`, if it has this `tag_value`.
 *
 * NOTE THAT: It's impossible, with our calling conventions, to pass an
 * exception back from this function. Consequently, if anything goes wrong
 * we return NULL. The caller *should* check for that and throw an exception.
 */
struct pso2 * pointer_to_object_with_tag_value( struct pso_pointer p, uint32_t tag_value) {
	struct pso2 * result = pointer_to_object( p);

	if ((result->header.tag.value & 0xffffff) != tag_value) {
		result = NULL;
	}

	return result;
}

