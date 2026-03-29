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
#include "payloads/cons.h"

/**
 * @brief allocate a cons cell with this car and this cdr, and return a pointer
 * to it.
 * 
 * @param car the pointer which should form the car of this cons cell;
 * @param cdr the pointer which should form the cdr of this cons cell.
 * @return struct pso_pointer a pointer to the newly allocated cons cell.
 */
struct pso_pointer cons( struct pso_pointer car, struct pso_pointer cdr) {
    struct pso_pointer result = allocate( CONSTAG, 2);

    struct pso2 *object = pointer_to_object( result );
    object->payload.cons.car = car;
    object->payload.cons.cdr = cdr;

    inc_ref( car);
    inc_ref( cdr);

    return result;
}

/**
 * @brief return true if `ptr` indicates a cons cell, else false.
 * 
 * @param ptr a pointer.
 * @return true if `ptr` indicates a cons cell.
 * @return false otherwise
 */
bool consp( struct pso_pointer ptr) {
    // TODO: make it actually work!
    return false;
}

/**
 * @brief return the car of this cons cell.
 * 
 * @param cons a pointer to the cell.
 * @return the car of the indicated cell.
 * @exception if the pointer does not indicate a cons cell.
 */
struct pso_pointer car( struct pso_pointer cons) {
    struct pso_pointer result = nil;
    struct pso2 *object = pointer_to_object( result );

    if ( consp( cons)) {
        result = object->payload.cons.car;
    }
    // TODO: else throw an exception

    return result;
}

/**
 * @brief return the cdr of this cons cell.
 * 
 * @param cons a pointer to the cell.
 * @return the cdr of the indicated cell.
 * @exception if the pointer does not indicate a cons cell.
 */
struct pso_pointer cdr( struct pso_pointer cons) {
    struct pso_pointer result = nil;
    struct pso2 *object = pointer_to_object( result );

    if ( consp( cons)) {
        result = object->payload.cons.cdr;
    }
    // TODO: else throw an exception

    return result;
}