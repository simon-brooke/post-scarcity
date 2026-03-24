# Vector space, Pages, Mark-but-don't-sweep, and the world's slowest ever rapid prototype

By: Simon Brooke :: 13 March 2026

I started work on the Post-scarcity Software Environment on the second of January, 2017; which is to say, more than nine years ago. It was never intended to be a rapid prototype; it was intended, largely, to be a giant thought experiment. But now enough of it does work that I can see fundamental design mistakes, and I'm thinking about whether it's time to treat it as a rapid prototype: to take what has been learned from this code, and instead of trying to fix those mistakes, to start again from scratch.

So what are the mistakes?

## Allocating only cons-sized objects in pages

### What currently happens

The current post-scarcity prototype allocates objects that are the size of a cons cell in 'cons pages'. A cons page is an object that floats in vector space, which is to say the heap, which has a header to identify it, followed by an array of slots each of which is the size of a cons cell. When a cons page is initialised, each slot is initialised as a FREE object, and these are linked together onto the front of the global free list.

A cons pointer comprises a page part and an offset part. The exact size of these two parts is implementation dependent, but in the present implementation they're both uint32_t, which essentially means you can address four billion pages each of four billion slots; consequently, the size of the pointer is 64 bits, which means that the size of the payload of a cons cell is 128 bits. But a cons cell also needs a header to do housekeeping in, which is

