/**
 *  memory/node.h
 *
 *  Top level data about the actual node on which this memory system sits.
 *  May not belong in `memory`. 
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_memory_node_h
#define __psse_memory_node_h


/**
 * @brief The index of this node in the hypercube.
 * 
 */
extern int node_index;

/**
 * @brief The canonical `nil` pointer
 * 
 */
extern struct pso_pointer nil;

/**
 * @brief the canonical `t` (true) pointer.
 * 
 */
extern struct pso_pointer t;

#endif
