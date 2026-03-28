/**
 *  memory/memory.h
 *
 *  The memory management subsystem.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_memory_memory_h
#define __psse_memory_memory_h

/**
 * @brief Maximum size class
 * 
 * Size classes are poweres of 2, in words; so an object of size class 2 
 * has an allocation size of four words; of size class 3, of eight words, 
 * and so on. Size classes of 0 and 1 do not work for managed objects, 
 * since managed objects require a two word header; it's unlikely that
 * these undersized size classes will be used at all.
 */
#define MAX_SIZE_CLASS = 0xf

int initialise_memory(  );

extern struct pso_pointer out_of_memory_exception;
extern struct pso_pointer freelists[];
#endif
