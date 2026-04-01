/**
 *  memory/pointer.h
 *
 *  A pointer to a paged space object.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_memory_pointer_h
#define __psse_memory_pointer_h

#include <stdint.h>

/**
 * @brief A pointer to an object in page space.
 * 
 */
struct pso_pointer {
    /**
     * @brief The index of the node on which this object is curated.
     * 
     * NOTE: This will always be NULL until we have the hypercube router
     * working.
     */
    uint32_t node;
    /**
     * @brief The index of the allocated page in which this object is stored.
     */
    uint16_t page;
    /**
     * @brief The offset of the object within the page **in words**.
     *
     * NOTE THAT: This value is always **in words**, regardless of the size
     * class of the objects stored in the page, because until we've got hold
     * of the page we don't know its size class. 
     */
    uint16_t offset;
};


struct pso_pointer make_pointer( uint32_t node, uint16_t page, uint16_t offset);

struct pso2* pointer_to_object( struct pso_pointer pointer);

struct pso2 * pointer_to_object_of_size_class( struct pso_pointer p, uint8_t size_class);

struct pso2 * pointer_to_object_with_tag_value( struct pso_pointer p, uint32_t tag_value);

#endif
