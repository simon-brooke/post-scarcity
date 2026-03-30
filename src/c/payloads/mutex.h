/**
 *  payloads/mutex.h
 *
 *  A mutex (mutual exclusion lock) cell. Requires a size class 3 object.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_payloads_mutex_h
#define __psse_payloads_mutex_h

#include <pthread.h>

#include "memory/pointer.h"

/**
 * @brief payload for mutex objects.
 *
 * NOTE that the size of `pthread_mutex_t` is variable dependent on hardware 
 * architecture, but the largest known size is 40 bytes (five words).
 */
struct mutex_payload {
    pthread_mutex_t mutex;
};

struct pso_pointer make_mutex(  );

/**
 * @brief evaluates these forms within the context of a thread-safe lock.
 *
 * 1. wait until the specified mutex can be locked;
 * 2. evaluate each of the forms sequentially in the context of that locked 
 *    mutex;
 * 3. if evaluation of any of the forms results in the throwing of an 
 *    exception, catch the exception, unlock the mutex, and then re-throw the
 *    exception;
 * 4. on successful completion of the evaluation of the forms, unlock the mutex
 *    and return the value of the last form.
 * 
 * @param lock the lock: a mutex (MTX) object;
 * @param forms a list of arbitrary Lisp forms.
 * @return struct pso_pointer the result.
 */
struct pso_pointer with_lock( struct pso_pointer lock,
                              struct pso_pointer forms );

/**
 * @brief as with_lock, q.v. but attempts to obtain a lock and returns an
 * exception on failure
 *
 * 1. attempt to lock the specified mutex;
 * 2. if successful, proceed as `with_lock`;
 * 3. otherwise, return a specific exception which can be trapped for.
 * 
 * @param lock the lock: a mutex (MTX) object;
 * @param forms a list of arbitrary Lisp forms.
 * @return struct pso_pointer the result.
 */
struct pso_pointer attempt_with_lock( struct pso_pointer lock,
                                      struct pso_pointer forms );

#endif
