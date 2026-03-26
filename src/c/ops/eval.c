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

#include "memory/pointer"
#include "memory/stack.h"
#include "payloads/cons.h"
#include "payloads/function.h"
#include "payloads/keyword.h"
#include "payloads/lambda.h"
#include "payloads/nlambda.h"
#include "payloads/special.h"

/**
 * @brief Despatch eval based on tag of the form in the first position.
 * 
 * @param frame The current stack frame;
 * @param frame_pointer A pointer to the current stack frame;
 * @param env the evaluation environment.
 * @return struct pso_pointer 
 */
struct pso_pointer eval_despatch( struct stack_frame *frame, struct pso_pointer frame_pointer,
           struct pso_pointer env ) {
    struct pso_pointer result = frame->arg[0];

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

    return result;
}

struct pso_pointer lisp_eval( struct stack_frame *frame, struct pso_pointer frame_pointer,
           struct pso_pointer env ) {
    struct pso_pointer result = eval_despatch( frame, frame_pointer, env);

    if (exceptionp( result)) {
        // todo: if result doesn't have a stack frame, create a new exception wrapping
        // result with this stack frame.
    }

    return result;
}