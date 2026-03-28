/**
 *  payloads/namespace.h
 *
 *  a Lisp namespace - a hashtable whose contents are mutable.
 *
 *  Can sensibly sit in any pso from size class 6 upwards.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_payloads_namespace_h
#define __psse_payloads_namespace_h

#include "memory/pointer.h"

/**
 * @brief Tag for a Lisp namespace - a hashtable whose contents are mutable.
 * \see HASHTABLETAG for mutable hashtables.
 */
#define NAMESPACETAG "NSP"

/**
 * The payload of a namespace. The number of buckets is assigned at run-time,
 * and is stored in n_buckets. Each bucket is something ASSOC can consume:
 * i.e. either an assoc list or a further namespace.
 */
struct namespace_payload {
    struct cons_pointer hash_fn;    /* function for hashing values in this namespace, or 
                                     * `NIL` to use the default hashing function */
    struct cons_pointer write_acl;  /* it seems to me that it is likely that the
                                     * principal difference between a hashtable and a
                                     * namespace is that a hashtable has a write ACL
                                     * of `NIL`, meaning not writeable by anyone */
    struct cons_pointer mutex;      /* the mutex to lock when modifying this namespace.*/
    uint32_t n_buckets;             /* number of hash buckets */
    uint32_t unused;                /* for word alignment and possible later expansion */
    struct cons_pointer buckets[];  /* actual hash buckets, which should be `NIL`
                                     * or assoc lists or (possibly) further hashtables. */
};

#endif