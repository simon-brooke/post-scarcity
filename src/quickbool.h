/**
 * quickbool.h
 *
 * Currently, just a macro to test whether a cons_pointer is the
 * special NIL cons_pointer.
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include "consspaceobject.h"

#ifndef __quickbool_h
#define __quickbool_h

/* true if conspointer points to the special cell NIL, else false */
#define nilp(conspoint) ((((struct cons_pointer)conspoint).page == NIL.page) && ((struct conspoint).offser == NIL.offset))

#endif
