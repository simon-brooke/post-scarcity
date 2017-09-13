/**
 * peano.c
 *
 * Basic peano arithmetic
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "consspaceobject.h"
#include "conspage.h"
#include "equal.h"
#include "integer.h"
#include "intern.h"
#include "lispops.h"
#include "print.h"
#include "read.h"
#include "stack.h"

/*
struct cons_pointer
lisp_plus( struct cons_pointer s_expr, struct cons_pointer env,
           struct stack_frame *frame ) {
    struct cons_space_object cell = pointer2cell( s_expr );
    struct cons_space_object result = NIL;
    
    
}
*/
