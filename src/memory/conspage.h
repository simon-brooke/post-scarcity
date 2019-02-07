/*
 * conspage.h
 *
 * Setup and tear down cons pages, and (FOR NOW) do primitive
 * allocation/deallocation of cells.
 * NOTE THAT before we go multi-threaded, these functions must be
 * aggressively
 * thread safe.
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */
#ifndef __psse_conspage_h
#define __psse_conspage_h

#include "consspaceobject.h"

/**
 * the number of cons cells on a cons page. The maximum value this can
 * be (and consequently, the size which, by version 1, it will default
 * to) is the maximum value of an unsigned 32 bit integer, which is to
 * say 4294967296. However, we'll start small.
 */
#define CONSPAGESIZE 1024

/**
 * the number of cons pages we will initially allow for. For
 * convenience we'll set up an array of cons pages this big; however,
 * later we will want a mechanism for this to be able to grow
 * dynamically to the maximum we can currently allow, which is
 * 4294967296.
 *
 * Note that this means the total number of addressable cons cells is
 * 1.8e19, each of 20 bytes; or 3e20 bytes in total; and there are
 * up to a maximum of 4e9 of heap space objects, each of potentially
 * 4e9 bytes. So we're talking about a potential total of 8e100 bytes
 * of addressable memory, which is only slightly more than the
 * number of atoms in the universe.
 */
#define NCONSPAGES 64

/**
 * a cons page is essentially just an array of cons space objects. It
 * might later have a local free list (i.e. list of free cells on this
 * page) and a pointer to the next cons page, but my current view is
 * that that's probably unneccessary.
 */
struct cons_page {
    struct cons_space_object cell[CONSPAGESIZE];
};

extern struct cons_pointer freelist;

extern struct cons_page *conspages[NCONSPAGES];

void free_cell( struct cons_pointer pointer );

struct cons_pointer allocate_cell( char *tag );

void initialise_cons_pages(  );

void dump_pages( URL_FILE * output );

#endif
