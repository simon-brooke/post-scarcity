/**
 *  payloads/exception.h
 *
 *  An exception; required three pointers, so use object of size class 3.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_payloads_exception_h
#define __psse_payloads_exception_h
#include <stdbool.h>

#include "memory/pointer.h"

#define EXCEPTIONTAG "EXP"
#define EXCEPTIONTV  5265477

/**
 * @brief An exception; required three pointers, so use object of size class 3.
 */
struct exception_payload {
  /** @brief the exception message. Expected to be a string, but may be anything printable. */
    struct pso_pointer message;
  /** @brief the stack frame at which the exception was thrown. */
    struct pso_pointer stack;
    /** @brief the cause; expected to be another exception, or (usually) `nil`. */
    struct pso_pointer cause;
};

bool exceptionp( struct pso_pointer p );

#endif
