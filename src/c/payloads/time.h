/**
 *  payloads/cons.h
 *
 *  A cons cell.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_payloads_cons_h
#define __psse_payloads_cons_h

#include <stdint.h>

#include "memory/pointer.h"

/**
 * @brief Tag for a time stamp.
 */
#define TIMETAG     "TIM"
#define TIMETV      5065044

/**
 * The payload of a time cell: an unsigned 128 bit value representing micro-
 * seconds since the estimated date of the Big Bang (actually, for
 * convenience, 14Bn years before 1st Jan 1970 (the UNIX epoch))
 */
struct time_payload {
    unsigned __int128_t value;
};

#endif
