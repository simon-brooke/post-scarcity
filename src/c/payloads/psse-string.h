/**
 *  payloads/string.h
 *
 *  A string cell.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_payloads_string_h
#define __psse_payloads_string_h
/*
 * wide characters
 */
#include <wchar.h>
#include <wctype.h>

#include "memory/pointer.h"

/**
 * @brief payload of a string cell. 
 *
 * At least at first, only one UTF character will be stored in each cell. At 
 * present the payload of a symbol or keyword cell is identical
 * to the payload of a string cell.
 */
struct string_payload {
    /** the actual character stored in this cell */
    wint_t character;
    /** a hash of the string value, computed at store time. */
    uint32_t hash;
    /** the remainder of the string following this character. */
    struct pso_pointer cdr;
};

#endif
