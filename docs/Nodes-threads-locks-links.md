# Nodes, threads, locks and links

## The problem

Up to now, I've been building a single threaded Lisp. I haven't had to worry about who is mutating memory while I'm trying to read it. The idea that this is a mostly immutable Lisp has encouraged me to be blas&eacute; about this. But actually, it isn't entirely immutable, and that matters.

Whenever *any* new datum is created, the freelist pointers have to mutate; whenever any new value is written to any namespace, the namespace has to mutate. The freelist pointers also mutate when objects are allocated and when objects are freed.

Earlier in the design, I had the idea that in the hypercube system, each node would have a two core processor, one core doing execution &mdash; actually evaluating Lisp functions &mdash; the other handling inter-node communication. I had at one stage the idea that the memory on the node would be partitioned into fixed areas:

| Partition | Contents | Core written by |
| --------- | -------- | --------------- |
| Local cons space | Small objects curated locally | Execution |
| Local vector space | Large objects curated locally | Excecution |
| Cache cons space | Copies of small objects curated elsewhere | Communications |
| Cache vector space | Copies of large objects curated elsewhere | Communications |

So, the execution thread is chuntering merrily along, and it encounters a data item it needs to get from another node. This is intended to happen all the time: every time a function of more than one argument is evaluated, the node will seek to farm out some of the arguments to idle neighbours for evaluation. So the results will often be curated by them. My original vague idea was that the execution node would choose the argument which seemed most costly to evaluate to evaluate locally, pass off the others to neighbours, evaluate the hard one, and by the time that was done probably all the farmed out results would already be back. 

The move from cons space objects to the more flexible [paged space objects](Paged-space-objects.md) doesn't really change this, in principle. There will still be a need for some objects which do not fit into pages, and will thus have to lurk in the outer darkness of vector space. Paged space should make the allocation of objects more efficient, but it doesn't change the fundamental issue

But there's an inevitable overhead to copying objects over inter-node links. Even if we have 64 bit (plus housekeeping) wide links, copying a four word object still takes four clock ticks. Of course, in the best case, we could be receiving six four word objects over the six links in those four clock ticks, but

1. The best case only applies to the node initiating a computation;
2. This ignores contention on the communication mesh consequent on hoppity-hop communications between more distant nodes.

So, even if the execution core correctly chose the most expensive argument to evaluate locally, it's quite likely that when it returns to the stack frame, some results from other nodes have still not arrived. What does it do then? Twiddle its thumbs?

