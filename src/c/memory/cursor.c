/*
 * a cursor is a cons-space object which holds:
 * 1. a pointer to a vector (i.e. a vector-space object which holds an
 *    array of `cons_pointer`);
 * 2. an integer offset into that array.
 *
 * this provides a mechanism for iterating through vectors (actually, in
 * either direction)
 */
