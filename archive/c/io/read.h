/**
 * read.c
 *
 * First pass at a reader, for bootstrapping.
 *
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __read_h
#define __read_h

#include "memory/consspaceobject.h"

/* characters (other than arabic numberals) used in number representations */
#define LCOMMA L','
#define LPERIOD L'.'
#define LSLASH L'/'
/* ... used in map representations */
#define LCBRACE L'}'
/* ... used in path representations */
#define LSESSION L'§'

/**
 * read the next object on this input stream and return a cons_pointer to it.
 */
struct cons_pointer read( struct stack_frame *frame,
                          struct cons_pointer frame_pointer,
                          struct cons_pointer env, URL_FILE * input );

#endif
