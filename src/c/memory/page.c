/**
 *  memory/page.c
 *
 *  Page for paged space psoects.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <math.h>
#include <stdint.h>

#include "memory/memory.h"
#include "memory/node.h"
#include "memory/page.h"
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
#include "payloads/free.h"

/**
 * @brief The pages which have so far been initialised.
 * 
 * TODO: This is temporary. We cannot afford to allocate an array big enough
 * to hold the number of pages we *might* create at start up time. We need a
 * way to grow the number of pages, while keeping access to them cheap.
 */
struct page * pages[NPAGES];

/**
 * @brief the number of pages which have thus far been allocated.
 * 
 */
uint32_t npages_allocated = 0

/**
 * @brief private to allocate_page; do not use.
 * 
 * @param page_addr address of the newly allocated page to be initialised;
 * @param page_index its location in the pages[] array;
 * @param size_class the size class of objects in this page;
 * @param freelist the freelist for objects of this size class.
 * @return struct cons_pointer the new head for the freelist for this size_class,
 */
struct cons_pointer initialise_page( struct page * page_addr, uint16_t page_index, uint8_t size_class, pso_pointer freelist) {
    struct cons_pointer result = freelist;
    int obj_size = pow(2, size_class);
    int obj_bytes = obj_size  * sizeof(uint64_t);
    int objs_in_page = PAGE_BYTES/obj_bytes;

    // we do this backwards (i--) so that object {0, 0, 0} will be first on the
    // freelist when the first page is initiated, so we can grab that one for 
    // `nil` and the next on for `t`.
    for (int i = objs_in_page - 1; i >= 0; i--) {
        // it should be safe to cast any pso object to a pso2
        struct pso2* object = (pso2 *)(page_addr + (i * obj_bytes)); 

        object->header.tag.size_class = size_class;
        strncpy( (char *)(object->header.tag.mnemonic), FREETAG, TAGLENGTH);
        object->payload.free.next = result;

        result = make_pointer( node_index, page_index, (uint16_t)( i * obj_size));
    }

    return result;
}

/** 
 * @brief Allocate a page for objects of this size class, initialise it, and
 * link the objects in it into the freelist for this size class.
 *
 * Because we can't return an exception at this low level, and because there 
 * are multiple possible causes of failure, for the present this function will
 * print errors to stderr. We cast the error stream to wide, since we've 
 * probably (but not certainly) already cast it to wide, and we can't reliably
 * cast it back.
 *
 * @param size_class an integer in the range 0...MAX_SIZE_CLASS.
 * @return t on success, an exception if an error occurred.
 */
struct cons_pointer allocate_page( uint8_t size_class ) {
    struct cons_pointer result = t;

    if ( npages_allocated == 0) {
        for (int i = 0; i < NPAGES; i++) {
            pages[i] = NULL;
        }
        debug_print( L"Pages array zeroed.\n", DEBUG_ALLOC, 0);
    }

    if ( npages_allocated < NPAGES) {    
        if ( size_class >= 2 && size_class <= MAX_SIZE_CLASS ) {
            result = malloc( sizeof( page ) );

            if ( result != NULL ) {
                memset( result, 0, sizeof( page ) );
                pages[ npages_allocated] = result;
                debug_printf( DEBUG_ALLOC, 0, 
                    L"Allocated page %d for objects of size class %x.\n", 
                    npages_allocated, size_class);

                freelists[size_class] =
                    initialise_page( result, npages_allocated, size_class, freelists[size_class] );

                debug_printf( DEBUG_ALLOC, 0, 
                    L"Initialised page %d; freelist for size class %x updated.\n", 
                    npages_allocated,
                    size_class);    

                npages_allocated ++;
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
