#include "consspaceobject.h"

#ifndef __conspage_h
#define __conspage_h


/**
 * the number of cons cells on a cons page. The maximum value this can be (and consequently,
 * the size which, by version 1, it will default to) is the maximum value of an unsigned 32
 * bit integer, which is to say 4294967296. However, we'll start small.
 */
#define CONSPAGESIZE 256

/**
 * the number of cons pages we will initially allow for. For convenience we'll set up an array
 * of cons pages this big; however, later we will want a mechanism for this to be able to grow
 * dynamically to the maximum we can currently allow, which is 4294967296.
 */
#define NCONSPAGES 256

/**
 * a cons page is essentially just an array of cons space objects. It might later have a local
 * free list (i.e. list of free cells on this page) and a pointer to the next cons page, but
 * my current view is that that's probably unneccessary.
 */
struct cons_page {
  struct cons_space_object cell[CONSPAGESIZE];
};


/**
 * initialise the cons page system; to be called exactly once during startup.
 */
void conspagesinit();

#endif
