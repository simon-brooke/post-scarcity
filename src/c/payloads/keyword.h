/**
 *  payloads/keyword.h
 *
 *  A keyword cell.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_payloads_keyword_h
#define __psse_payloads_keyword_h

#include "memory/pointer.h"

/**
 * Tag for a keyword - an interned, self-evaluating string.
 */
#define KEYTAG	    "KEY"
#define KEYTV       5850443

/* TODO: for now, Keyword shares a payload with String, but this may change.
 * Strings are of indefinite length, but keywords are really not, and might
 * fit into any size class. */

#endif
