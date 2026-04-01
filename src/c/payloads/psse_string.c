/**
 *  payloads/string.c
 *
 *  A string cell.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */


#include <stdint.h>

 /*
  * wide characters
  */
#include <wchar.h>
#include <wctype.h>

#include "memory/node.h"
#include "memory/pointer.h"
#include "memory/pso.h"
#include "memory/pso2.h"
#include "memory/pso4.h"
#include "memory/tags.h"

#include "ops/string_ops.h"
#include "ops/truth.h"

/**
 * @brief When an string is freed, its cdr pointer must be decremented.
 *
 * Lisp calling conventions; one expected arg, the pointer to the object to
 * be destroyed.
 */
struct pso_pointer destroy_string( struct pso_pointer fp,
                                   struct pso_pointer env ) {
    if ( stackp( fp ) ) {
        struct pso4 *frame = pointer_to_pso4( fp );
        struct pso_pointer p = frame->payload.stack_frame.arg[0];

        dec_ref( cdr( p ) );
    }

    return nil;
}
