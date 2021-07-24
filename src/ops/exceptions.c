 /*
 * exceptions.c
 *
 * This is really, really unfinished and doesn't yet work. One of the really key 
 * things about exceptions is that the stack frames between the throw and the 
 * catch should not be derefed, so eval/apply will need to be substantially 
 * re-written.
 *
 * (c) 2021 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "consspaceobject.h"
#include "conspage.h"
#include "debug.h"
#include "dump.h"
#include "equal.h"
#include "integer.h"
#include "intern.h"
#include "io.h"
#include "lispops.h"
#include "map.h"
#include "print.h"
#include "read.h"
#include "stack.h"
#include "vectorspace.h"


/**
 * OK, the idea here (and I know this is less than perfect) is that the basic `try` 
 * function in PSSE takes two arguments, the first, `body`, being a list of forms, 
 * and the second, `catch`, being a catch handler (which is also a list of forms). 
 * Forms from `body` are evaluated in turn until one returns an exception object, 
 * or until the list is exhausted. If the list was exhausted, then the value of
 * evaluating the last form in `body` is returned. If an exception was encountered, 
 * then each of the forms in `catch` is evaluated and the value of the last of 
 * those is returned.
 * 
 * This is experimental. It almost certainly WILL change.
 */
struct cons_pointer lisp_try(struct stack_frame *frame,
                             struct cons_pointer frame_pointer,
                             struct cons_pointer env)
{
    struct cons_pointer result = c_progn(frame, frame_pointer, frame->arg[0], env);

    if (loopexitp(result))
    {
        // TODO: need to put the exception into the environment!
        result = c_progn(frame, frame_pointer, frame->arg[1], env);
    }

    return result;
}


