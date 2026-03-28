/**
 *  memory/pso.c
 *
 *  Paged space objects.
 *
 *  Broadly, it should be save to cast any paged space object to a pso2, since
 *  that is the smallest actually used size class. This should work to extract 
 *  the tag and size class fields from the header, for example. I'm not 
 *  confident enough of my understanding of C to know whether it is similarly 
 *  safe to cast something passed to you as a pso2 up to something larger, even
 *  if you know from the size class field that it actually is something larger.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

 #include "memory/page.h"
 #include "memory/pointer.h"
 #include "memory/pso.h"

 /**
  * @brief Allocate an object of this size_class with this tag.
  * 
  * @param tag The tag. Only the first three bytes will be used;
  * @param size_class The size class for the object to be allocated;
  * @return struct pso_pointer a pointer to the newly allocated object
  */
struct pso_pointer allocate( char* tag, uint8_t size_class) {
    struct pso_pointer result = nil;

    if (size_class <= MAX_SIZE_CLASS) {
        if (freelists[size_class] == nil) {
            result = allocate_page(size_class)
        }

        if ( !exceptionp( result) && not( freelists[size_class] ) ) {
            result = freelists[size_class];
            struct pso2* object = pointer_to_object( result);
            freelists[size_class] = object->payload.free.next;

            strncpy( (char *)(object->header.tag.mnemonic), tag, TAGLENGTH);

            /* the object ought already to have the right size class in its tag
             * because it was popped off the freelist for that size class. */
            if ( object->header.tag.size_class != size_class) {
                // TODO: return an exception instead? Or warn, set it, and continue?
            }
            /* the objext ought to have a reference count ot zero, because it's 
             * on the freelist, but again we should sanity check. */
            if ( object->header.header.count != 0) {
                // TODO: return an exception instead? Or warn, set it, and continue?
            }

        }
    } // TODO: else throw exception

    return result;
}

/**
 * increment the reference count of the object at this cons pointer.
 *
 * You can't roll over the reference count. Once it hits the maximum
 * value you cannot increment further.
 *
 * Returns the `pointer`.
 */
struct pso_pointer inc_ref( struct pso_pointer pointer ) {
    struct pso2 *object = pointer_to_object( pointer );

    if ( object->header.count < MAXREFERENCE ) {
        object->header.count++;
#ifdef DEBUG
        debug_printf( DEBUG_ALLOC,
                      L"\nIncremented object of type %4.4s at page %u, offset %u to count %u",
                      ( ( char * ) object->header.tag.bytes ), pointer.page,
                      pointer.offset, object->header.count );
        if ( strncmp( object->header.tag.bytes, VECTORPOINTTAG, TAGLENGTH ) == 0 ) {
            debug_printf( DEBUG_ALLOC,
                          L"; pointer to vector object of type %4.4s.\n",
                          ( ( char * ) ( object->header.payload.vectorp.tag.bytes ) ) );
        } else {
            debug_println( DEBUG_ALLOC );
        }
#endif
    }

    return pointer;
}

/**
 * Decrement the reference count of the object at this cons pointer.
 *
 * If a count has reached MAXREFERENCE it cannot be decremented.
 * If a count is decremented to zero the object should be freed.
 *
 * Returns the `pointer`, or, if the object has been freed, a pointer to `nil`.
 */
struct pso_pointer dec_ref( struct pso_pointer pointer ) {
    struct pso2 *object = pointer_to_object( pointer );

    if ( object->count > 0 && object->count != MAXREFERENCE ) {
        object->count--;
#ifdef DEBUG
        debug_printf( DEBUG_ALLOC,
                      L"\nDecremented object of type %4.4s at page %d, offset %d to count %d",
                      ( ( char * ) object->tag.bytes ), pointer.page,
                      pointer.offset, object->count );
        if ( strncmp( ( char * ) object->tag.bytes, VECTORPOINTTAG, TAGLENGTH )
             == 0 ) {
            debug_printf( DEBUG_ALLOC,
                          L"; pointer to vector object of type %4.4s.\n",
                          ( ( char * ) ( object->payload.vectorp.tag.bytes ) ) );
        } else {
            debug_println( DEBUG_ALLOC );
        }
#endif

        if ( object->header.count == 0 ) {
            free_cell( pointer );
            pointer = NIL;
        }
    }

    return pointer;
}

/**
 * @brief Prevent an object ever being dereferenced.
 * 
 * @param pointer pointer to an object to lock.
 */
void lock_object( struct pso_pointer pointer) {
    struct pso2* object = pointer_to_object( pointer );

    object->header.header.count = MAXREFERENCE;
}


/**
 * @brief Get the numeric value of the tag bytes of the object indicated
 * by this pointer
 * 
 * @param pointer a pointer to an object.
 * @return the tag value of the object indicated.
 */
uint32_t get_tag_value( struct pso_pointer pointer) {
    result = (pointer_to_object( pointer)->tag.value & 0xffffff;

    // TODO: deal with the vector pointer issue

    return result;
}