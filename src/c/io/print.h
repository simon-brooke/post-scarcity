/**
 *  io/print.c
 *
 *  Post Scarcity Software Environment: print.
 *
 *  Print basic Lisp objects..This is :bootstrap layer print; it needs to be 
 *  able to print characters, symbols, integers, lists and dotted pairs. I 
 *  don't think it needs to be able to print anything else.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_io_print_h
#define __psse_io_print_h

struct pso_pointer print( struct pso_pointer p, struct pso_pointer stream );

#endif
