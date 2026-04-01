/**
 *  read.h
 *
 *  Read basic Lisp objects..This is :bootstrap layer print; it needs to be
 *  able to read characters, symbols, integers, lists and dotted pairs. I
 *  don't think it needs to be able to read anything else. It must, however,
 *  take a readtable as argument and expand reader macros.
 *
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_io_read_h
#define __psse_io_read_h
struct pso_pointer read_number( struct pso_pointer frame_pointer,
                                struct pso_pointer env );

struct pso_pointer read_symbol( struct pso_pointer frame_pointer,
                                struct pso_pointer env );

struct pso_pointer read( struct pso_pointer frame_pointer,
                         struct pso_pointer env );

#endif
