/*
 * bignum.c
 *
 * Allocation of and operations on arbitrary precision integers.
 *
 * (c) 2018 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

/*
 * Bignums generally follow Knuth, vol 2, 4.3. The word size is 64 bits,
 * and words are stored in individual cons-space objects, comprising the
 * word itself and a pointer to the next word in the number.
 */
