/**
 *  memory/page.c
 *
 *  Page for paged space psoects.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "debug.h"

#include "memory/memory.h"
#include "memory/node.h"
#include "memory/page.h"
#include "memory/pso.h"
#include "memory/pso2.h"
#include "memory/pso3.h"
#include "memory/pso4.h"
#include "memory/pso5.h"
#include "memory/pso6.h"
#include "memory/pso7.h"
#include "memory/pso8.h"
#include "memory/pso9.h"
#include "memory/psoa.h"
#include "memory/psob.h"
#include "memory/psoc.h"
#include "memory/psod.h"
#include "memory/psoe.h"
#include "memory/psof.h"
#include "memory/tags.h"

#include "payloads/free.h"

/**
 * @brief The pages which have so far been initialised.
 * 
 * TODO: This is temporary. We cannot afford to allocate an array big enough
 * to hold the number of pages we *might* create at start up time. We need a
 * way to grow the number of pages, while keeping access to them cheap.
 */
union page *pages[NPAGES];

/**
 * @brief the number of pages which have thus far been allocated.
 * 
 */
uint32_t npages_allocated = 0;

/**
 * @brief private to allocate_page; do not use.
 * 
 * @param page_addr address of the newly allocated page to be initialised;
 * @param page_index its location in the pages[] array;
 * @param size_class the size class of objects in this page;
 * @param freelist the freelist for objects of this size class.
 * @return struct pso_pointer the new head for the freelist for this size_class,
 */
struct pso_pointer initialise_page( union page *page_addr, uint16_t page_index,
                                    uint8_t size_class,
                                    struct pso_pointer freelist ) {
    struct pso_pointer result = freelist;
    int obj_size = pow( 2, size_class );
    int obj_bytes = obj_size * sizeof( uint64_t );
    int objs_in_page = PAGE_BYTES / obj_bytes;

    // we do this backwards (i--) so that object {0, 0, 0} will be first on the
    // freelist when the first page is initiated, so we can grab that one for 
    // `nil` and the next on for `t`.
    for ( int i = objs_in_page - 1; i >= 0; i-- ) {
        // it should be safe to cast any pso object to a pso2
        struct pso2 *object =
            ( struct pso2 * ) ( page_addr + ( i * obj_bytes ) );

        object->header.tag.bytes.size_class = size_class;
        strncpy( &( object->header.tag.bytes.mnemonic[0] ), FREETAG,
                 TAGLENGTH );
        object->payload.free.next = result;

        result =
            make_pointer( node_index, page_index,
                          ( uint16_t ) ( i * obj_size ) );
    }

    return result;
}

/** 
 * @brief Allocate a page for objects of this size class, initialise it, and
 * link the objects in it into the freelist for this size class.
 *
 * @param size_class an integer in the range 0...MAX_SIZE_CLASS.
 * @return t on success, an exception if an error occurred.
 */
struct pso_pointer allocate_page( uint8_t size_class ) {
    struct pso_pointer result = t;

    if ( npages_allocated == 0 ) {
        for ( int i = 0; i < NPAGES; i++ ) {
            pages[i] = NULL;
        }
        debug_print( L"Pages array zeroed.\n", DEBUG_ALLOC, 0 );
    }

    if ( npages_allocated < NPAGES ) {
        if ( size_class >= 2 && size_class <= MAX_SIZE_CLASS ) {
            void *pg = malloc( sizeof( union page ) );

            if ( pg != NULL ) {
                memset( pg, 0, sizeof( union page ) );
                pages[npages_allocated] = pg;
                debug_printf( DEBUG_ALLOC, 0,
                              L"Allocated page %d for objects of size class %x.\n",
                              npages_allocated, size_class );

                freelists[size_class] =
                    initialise_page( ( union page * ) pg, npages_allocated,
                                     size_class, freelists[size_class] );

                debug_printf( DEBUG_ALLOC, 0,
                              L"Initialised page %d; freelist for size class %x updated.\n",
                              npages_allocated, size_class );

                if (npages_allocated == 0) {
                	// first page allocated; initialise nil and t
                	nil = lock_object( allocate(NILTAG, 2));
                	t = lock_object( allocate(TRUETAG, 2));
                }

                npages_allocated++;
            } else {
                // TODO: exception when we have one.
                result = nil;
                fwide( stderr, 1 );
                fwprintf( stderr,
                          L"\nCannot allocate page: heap exhausted,\n",
                          size_class, MAX_SIZE_CLASS );
            }
        } else {
            // TODO: exception when we have one.
            result = nil;
            fwide( stderr, 1 );
            fwprintf( stderr,
                      L"\nCannot allocate page for size class %x, min is 2 max is %x.\n",
                      size_class, MAX_SIZE_CLASS );
        }
    } else {
        // TODO: exception when we have one.
        result = nil;
        fwide( stderr, 1 );
        fwprintf( stderr,
                  L"\nCannot allocate page: page space exhausted.\n",
                  size_class, MAX_SIZE_CLASS );
    }

    return result;
}

/**
 * @brief allow other files to see the current value of npages_allocated, but not
 * change it.
 */
uint32_t get_pages_allocated() {
	return npages_allocated;
}
