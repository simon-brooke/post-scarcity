/*
 * loop.c
 *
 * Iteration functions. This has *a lot* of similarity to try/catch -- 
 * essentially what `recur` does is throw a special purpose exception which is
 * caught by `loop`.
 * 
 * Essentially the syntax I want is 
 * 
 *  (defun expt (n e)
 *      (loop ((n1 . n) (r . n) (e1 . e))
 *          (cond ((= e 0) r)
 *              (t (recur n1 (* n1 r) (- e 1)))))
 * 
 * It might in future be good to allow the body of the loop to comprise many
 * expressions, like a `progn`, but for now if you want that you can just
 * shove a `progn` in. Note that, given that what `recur` is essentially
 * doing is throwing a special purpose exception, the `recur` expression
 * doesn't actually have to be in the same function as the `loop` expression.
 *
 * (c) 2021 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include "consspaceobject.h"
#include "lispops.h"
#include "loop.h"

/**
 * Special form, not dissimilar to `let`. Essentially,
 * 
 * 1. the first arg (`args`) is an assoc list;
 * 2. the second arg (`body`) is an expression.
 * 
 * Each of the vals in the assoc list is evaluated, and bound to its 
 * respective key in a new environment. The body is then evaled in that
 * environment. If the result is an object of type LOOP, it should carry 
 * a list of values of the same arity as args. Each of the keys in args
 * is then rebound in a new environment to the respective value from the 
 * LOOP object, and body is then re-evaled in that environment.
 * 
 * If the result is not a LOOP object, it is simply returned.
 */
struct cons_pointer
lisp_loop( struct stack_frame *frame, struct cons_pointer frame_pointer,
           struct cons_pointer env ) {
    struct cons_pointer keys = c_keys( frame->arg[0] );
    struct cons_pointer body = frame->arg[1];

}
