/**
 *  payloads/cons.h
 *
 *  A cons cell.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <stdbool.h>

#include "memory/node.h"
#include "memory/pointer.h"
#include "memory/pso.h"
#include "memory/pso2.h"
#include "memory/pso4.h"
#include "memory/tags.h" 

#include "payloads/cons.h"
#include "payloads/exception.h"

#include "ops/string_ops.h"

/**
 * @brief allocate a cons cell with this car and this cdr, and return a pointer
 * to it.
 * 
 * @param car the pointer which should form the car of this cons cell;
 * @param cdr the pointer which should form the cdr of this cons cell.
 * @return struct pso_pointer a pointer to the newly allocated cons cell.
 */
struct pso_pointer cons( struct pso_pointer car, struct pso_pointer cdr ) {
    struct pso_pointer result = allocate( CONSTAG, 2 );

    struct pso2 *object = pointer_to_object( result );
    object->payload.cons.car = inc_ref( car );
    object->payload.cons.cdr = inc_ref( cdr );

    return result;
}


/**
 * @brief return the car of this cons cell.
 * 
 * @param cons a pointer to the cell.
 * @return the car of the indicated cell.
 * @exception if the pointer does not indicate a cons cell.
 */
struct pso_pointer car( struct pso_pointer cons ) {
    struct pso_pointer result = nil;
    struct pso2 *object = pointer_to_object( result );

    if ( consp( cons ) ) {
        result = object->payload.cons.car;
    }
    // TODO: else throw an exception

    return result;
}

/**
 * @brief return the cdr of this cons (or other sequence) cell.
 * 
 * @param cons a pointer to the cell.
 * @return the cdr of the indicated cell.
 * @exception if the pointer does not indicate a cons cell.
 */
struct pso_pointer cdr( struct pso_pointer p ) {
    struct pso_pointer result = nil;
    struct pso2 *object = pointer_to_object( result );

    switch (get_tag_value( p)) {
    case CONSTV : result = object->payload.cons.cdr; break;
    case KEYTV :
    case STRINGTV :
    case SYMBOLTV :
    	result = object->payload.string.cdr; break;
    default :
    	result = make_exception(
    			cons(c_string_to_lisp_string(L"Invalid type for cdr"), p),
				nil, nil);
    	break;
    }

    // TODO: else throw an exception

    return result;
}

/**
 * @brief When a cons cell is freed, its car and cdr pointers must be
 * decremented.
 *
 * Lisp calling conventions; one expected arg, the pointer to the cell to
 * be destroyed.
 */
struct pso_pointer destroy_cons( struct pso_pointer fp, struct pso_pointer env) {
	if (stackp(fp)) {
		struct pso4 *frame = pointer_to_pso4( fp);
		struct pso_pointer p = frame->payload.stack_frame.arg[0];
		dec_ref( car( p));
		dec_ref( cdr( p));
	}
}