It could start another thread, declare itself idle, accept a work request from a neighbour, execute that, and return to the frame to see whether its original task was ready to continue. One of the benefits of having the stack in managed space is that a single stack frame can have arbitrarily many 'next' frames, in arbitrarily many threads. This is exactly how [Interlisp](https://dl.acm.org/doi/10.1145/362375.362379) manages multitasking, after all. 

If we do it like that I think we're still safe, because it can't have left any data item in a half-modified state when it switched contexts.

But nevertheless, we still have the issue of contention between the execution process and the communications process. They both need to be able to mutate freelist pointers; and they both need to be able to mutate explicitly mutable objects, which for the present is just namespaces but this will change.

We can work around the freelist problem by assigning separate freelists for each size of paged-space objects to each processor, that's just sixteen more words. But if a foreign node wants to change a value in a local namespace, then the communications process needs to be able to make that change.

Which means we have to be able to lock objects. Which is something I didn't want to have to do.

## Mutexes

It's part of the underlying philosophy of the post scarcity project that one person can't be expert in every part of the stack. I don't fully understand the subtleties of thread safe locking. In my initial draft of this essay, I was planning to reserve one bit in the tag of an object as a thread lock.

There is a well respected standard thread locking library, [`pthreads`](https://www.cs.cmu.edu/afs/cs/academic/class/15492-f07/www/pthreads.html), part of the [POSIX](https://en.wikipedia.org/wiki/POSIX) standard, which implements thread locks. The lock object it implements is called a `mutex` ('mutual exclusion'), and the size of a `mutex` is... complicated. It is declared as a union:

```c
typedef union
{
  struct __pthread_mutex_s __data;
  char __size[__SIZEOF_PTHREAD_MUTEX_T];
  long int __align;
} pthread_mutex_t;

```

I guessed that the `long int __align` member was intended as a contract that this would be *no bigger* than a `long int`, but `long int` may mean 32 or 64 bits depending on context. The payload is clearly `__pthread_mutex_s`; so how big is that? Answer: it varies, dependent on the hardware architecture. But `__SIZEOF_PTHREAD_MUTEX_T` also varies dependent on architecture, and is defined as 40 *bytes* on 64 bit Intel machines:

```c
#ifdef __x86_64__
# if __WORDSIZE == 64
#  define __SIZEOF_PTHREAD_MUTEX_T 40
...
```

The header file I have access to declares that for 32 bit Intel machines it's 32 bytes and for all non-Intel machines the size is only 24 bytes, but

1. the machines I'm working on are actually AMD, but x86 64 bit Intel architecture; and
2. I don't currently have a 64 bit ARM version of this library, and ARM is quite likely to be the architecture I would use for a hardware implementation;

So let's be cautious.

Let's also be realistic: what I'm building now is the 0.1.0 prototype, which is not planned to run on even a simulated hypercube, so it doesn't need to have locks at all. I am crossing a bridge I do not yet strictly need to cross.

## Where to put the lock?

Currently, we have namespaces implemented as hashtables (or hashmaps, if you prefer, but I appreciate that it's old fashioned). We have hashtables implemented as an array of buckets. We have buckets implemented, currently, as association lists (lists of dotted pairs), although they could later be implemented as further hashtables. We can always cons a new `(key . value)` pair onto the front of an association list; the fact that there may be a different binding of the same key further down the association list doesn't matter, except in so far as it slows further searches down that association list.

Changing the pointer to the bucket happens in one clock tick: we're writing one 64 bit word to memory over a 64 bit wide address bus. The replacement bucket can &mdash; must! &mdash; be prepared in advance. So changing the bucket is pretty much an atomic operation. 

But the size of a mutex is uncertain, and **must** fit within the footprint of the namespace object. 

Forty bytes is (on a 64 bit machine) five words; but, more relevantly, our `pso_pointer` object is 64 bits irrespective of hardware architecture, so forty bytes is the size of five (pointers to) buckets. This means that namespaces are no longer 'the same' as hashtables; hashtables can accommodate (at least) five more buckets within a given [paged space object](Paged-space-objects.md) size. But obviously we can &mdash; the whole paged space objects architecture is predicated on ensuring that we can &mdash; accommodate any moderately sized fixed size datum into a paged space object, so we can accommodate a mutex into the footprint of a namespace object.

Oh, but wait.

Oh, but wait, here's a more beautiful idea.

### First class mutexes

We can make the mutex a first class object in paged space in its own right.

This has a number of advantages:

1. the space we need to reserve in the namespace object is just a pointer like any other pointer, and is not implementation dependent;
2. we can change the implementation of the mutex object, if we need to do so when changing architecture, without changing the implementation of anything which relies on a mutex;
3. mutexes then become available as ordinary objects in the Lisp system, to be used by any Lisp functions which need to do thread-safe locking.

So we need a new Lisp function,

```lisp
(with-lock mutex forms...)
```

which, when called

1. waits until it can lock the specified mutex;
2. evaluates each of the forms sequentially in the context of that locked mutex;
3. if evaluation of any of the forms results in the throwing of an exception, catches the exception, unlocks the mutex, and then re-throws the exception;
4. on successful completion of the evaluation of the forms, unlocks the mutex and returns the value of the last form.

This means that I *could* write the bootstrap layer namespace handling code non-thread-safe, and then reimplement it for the user layer in Lisp, thread-safe. But it also means that users could write thread safe handlers for any new types of mutable object they need to define.

### Other types

We don't currently have any other mutable objects, but in future at least lazy objects will be mutable; we may have other things that are mutable. It doesn't seem silly to have a single consistent way to store locks, even if it will only be used in the case of a small minority of objects.

## Procedure for using the lock

### Reading namespaces

Secondly, reading from a namespace does not happen in a single clock tick, it takes quite a long time. So it's no good setting a lock bit on the namespace object itself and then immediately assuming that it's now mutable. A reading process could already have started, and be proceeding.

So what I think is, that we have a single top level function, `(::substrate:search-store key store return-key?)` (which we already sort of have in the 0.0.6 prototype, [here](https://www.journeyman.cc/post-scarcity/doc/html/intern_8c.html#a2189c0ab60e57a70adeb32aca99dbc43)). This searches a store (hashmap, namespace, association list, or hybrid association list) to find a binding for a key, and, having found that binding, then, if there is a namespace on the search path, checks whether the lock on the any namespace on the search path is set, and if it is, aborts the search and tries again; but otherwise returns either the key found (if `return-key?` is non-`nil`), or the value found otherwise.

This function implements the user-level Lisp functions `assoc`, `interned`, and `interned?`. It also implements *hashmap-in-function-position* and *keyword-in-function-position*, in so far as both of these are treated as calls to `assoc`.

### Writing namespaces

When writing to a namespace, top level function [`(::substrate:set key value store)`](https://www.journeyman.cc/post-scarcity/doc/html/intern_8c.html#af8e370c233928d41c268874a6aa5d9e2), we first try to acquire the lock on the namespace. If it is not available, we pause a short time, and try again. It it is clear, we lock it, then identify the right bucket, then cons the new `(key . value)` pair onto the front of the bucket[^1], then update the bucket pointer, and finally unlock the lock.

This function implements the user-level Lisp functions `set` and `set!`.

### Allocating/deallocating objects

When allocating a new object from a freelist... Actually, a lock on the tag of the `car` of the freelist doesn't work here. The lock has to be somewhere else. We could have a single lock for all freelists; that feels like a bad idea because it means e.g. that you can't allocate stack frames while allocating cons cells, and you're bound to get in a mess there. But actually, allocating and deallocating objects of size class 2 &mdash; cons cells, integers, other numbers, links in strings, many other small things &mdash; is going to be happening all the time, so I'm not sure that it makes much difference. Most of the contention is going to be in size class 2. Nevertheless, one lock per size class is probably not a bad idea, and doesn't take up much space.

So: one lock per freelist.

When allocating *or deallocating* objects, we first try to obtain the lock for the freelist. If it is already locked, wait and try again. If it is clear, lock it, make the necessary change to the freelist, then unlock it.

[^1]: We probably remove any older bindings of the same key from the bucket at this point, too, because it will speed later searches, but this is not critical.

