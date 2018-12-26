/**
 *  dump.h
 *
 *  Dump representations of both cons space and vector space objects.
 *
 *  (c) 2018 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
/*
 * wide characters
 */
#include <wchar.h>
#include <wctype.h>

#ifndef __dump_h
#define __dump_h


/**
 * dump the object at this cons_pointer to this output stream.
 */
void dump_object( FILE * output, struct cons_pointer pointer );


#endif
