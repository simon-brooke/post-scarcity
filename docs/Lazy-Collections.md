# Lazy Collections

If we're serious about a massively parallel architecture, and especially if we're serious about  passing argument evaluation off to peer processors, then it would be madness not to implement lazy collections.

Architecturally lazy collections violate the 'everything is immutable' rule, but only in a special case: you can do the equivalent of `replacd` on a cell that forms part of lazy collection, but only as you create it, before you release it to another process to be consumed; and you may only do it once, to replace the `NIL` list termination value with a pointer to a new lazy sell.

From the consumer's point of view, assuming for the moment that the consumer is a peer processor, it looks just like a list, except that when you request an element which is, as it were, at the write cursor &mdash; in other words, the producing process hasn't yet generated the next element in the collection &mdash;  the request will block.

## How are lazy sequences created?

Essentially, lazy collections are created by a very few primitive functions; indeed, it may boil down to just two,`read` which reads characters from a stream, and `mapc` which applies a function to successive values from a sequence. `reduce` may be a third lazy-generating function, but I suspect that it may be possible to write `reduce` using `mapc`. 

## What does a lazy sequence look like?

Essentially you can have a lazy sequence of objects, which looks exactly like a list except that it's lazy, of a lazy sequence of characters, which looks exactly like a string except that it's lazy. For practical purposes it would be possible for `mapc` to generate perfectly normal `CONS` cells and for `read` to generate perfectly normal `STRG` cells, but we've actually no shortage of tags, and I think it would be useful for debugging and also for process scheduling to know whether one is dealing with a lazy construct or not. so I propose three new tags:

* `LZYC` &mdash; like a cons cell, but lazy;
* `LZYS` &mdash; like a string cell, but lazy;
* `LZYW` &mdash; the thing at the end of a lazy sequence which does some work when kicked.

I acknowledge that, given that keywords and symbols are also sequences of characters, one might also have lazy symbols and lazy keywords but I'm struggling to think of situations in which these would be useful.

## How do we compute with lazy sequences in practice?

Consider the note [parallelism](Parallelism.html). Briefly, this proposes that a compile time judgement is made at the probable cost of evaluating each argument; that the one deemed most expensive to evaluate is reserved to be evaluated on the local node, and for the rest, a judgement is made as to whether it would be cheaper to hand them off to peers or to evaluate them locally. Well, for functions which return lazies –– and the compiler should certainly be able to infer whether a function will return a lazy &mdash; it will always make sense to hand them off, if there is an available idle peer to which to hand off. In fact, lazy-producers are probably the most beneficial class of function calls to hand off, since, if handed off to a peer, the output of the function can be consumed without any fancy scheduling on the local node. Indeed, if all lazy-producers can be reliably handed off, we probably don't need a scheduler at all.

## How do lazy sequences actually work?

As you iterate down a lazy list, you may come upon a cell whose `CDR` points to a 'Lazy Worker' (`WRKR`) cell. This is a pointer to a function. You (by which I mean the CDR function &mdash; just the ordinary everyday CDR function) mark the cell as locked, and call the function. 

1. It may compute and return the next value; or 
2.  it may return a special marker (which cannot be `NIL`, since that's a legitimate value), indicating that there will be no further values; or 
3. it may block, waiting for the next value to arrive from a stream or something; or 
4. it may return a special value (which I suspect may just be its own address) to indicate that 'yes, there are in principle more values to come but they're not ready yet'.

In cases 1 above, you replace the `CDR` (hard replace &mdash; actual immutability defying change) of the sequence cell you were looking at with a new sequence cell of the same type whose `CAR` points to the newly delivered value and whose `CDR` points to the `WRKR` cell.
