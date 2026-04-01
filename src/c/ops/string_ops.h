/**
 *  ops/string_ops.h
 *
 *  Operations on a Lisp string.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_ops_string_ops_h
#define __psse_ops_string_ops_h

/*
 * wide characters
 */
#include <wchar.h>
#include <wctype.h>

struct pso_pointer make_string_like_thing( wint_t c, struct pso_pointer tail,
                                           char *tag );

struct pso_pointer make_string( wint_t c, struct pso_pointer tail );

struct pso_pointer make_keyword( wint_t c, struct pso_pointer tail );

struct pso_pointer make_symbol( wint_t c, struct pso_pointer tail );

struct pso_pointer c_string_to_lisp_string( wchar_t *string );

struct pso_pointer c_string_to_lisp_keyword( wchar_t *symbol );

#endif
