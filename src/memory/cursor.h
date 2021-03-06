/**
 *  cursor.h
 *
 *  Declarations common to all cons space objects.
 *
 *
 *  (c) 2017 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __cursor_h
#define __cursor_h

/**
 * A cursor cell: 1397904707
 */
#define CURSORTAG     "CURS"
#define CURSORTV      1397904707

/*
 * there are two ways the cursor payload could be defined. One is that it
 * could hold a `cons_pointer` that points to the canonical `VECP` cell
 * that points to the `vector_space_object`; the other is that it could
 * hold a `struct vector_space_object`

#endif
 
