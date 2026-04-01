/**
 *  ops/eval.c
 *
 *  Post Scarcity Software Environment: eval.
 *
 *  Evaluate an arbitrary Lisp expression.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include "memory/pointer.h"
#include "memory/pso.h"
#include "memory/pso2.h"
#include "memory/pso3.h"
#include "memory/pso4.h"
#include "memory/tags.h"

#include "payloads/cons.h"
#include "payloads/exception.h"
#include "payloads/function.h"
#include "payloads/keyword.h"
#include "payloads/lambda.h"
#include "payloads/nlambda.h"
#include "payloads/special.h"
#include "payloads/stack.h"

#include "ops/truth.h"

/**
 * @brief Despatch eval based on tag of the form in the first position.
 * 
 * @param frame The current stack frame;
 * @param frame_pointer A pointer to the current stack frame;
 * @param env the evaluation environment.
 * @return struct pso_pointer 
 */
struct pso_pointer lisp_eval( struct pso_pointer frame_pointer,
                              struct pso_pointer env ) {
    struct pso4 *frame = pointer_to_pso4( frame_pointer );
    struct pso_pointer result = frame->payload.stack_frame.arg[0];

    // switch ( get_tag_value( result)) {
    //     case CONSTV:
    //         result = eval_cons( frame, frame_pointer, env);
    //         break;
    //     case KEYTV:
    //     case SYMBOLTV:
    //         result = eval_symbol( frame, frame_pointer, env);
    //         break;
    //     case LAMBDATV:
    //         result = eval_lambda( frame, frame_pointer, env);
    //         break;
    //     case NLAMBDATV:
    //         result = eval_nlambda( frame, frame_pointer, env);
    //         break;
    //     case SPECIALTV:
    //         result = eval_special( frame, frame_pointer, env);
    //         break;        
    // }

    if ( exceptionp( result ) ) {
        struct pso3 *x =
            ( struct pso3 * ) pointer_to_object_with_tag_value( result,
                                                                EXCEPTIONTV );

        if ( nilp( x->payload.exception.stack ) ) {
            inc_ref( result );
            result =
                make_exception( x->payload.exception.message, frame_pointer,
                                result );
        }
    }

    return result;
}
