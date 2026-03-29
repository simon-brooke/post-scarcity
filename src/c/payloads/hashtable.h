/**
 *  payloads/hashtable.h
 *
 *  an ordinary Lisp hashtable - one whose contents are immutable.
 *
 *  Can sensibly sit in any pso from size class 6 upwards. However, it's often
 *  considered a good thing to have a prime number of buckets in a hash table.
 *  Our total overhead on the full object size is two words header, and, for 
 *  hashtables, one word for the pointer to the (optional) hash function, and 
 *  one for the number of buckets, total four. 
 *
 *  | size class | words | less overhead | nearest prime | wasted |
 *  | ---------- | ----- | ------------- | ------------- | ------ |
 *  | 5          | 32    | 28            | 23            | 5      |
 *  | 6          | 64    | 60            | 59            | 1      |
 *  | 7          | 128   | 124           | 113           | 11     |
 *  | 8          | 256   | 252           | 251           | 1      |
 *  | 9          | 512   | 508           | 503           | 5      |
 *  | 10         | 1024  | 1020          | 1019          | 1      |
 *
 *  So we can fit 59 buckets into a 64 word class 6 pso, wasting one word;
 *  251 buckets in a 256 word class 8 again wasting one word; 1019 in a size
 *  class 10, also wasting only one word. In a 32 word class 5, the best prime 
 *  we can do is 23 buckets, wasting five words.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_payloads_hashtable_h
#define __psse_payloads_hashtable_h

#include "memory/pointer.h"

/**
 * @brief Tag for an ordinary Lisp hashtable - one whose contents are immutable.
 * \see NAMESPACETAG for mutable hashtables.
 */
#define HASHTABLETAG "HTB"
#define HASHTABLETV  4346952

/**
 * The payload of a hashtable. The number of buckets is assigned at run-time,
 * and is stored in n_buckets. Each bucket is something ASSOC can consume:
 * i.e. either an assoc list or a further hashtable.
 */
struct hashtable_payload {
    struct pso_pointer hash_fn;    /* function for hashing values in this hashtable, or `NIL` to use
                                     the default hashing function */
    uint32_t n_buckets;             /* number of hash buckets */
    struct pso_pointer buckets[];  /* actual hash buckets, which should be `NIL`
                                     * or assoc lists or (possibly) further hashtables. */
};

#endif
