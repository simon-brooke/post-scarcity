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

struct cons_pointer allocate( char* tag, uint8_t size_class) {
    struct cons_pointer result = nil;

    if (size_class <= MAX_SIZE_CLASS) {
        if ( not( freelists[size_class] ) ) {
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
            if ( object->header.count != 0) {
                // TODO: return an exception instead? Or warn, set it, and continue?
            }

        }
    } // TODO: else throw exception

    return result;
}

struct cons_pointer get_tag_value( struct cons_pointer pointer) {
    result = 
}