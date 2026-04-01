/**
 *  read.c
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

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/*
 * wide characters
 */
#include <wchar.h>
#include <wctype.h>

#include "debug.h"
#include "memory/node.h"
#include "memory/pointer.h"
#include "memory/pso2.h"

#include "io/io.h"
#include "io/read.h"

#include "payloads/integer.h"
#include "ops/stack_ops.h"


// TODO: what I've copied from 0.0.6 is *wierdly* over-complex for just now.
// I think I'm going to essentially delete all this and start again. We need
// to be able to despatch on readttables, and the initial readtable functions
// don't need to be written in Lisp.
//
// In the long run a readtable ought to be a hashtable, but for now an assoc
// list will do.
//
// A readtable function is a Lisp function so needs the stackframe and the
// environment. Other arguments (including the output stream) should be passed
// in the argument, so I think the first arg in the frame is the character read;
// the next is the input stream; the next is the readtable, if any.

/*
 * for the time being things which may be read are: 
 * * integers
 * * lists 
 * * atoms
 * * dotted pairs
 */

/**
 * An example wrapper function while I work out how I'm going to do this.
 */
struct pso_pointer read_example( struct pso4 *frame,
                            struct pso_pointer frame_pointer,
                            struct pso_pointer env) {
	struct pso_pointer character = fetch_arg( frame, 0);
	struct pso_pointer stream = fetch_arg( frame, 1);
	struct pso_pointer readtable = fetch_arg( frame, 2);

	return character;
}


/**
 * Read the next object on this input stream and return a pso_pointer to it.
 */
struct pso_pointer read( struct pso4 *frame,
						  struct pso_pointer frame_pointer,
                          struct pso_pointer env ) {
	struct pso_pointer* character = fetch_arg( frame, 0);
	struct pso_pointer stream = fetch_arg( frame, 1);
	struct pso_pointer readtable = fetch_arg( frame, 2);

	if (nilp(stream)) {

	}
}