struct cons_space_object {
    union {
        /** the tag (type) of this cell,
         * considered as bytes */
        char bytes[TAGLENGTH];
        /** the tag considered as a number */
        uint32_t value;
    } tag;
    /** the count of the number of references to this cell */
    uint32_t count;
    /** cons pointer to the access control list of this cell */
    struct cons_pointer access;
//...

which is to say, 32 bits tag, 32 bits reference count, 64 bits access control list pointer, total 16 bytes. So the whole cell is 32 bytes.

We currently have nineteen different types of object which can fit into the size of the payload of a cons cell (plus FREE, which is sort of a non-object, but must exist), namely


|​	| Tag (byte string) 	| Tag (numeric)	| Interpretation |
| ---- | ---- | ---- | ---- |
| 1	| CONS	| 1397641027	| An ordinary cons cell. | 
| 2	| EXEP	| 1346721861	| An exception.| 
| 3	| FREE	| 1162170950	| An unallocated cell on the free list — should never be encountered by a Lisp function. | 
| 4	| FUNC	| 1129207110	| An ordinary Lisp function — one whose arguments are pre-evaluated. | 
| 5	| INTR	| 1381256777	| An integer number (bignums are integers). | 
| 6	| KEYW	| 1465468235	| A keyword — an interned, self-evaluating string. | 
| 7	| LMDA	| 1094995276	| A lambda cell. Lambdas are the interpretable (source) versions of functions.| 
| 8	| LOOP	| 1347374924	| A loop exit is a special kind of exception which has exactly the same payload as an exception.| 
| 9	| NIL	| 541870414	| The special cons cell at address {0,0} whose car and cdr both point to itself.| 
| 10	| NLMD	| 1145916494	| An nlambda cell. NLambdas are the interpretable (source) versions of special forms.| 
| 11	| RTIO	| 1330205778	| A rational number, stored as pointers to two integers representing dividend and divisor respectively| 
| 12	| READ	| 1145128274	| An open read stream.| 
| 13	| REAL	| 1279346002	| A real number, represented internally as an IEEE 754-2008 binary128.| 
| 14	| SPFM	| 1296453715	| A special form — one whose arguments are not pre-evaluated but passed as provided.| 
| 15	| STRG	| 1196577875	| A string of characters, organised as a linked list.| 
| 16	| SYMB	| 1112365395	| A symbol is just like a keyword except not self-evaluating.| 
| 17	| TIME	| 1162692948	| A time stamp, representing milliseconds since the big bang.| 
| 18	| TRUE	| 1163219540	| The special cons cell at address {0,1} which is canonically different from NIL.| 
| 19	| VECP	| 1346585942	| A pointer to an object in vector space.| 
| 20	| WRIT	| 1414091351	| An open write stream.| 

Obviously it is absurdly wasteful to allocate 32 bits to a tag for twenty different types of object, but

1. The type system should be extensible; and
2. While debugging, it is useful to have human-readable mnemonics as tags.

But the point is, all these types of thing can be allocated into an identical footprint, which means that a cell can be popped off the free list and populated as any one of these; so that memory churn of objects of these types happens only in cons pages, not in the heap.
Why this is a good thing

Cons cells very often have quite transient life-cycles. They're allocated, and, in the majority of cases, deallocated, in the process of computation; of a single function call. Only a small minority of cons cells become parts of the values of interned symbols, and consequently retained in the long term. In other words, there is a lot of churn of cons cells. If you allocate and deallocate lots of small objects in the heap, the heap rapidly fragments, and then it becomes increasingly difficult to allocate new, larger objects.

But by organising them in pages with an internal free list, we can manage that churn in managed space, and only bother the heap allocator when all the cells in all the pages that we currently have allocated are themselves allocated.

Other objects which live in cons space, such as numbers, are also likely to experience considerable churn. Although I needed to solve the churn problem for cons cells, the fact that the same solution automatically generalises to all other cons space objects is a good thing.
### Why this needs to be different in future anyway

A two part cons pointer implies a single integrated address space, but in fact in a massively parallel machine we won't have that. In the final machine, the cons pointer would have to comprise three parts: a node part, a page part, and an offset part. And, indeed, in the next iteration of the project it ought to, because in the next iteration I do want to start experimenting with the hypercube topology. So actually, these parts are probably node: 32 bits; page; 8 bits; offset: 24 bits. So that you could have (in a fully populated machine) a hypercube of four billion nodes, each of which can locally address probably 256 pages each of sixteen million cells; and given that a cell is (currently) eight bytes, that's a total potential address space of 4,722,366,482,869,645,213,696 bytes, which is 4.7x1022, which is rather a lot.

You also need an additional cell type, CACH, a cache cell, a specialisation of CONS, whose first pointer points to the (foreign) cell which is cached, and whose second pointer points to the local (i.e. in this node's cons space) copy. When a non-local cell is first requested by EVAL,

1. the communications thread on the node requests it from the ('foreign') node which curates it;
2. the foreign node increments the reference counter on its copy;
3. the foreign node sends a representation of the content of the cell hoppity-hop across the grid to the requesting node;
4. the requesting node pops a cell off its local free list, writes into it the content it has received, increments its reference counter to one, pops a second cell off its free list, writes CACH into the tag, the address of the foreign cell into the first pointer, the address of the newly created copy into the second, and returns this second cell.

When the reference counter on a CACH cell is decremented to zero,

1. the communications thread on the requesting node notifies the curating node that the reference can be decremented;
2. the curating node decrements the reference and signals back that this has been done;
3. the requesting node clears both cells and pushes them back onto its free list.

### Why we should generalise this idea: stack frames

We currently allocate stack frames in vector space, which is to say on the heap. The payload of a stack frame is currently 96 bytes (eleven cons pointers plus two 32 bit integers):

```C
/*
 * number of arguments stored in a stack frame
 */
#define args_in_frame 8

/**
 * A stack frame. Yes, I know it isn't a cons-space object, but it's defined
 * here to avoid circularity. \todo refactor.
 */
    struct stack_frame {
        /** the previous frame. */
        struct cons_pointer previous;
        /** first 8 arument bindings. */
        struct cons_pointer arg[args_in_frame];
        /** list of any further argument bindings. */
        struct cons_pointer more;
        /** the function to be called. */
        struct cons_pointer function;
        /** the number of arguments provided. */
        int args;
        /** the depth of the stack below this frame */
        int depth;
    };
```

But because it's a Lisp object in vector space it also needs a vector space object header, so that we can identify it and manage it:

```c
/**
 * the header which forms the start of every vector space object.
 */
    struct vector_space_header {
    /** the tag (type) of this vector-space object. */
    union {
        /** the tag considered as bytes. */
        char bytes[TAGLENGTH];
        /** the tag considered as a number */
        uint32_t value;
    } tag;
    /** back pointer to the vector pointer which uniquely points to this vso */
    struct cons_pointer vecp;
    /** the size of my payload, in bytes */
    uint64_t size;
    };
```

which is a further twenty bytes, so one hundred and sixteen bytes in total. We're allocating one of these objects every time we evaluate a function; we're deallocating one every time we leave a function. The present prototype will happily run up a stack of several tens of thousands of frames, and collapse it back down again, in a single recursive computation.

That's a lot of churn.

If we allocated stack frames in pages, in the same way that we allocate cons cells, that churn would never hit the heap allocator: we would not fragment the heap.
Generalising the generalisation

So we have one set of objects which are each 32 bytes, and one set which are each 116; and just as there are lots of things which are not cons cells which can be fitted into the payload footprint of a cons cell, so I suspect we may find, when we move on to implementing things like regularities, that there many things which are not stack frames which fit into the payload footprint of a stack frame, more or less.

But the size of a stack frame is closely coupled to the number of registers of the actual hardware of the processor on the node; and though if I ever get round to building an actual prototype that's probably ARM64, I like the idea that there should at least in theory be a custom processor for nodes that runs Lisp on the metal, as the Symbolics Ivory did.

So while a cons cell payload probably really is 128 bits for all time, a stack frame payload is more mutable. Eight argument registers and one 'more' register seems about right to me, but...

However, if we say we will have a number of standard sizes of paged objects; that every paged object shall have the same sized header; that all objects on any given page shall be the same size; and that all pages shall fit into the same footprint (that is to say, a page with larger objects must needs have proportionally fewer of them), then we can say that the standard payload sizes, in bytes, shall be powers of two, and that we don't allocate a page for a standard size until we have a request to allocate an object of that size.

So our standard sizes have payloads of 1, 2, 4, 8, 16, 32, 64, 128, 256, 512...

I've highlighted 16 because that will accommodate all our existing cons space objects; 32 because that will accommodate my current implementation of hash tables and namespaces,128 because that will accommodate stack frames... But actually, we would do a much more efficient implementation of hash tables if we allocated an object big enough to have a separate pointer for each bucket, so we probably already have a need for three distinct standard sizes of object, and, as I say, I see benefit of having a generalised scheme.

In the current prototype I'm allocating pages to fit only 1024 cons cells each, because I wanted to be able to test running a free list across multiple pages. My current idea of the final size of a cons page is that it should accommodate 16 million (224) cells, which is 134 million (227) bytes. So on the generalised scheme, we would be able in principle to allocate a single object of up to ~134 megabytes in a page that would fit sixteen million cells, and we would only need to introduce any fragmentation into the heap if we needed to allocate single objects larger than this.

That seems a very big win.
## Mark but don't sweep

The post scarcity architecture was designed around the idea of a reference counting garbage collector, and I have a very clear idea of how you can make tracking references, and collecting garbage, work across a hypercube

[^1]: I'm not certain I'm using the word hypercube strictly correctly; the topology I'm contemplating is more than three dimensions but fewer than four. However, the architecture would scale to fractal dimensions greater than four, although I think it would get progressively harder to physically build such machines as the dimensions increase.

 in which pretty much every node is caching copies of objects which actually 'belong to', or are curated by, other nodes — provided that you can make reference counting work at all, which so far I'm struggling to do (but I think this is because I'm stupid, not because it's impossible).

I don't yet have a clear account of how you could make a non-reference counting garbage collector work across a distributed network.

However, I can see how, in having pages of equal sized objects, you can make garbage collection very much faster and can probably do it without even interrupting the evaluation thread.

Conventional mark and sweep garbage collectors — including generational garbage collectors — implement the following algorithm:

1. Halt execution of program evaluation;
2. Trace every single pointer on every single object in the generation being collected, and mark the object each points to;
3. Then go through every object in that generation, and those which have not been marked, schedule for overwriting;
4. Then move objects which have been marked downwards in memory to fill the voids left by objects which have not been marked (this is the sweeping phase);
5. Then correct all the pointers to all the objects which have been moved;
6. If that didn't recover enough memory, repeat for the previous generation, recursively;
7. Finally restart execution.

This is a really complicated operation and takes considerable time. It's this which is the main cause of the annoying pauses in programs which use automatic memory management. Of course, in a reference counting system, when you remove the last link to the top node of a large data structure, there is a cascade of decrements below it, but these can take place in a separate thread and do not have to interrupt program execution.

However, a large part of the cost of the mark-and-sweep algorithm is the sweep phase (and as I say, even generational systems have a sweep phase). The reason you need to sweep is to avoid fragmentation of the heap. If you allocate objects in equal sized pages each of equal sized objects, you can never fragment the heap, so (there is a problem here, but I'm going to ignore it for a moment and then come back to it), you never(ish) need to sweep.

You instead, when a page becomes full,

1. Don't halt program execution, but temporarily mark this page as locked (allocation can continue on other pages);
2. In a separate thread, trace all the links in this page and pages newer than this page to objects in this page, and mark those objects
      1. Obviously, if while this is happening the execution thread makes a new link to something on the locked page, then that something needs to be marked;
3. Clear all the objects which have not been marked, and push them back onto the free list of the page;
4. If all the objects on this page are now on the free list, deallocate this page. Otherwise, remove the locked marker on this page (allocation can resume on this page).

Program execution never needs to halt. If the node hardware architecture has two cores, an execution core and a communications core, then garbage collection can run on the communications core, and execution doesn't even have to slow. If it proves in practice that this slows communications too much, then perhaps a third core is needed, or perhaps you shift garbage collection back to a separate thread on the evaluation core.
The problem

So, I said there was a problem. Obviously, a page which is empty (every object in it is FREE) can safely be deallocated, and another page, perhaps for objects of a different size, can later be allocated in the same real estate. The problem is that, in the worst case, you might end up with two (or more) pages for a given size of object each of which was less than half full, but neither of which was empty. I don't currently see how you can merge the two pages into one without doing a mark-and-sweep, and without interrupting execution.

Also, if another node is holding a pointer to an object on one of the two half-empty pages, then the housekeeping to maintain track of which nodes hold pointers to what, and where that has been moved to, becomes very awkward.

So it may be that a hypercube running mark-but-don't-sweep would eventually suffer from coronary artery disease, which would mean this architecture would be a bust. But it might also be that in practice this wouldn't happen; that newer pages — which is inevitably where churn would occur — would automatically empty and be deallocated in the normal course of computation. I don't know; it's quite likely but I certainly don't have a proof of it.

## The substrate language

### Emerging from the stone age

I started work on the post scarcity software environment, as I say, nine years ago. At that time Rust could not do unions, and I was not aware of Zig at all. I needed — or at least, I thought I needed (and still do think I need) a language in which to write the substrate from which Lisp could be bootstrapped: a language in which the memory management layer would be written.

I needed a language in which I could write as close to the metal as possible. I chose C, and because I'm allergic to the Byzantine complexity of C++, I chose plain old vanilla C. I've written large programs in C before, but it is not a language I'm comfortable with. When things break horribly in C — as they do — I really struggle. The thing which has really held development of this system back is that I tried to write bignum arithmetic in C, and I have utterly failed to get it working. And then spent literally years beating myself up about it.

I've also failed to get my garbage collector working to my satisfaction; I don't think I'm incrementing and decrementing counters where I should be, and I feel that far too much garbage is not being collected. But it sort of works. Well enough for now.

The solutions to these problems would probably be absurdly obvious to someone who is actually a good software engineer, rather than just cos-playing one, but they have proved beyond me.

I've been unwilling to change the substrate language, because I've done an awful lot of work in the memory architecture in C and up to now I've been pretty satisfied with that work; and because Rust still doesn't look very appealing to me; and because I really have not yet fully evaluated Zig.

However...

If I am going to do a big rewrite of the bottom layer of the memory allocation system, then it would make sense to write it in a more modern language.
A bootstrap made of bootstraps

But more! One of the things I'm thinking looking at what I've built so far is that I've tried to do too much in the substrate. Bignums could have been implemented — much more easily, and probably not much less efficiently — in the Lisp layer. So could rationals (and complex numbers, and all sorts of other fancy number systems). So could hash tables and namespaces and regularities and homogeneities and all the other fancy data structures that I want to build.

To do that, I would need a Lisp which had functions to do low level manipulation of memory structures, which is something I don't want 'user level' programmers to be able to do. But I already have a Lisp with access control lists on every data item, including functions. So it will be trivial to implement a :system privilege layer, and to have functions written at that :system privilege layer that most users would not be entitled to invoke.
Conclusion, for now

Of course, it's now the end of winter, and big software projects are, for me, these days, winter occupations; in summer there is too much to do outside.

But I think my plan now is to

1. get version 0.0.6 just a little bit more polished so that other people can — if they're mad enough — play with it; and then call the 0.0.X series done;
2. start again with a new 0.1.X series, with a much shallower substrate written probably in Zig, with generalised paged memory objects;
3. write the access control list system, something of a use authentication system, something of a privilege layer system;
4. write Lisp functions which can directly manipulate memory objects, and, within the paged memory objects framework, define completely new types of memory objects;
5. write the north, south, east, west, up, down internode communication channels, so that I can start patching together a virtual hypercube;
6. write a launcher (in some language) which can launch n3 instances of the same Lisp image as processes on a single conventional UN*X machine, stitch their channels together so that they can communicate, and allow clients to connect (probably over SSH) so that users can open REPL sessions.

If I ever get that completed, the next goal is probably a compiler, and the goal after that build a real physical hypercube of edge 2, probably using ARM or RISC-V processors.