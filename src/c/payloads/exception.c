/**
 *  payloads/exception.c
 *
 *  An exception; required three pointers, so use object of size class 3.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */


#include "memory/pointer.h"
#include "memory/pso.h"
#include "payloads/exception.h"

/**
 * @param p a pointer to an object.
 * @return true if that object is an exception, else false.
 */
bool exceptionp( struct pso_pointer p ) {
    return ( get_tag_value( p ) == EXCEPTIONTV );
}
