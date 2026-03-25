/**
 *  payloads/symbol.h
 *
 *  A symbol cell.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_payloads_symbol_h
#define __psse_payloads_symbol_h

#include "memory/pointer.h"


/**
 * Tag for a symbol: just like a keyword except not self-evaluating.
 */
#define SYMBOLTAG   "SYM"

/* TODO: for now, Symbol shares a payload with String, but this may change.
 * Strings are of indefinite length, but symbols are really not, and might
 * fit into any size class. */

#endif
