/**
 *  ops/reverse.c
 *
 *  Post Scarcity Software Environment: reverse.
 *
 *  Reverse a sequence. Didn'e want to do this in the substrate, but I need
 *  if for reading atoms!.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <stdbool.h>

#include "memory/node.h"
#include "memory/pointer.h"
#include "memory/pso.h"
#include "memory/pso2.h"
#include "memory/tags.h"

#include "payloads/cons.h"
#include "payloads/exception.h"
#include "payloads/psse_string.h"

#include "ops/string_ops.h"
#include "ops/truth.h"

/**
 * @brief reverse a sequence.
 *
 * A sequence is a list or a string-like-thing. A dotted pair is not a
 * sequence.
 *
 * @param sequence a pointer to a sequence.
 * @return a sequence like the `sequence` passed, but reversed; or `nil` if
 * 		the argument was not a sequence.
 */
struct pso_pointer reverse( struct pso_pointer sequence ) {
    struct pso_pointer result = nil;

    for ( struct pso_pointer cursor = sequence; !nilp( sequence );
          cursor = cdr( cursor ) ) {
        struct pso2 *object = pointer_to_object( cursor );
        switch ( get_tag_value( cursor ) ) {
            case CONSTV:
                result = cons( car( cursor ), result );
                break;
            case KEYTV:
                result =
                    make_string_like_thing( object->payload.string.character,
                                            result, KEYTAG );
                break;
            case STRINGTV:
                result =
                    make_string_like_thing( object->payload.string.character,
                                            result, STRINGTAG );
                break;
            case SYMBOLTV:
                result =
                    make_string_like_thing( object->payload.string.character,
                                            result, SYMBOLTAG );
                break;
            default:
                result =
                    make_exception( c_string_to_lisp_string
                                    ( L"Invalid object in sequence" ), nil,
                                    nil );
                goto exit;
                break;
        }
    }
  exit:

    return result;
}
