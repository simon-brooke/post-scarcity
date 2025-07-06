# Memory management

Most of the memory management ideas that I want to experiment with this thing are documented in essays on my blog:

1. [Functional languages, memory management, and modern language runtimes](https://www.journeyman.cc/blog/posts-output/2013-08-23-functional-languages-memory-management-and-modern-language-runtimes/)
2. [Reference counting, and the garbage collection of equal sized objects](https://www.journeyman.cc/blog/posts-output/2013-08-25-reference-counting-and-the-garbage-collection-of-equal-sized-objects/)

Brief summary:

# The problem

My early experience included Lisps with older, pre-generational, mark and sweep garbage collectors. The performance of these was nothing like as bad as some modern texts claim, but they did totally halt execution of the program for a period of seconds at unpredictable intervals. This is really undesirable.

I became interested in reference counting garbage collectors, because it seemed likely that these would generate shorter pauses in execution. But received wisdom was that mark-and-sweep outperformed reference counting (which it probably does, overall), and that in any case generational garbage collection has so improved mark and sweep performance that the problem has gone away. I don't wholly accept this.

## Separating cons space from vector space

Lisps generate lots and lots of very small, equal sized objects: cons cells and other things which are either the same size as or even smaller than cons cells and which fit into the same memory footprint. Furthermore, most of the volatility is in cons cells - they are often extremely short lived. Larger objects are allocated much more infrequently and tend to live considerably longer.

Because cons cells are all the same size, and because integers and doubles fit into the memory footprint of a cons  cell, if we maintain an array of memory units of this size then we can allocate them very efficiently because we never have to move them - we can always allocate a new object in memory vacated by deallocating an old one. Deallocation is simply a matter of pushing the deallocated cell onto the front of the free list; allocation is simply a matter of popping a cell off the free list.

By contrast, a conventional software heap fragments exactly because we allocate variable sized objects into it. When an object is deallocated, it leaves a hole in the heap, into which we can only allocate objects of the same size or smaller. And because objects are heterogeneously sized, it's probable that the next object we get to allocate in it will be smaller, leaving even smaller unused holes.

Consequently we end up with a memory like a swiss cheese - by no means fully occupied, but with holes which are too small to fit anything useful in. In order to make memory in this state useful, you have to mark and sweep it.

So my first observation is that [[cons space]] and what I call [[vector space]] - that is, the heap into which objects which won't fit into the memory footprint of a cons cell are allocated - are systematically different and require different garbage collection strategies.

## Reference counting: the objections

### Lockin from reference count overflow

Older reference counting Lisps tended to allocate very few bits for the reference count. Typically a cons cell was allocated in 32 bits, with two twelve bit pointers, leaving a total of eight bits for the tag bits and the reference counter bits. So you had reference counters with only eight or sixteen possible values. When a reference counter hits the top value which can be stored in its field, it cannot be further incremented. So you cannot ever decrement it, because you don't know whether it represents `max-value` or, e.g., `max-value + 21`. So any cell which ended up with that number of other cells pointing to it, even if only very temporarily, got locked into memory and couldn't be garbage collected.

But modern computers have vastly more memory than the computers on which those Lisps ran. My desktop machine now has more than 16,000 times as much memory as my very expensive workstation of only thirty years ago. We can afford much bigger reference counter fields. So the risk of hitting the maximum reference count value is much lower.

### Lockin from circular data structures

The other 'fault' of older reference counting Lisps is that in older Lisps, cons cells were not immutable. There were functions *RPLACA* and *RPLACD* which overwrote the value of the CAR and CDR pointers of a cons cell respectively. Thus it was possible to create circular data structures. In a reference counting Lisp, a circular data structure can never be garbage collected even if nothing outside the circle any longer points to it, because each cell in the circle is pointed to by another cell in the circle. Worse, any data structure outside the circle that is pointed to by a cell in the circle also cannot ever be garbage collected.

So badly designed programs on reference counting Lisps could leak memory badly and consequently silt up and run out of allocatable store.

But modern Lisps - like Clojure - use immutable data structures. The nature of immutable data structures is that an older node can never point to a newer node. So circular data structures cannot be constructed.

### Performance

When a memory management system with a reference counting garbage collector allocates a new cons cell, it needs to increment the reference counts on each of the cells the new cell points to. One with a mark-and-sweep garbage collector doesn't have to do this. When the system deallocates a cell, it has to decrement the counts on each of the cells it pointed to. One with a mark-and-sweep garbage collector doesn't have to do this. Overall, many well informed people have claimed to me, the performance of memory management system with a mark-and-sweep collector is a bit better than one with a reference counting garbage collector.

That's probably true.

## Why I think reference counting is nevertheless a good idea

A reference counting garbage collector does a little bit of work every time a cell (or something else that points to a cell) is deallocated. It's unlikely ever to pause the system for noticeable time. In the extreme case, when you remove the one link that points to a massive, complex data structure, the cascade of deallocations might produce a noticeable pause but in most programs that's a very rare occurrence. You could even run the deallocator in a separate thread, evening out its impact on performance even further.

Also, over the thirty years, the performance of our computers has also improved immeasurably. My desktop machine now has over 6,000 times the performance of a DEC VAX. We can afford to be a little bit inefficient. If reference counting were to prove to be 10% slower overall, it might still be preferable if it gave smoother performance.

## Separate freelists for different sized holes

We're still left with the possibility of fragmenting heap space in the manner I've described. Doing clever memory management of heap space can't be done if we depend on the memory management functions provided by the C or Rust compilers and their standard libraries, because while we know the size and location of memory allocations they've done for Lisp objects, we don't necessarily know the size or location of memory allocations they've made for other things.

So if we're going to do clever memory management of heap space, we're probably going to have to claim heap space from the system in pages and allocate our objects within these pages. This bothers me both because there's a possibility of space wasted at the edges of the pages, and because it becomes more complex to allocate objects that are bigger than a page.

The alternative is to bypass the standard library memory allocation and just allocate it ourselves, which will be tricky; but we can then manage the whole of the address space available to our process.

Either way, most of the Lisp objects we allocate will be of a few sizes. A default hashtable is always yay big, for example; it has this many buckets. How many 'this many' is I don't yet know, but the point is it will be a standard number. When a hashtable overflows that standard size, we'll automatically replace it with another of a bigger, but still standard size.

We're going to store strings in cons space, not vector space. There aren't a lot of things we're going to store in vector space. Probably raster images, or sound objects, will be the only ones which are truly variable in size.

So if we have three or four fixed sizes which accommodate most of our vector space objects, we should create free lists for each of those sizes. When deallocating a default sized hashtable, for example, we'll link the newly freed memory block onto a special purpose freelist of blocks the size of a default sized hashtable; when we want to allocate a new default sized hashtable, we'll first check that freelist and, if it has one, pop it from there, before we ask the system to allocate new virgin memory.

## Conclusion

Even the 'separate freelists for different sized objects' idea won't prevent vector space fragmenting, although it may slow it considerably. Ultimately, mark and sweep of vector space will be needed and will be very expensive. It can be made much less expensive by dividing cons pointers into a page number field and an offset field rather than being a simple memory address, but that will make all memory access slower, so it too is a trade off.

There are several possible approaches to optimising garbage collection. I want to try as many as possible of these approaches, although I'm not convinced that any of them will prove in the end better than a conventional generational garbage collector. Still, they're worth trying.

What we need to do is build a system into which different memory management subsystems can be plugged, and to develop a set of benchmarks which aggressively test memory allocation and deallocation, and then see how the different possibilities perform in practice.

## Further reading

I've written a [number of essays](https://blog.journeyman.cc/search/label/Memory%20management) on memory management, of which I'd particularly point you to [Reference counting, and the garbage collection of equal sized objects](https://blog.journeyman.cc/2013/08/reference-counting-and-garbage.html).