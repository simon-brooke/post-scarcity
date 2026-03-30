/**
 *  payloads/namespace.h
 *
 *  a Lisp namespace - a hashtable whose contents are mutable.
 *
 *  Can sensibly sit in any pso from size class 6 upwards. However, it's often
 *  considered a good thing to have a prime number of buckets in a hash table.
 *  Our total overhead on the full object size is two words header, and, for 
 *  namespaces, one word for the pointer to the (optional) hash function,  
 *  one for the number of buckets, one for the pointer to the write ACL, one
 *  for the pointer to the mutex, total six.
 *
 *  There are no really good fits until you get up to class 9, which might
 *  make sense for some namespaces, but it's quite large!
 *
 *  | size class | words | less overhead | nearest prime | wasted |
 *  | ---------- | ----- | ------------- | ------------- | ------ |
 *  | 5          | 32    | 26            | 23            | 3      |
 *  | 6          | 64    | 58            | 53            | 5      |
 *  | 7          | 128   | 122           | 113           | 9      |
 *  | 8          | 256   | 250           | 241           | 9      |
 *  | 9          | 512   | 506           | 503           | 3      |
 *  | 10         | 1024  | 1018          | 1013          | 5      |
 *
 *  Although it may be *better* to have prime numbers of buckets, how much
 *  better is it? Is a bucket with 23 slots sufficiently better than one
 *  with 26 slots to make up for its inevitably-longer hash buckets?
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_payloads_namespace_h
#define __psse_payloads_namespace_h

#include "memory/pointer.h"

/**
 * The payload of a namespace. The number of buckets is assigned at run-time,
 * and is stored in n_buckets. Each bucket is something ASSOC can consume:
 * i.e. either an assoc list or a further namespace.
 */
struct namespace_payload {
    struct pso_pointer hash_fn; /* function for hashing values in this namespace, or 
                                 * `NIL` to use the default hashing function */
    uint32_t n_buckets;         /* number of hash buckets */
    uint32_t unused;            /* for word alignment and possible later expansion */
    struct pso_pointer write_acl; /* it seems to me that it is likely that the
                                   * principal difference between a hashtable and a
                                   * namespace is that a hashtable has a write ACL
                                   * of `NIL`, meaning not writeable by anyone */
    struct pso_pointer mutex;   /* the mutex to lock when modifying this namespace. */
    struct pso_pointer buckets[]; /* actual hash buckets, which should be `NIL`
                                   * or assoc lists or (possibly) further hashtables. */
};

#endif
