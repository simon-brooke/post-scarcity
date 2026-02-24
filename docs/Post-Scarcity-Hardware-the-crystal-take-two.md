# Post Scarcity Hardware: the crystal, take two

In my previous essay on hardware for the Post Scarcity system, [Implementing Post Scarcity Hardware](Implementing-post-scarcity-hardware.md), I proposed a hypercube structure built of modules called crystals, each itself a cube of  64 modules called nodes, arranged in an 8 x 8 x 8 lattice, and each having bidirectional serial connections to its up, down, north, south, east and west neighbours. A single crystal can form a hypercube in itself by linking the cells of each of its outside faces to the cells on its opposite face; or they can be plugged together to form larger hypercubes.

In that essay I proposed a number of details on which my thinking has moved on.

First, I proposed that the nodes should be based on commercially available 32 or 64 bit processors. Custom hardware would be needed on each node only for its router, which runs the six bidirectional connections to neighbours. For prototyping that still makes sense, although I will sketch an idea for fully custom hardware in this essay.

I suggested that the address space of each node might be partitioned into four fixed and distinct spaces: its locally curated cons space for its own locally created cells; its locally curated vector space, for larger locally created objects; a space for cached copies of cons cells curated by other nodes; and a space for cached copies of vector space objects curated by other nodes.

I never liked this 'four distinct spaces' idea. As I wrote way back in my first essay on [Post Scarcity software](Post-scarcity-software.md), one of the things that essay was in reaction against was the fixed size stacks on (e.g.) the Java virtual machine, and, more generally, that a software system should hit a wall when it ran out of memory in some arbitrarily delimited space.

So I'm now back to the idea of each node treating its physical memory as one undifferentiated vector space, with its own cons pages, being arrays of equal sized cons-space objects, floating in that vector space. I'm proposing two new types of cons space object. 

The idea of having four distinct spaces per node was that each node would curate just one cons page, and that  each cons pointer was 64 bits comprising 32 bits of originating node address, and 32 bits of page offset. 

I'm still thinking that 64 bits is a not-unreasonable size for a cons pointer, but that it should now be considered made up of three distinct fields, node address, page number, offset. The exact sizes of each of those fields can be variable, but

```
+------*------*---------+
| 0    | 32   | 40...63 |
+------*------*---------+
| Node | Page | Offset  |
+------*------*---------+

```

would allow for a hypercube with edges 536,870,912 &mdash; half a billion &mdash; nodes long, with each node capable of addressing 256 pages of each of 16,777,216 cells for a total of 4 billion cells, each of 32 bytes. So the cells alone addressable by a single node could occupy 2<sup>37</sup> =  137,438,953,472 bytes; but each node would have a 64 bit address bus, so the potential heap is vastly larger.

In practice, I don't actually need a 64 bit cons pointer, and at some stage I may make a pragmatic decision to make it smaller. But the whole idea of the post scarcity computing project is to design systems as though there weren't physical constraints on them, so I'm not proposing to change it yet.

## The `CACH` Cell

The first of these is the cache cell, with the tag `CACH`. A cache cell is like a cons cell, which is like a cons cell except that its `CAR` points to the foreign object which has been cached, and its `CDR` points to the local copy.

There is a local namespace, `*cache*`, which holds a pointer to each such `CACH` cell indexed by the address of the foreign object it points to. A local sweep operation notes cells pointed to by any local cache cell in the `*cache*` which have only one remaining reference, removes the `CACH` cell from the `*cache*` into a temporary holding store (probably an assoc list, possibly a private namespace), sends a message to the owning node to decrement the reference to the object, and, on receiving confirmation that this has been received, decrements (and thus frees) the `CACH` cell and local copy.

Obviously, when any user space function references a cache cell as argument, what is fetched is the locally cached copy of the foreign object, an indirection which needs to be handled by `eval`. When a user space function references a foreign object of which there is a local copy in `*cache*`, then the local copy is fetched. If there isn't a local copy in cache, then execution is obviously halted while the master copy is fetched hopitty hop across the hypercube, which is obviously expensive and undesirable.

Consequently, copies of essential variables, functions and namespaces should be broadcast at bootstrap time and copied by each node. The only mutable things in this system are namespaces and output streams. Output streams are only readable by their destination, so nothing else needs to be alerted if they change. But any node may hold a cached copy of a namespace, so if a namespace is changed a change notification needs to be broadcast, or else every time a function on a node references a name in namespace, execution needs to halt while the curating node is queried whether the the binding has changed.

Both of these solutions are expensive. Probably the best compromise is to have two tiers of namespaces, those which broadcast changes (probably reserved for essential system namespaces), and those which have to be checked when accessed. Note that, provided the binding hasn't changed, nothing below the binding can have changed unless it also is a namespace, so nothing needs to be refetched.

## The `PROT` cell

I've given myself 32 bits of tag space, mainly to allow a simple representation of mnemonics as tags. For this reason, all the tags I've so far assigned have values which, considered as ASCII strings, represent four upper case characters. There are thus 456,976 possible upper case tags, and an equal number of possible lower case tags. I have a thought that tags encoding mnemonics in all upper could be tags of system level cons space object types, and tags encoding mnemonics in all lower could be tags of user created cons space object types.

But if users are able to create their own new types of cons space object, there has to be a way of specifying to the system how to use those novel cell types, and what sorts of operations are legal on them.

This is where the `PROT` &mdash; or `PROT`otype &mdash; cell comes in.

A cons space object is something which can be stored in [a cons cell](Cons-space.md), which has a fixed payload size of 128 bits.

In designing the bootstrapping cons space object types of the system, I've designed cells which are essentially two 64 bit pointers (such as `CONS` or `RTIO`); one which is a single 128 bit [IEEE754]() floating point number (`REAL`); one which is a single `unsigned __int128` (`TIME`); several which comprise one 32 bit `wide character`, some padding, and a cons pointer to another cell of the same type (`KEYW`, `STRG`, `SYMB`); one which comprises a tag, some padding, and a 64 bit pointer into vector space (`VECP`); ones that are simply markers and have no payload (`LOOP`, `WRKR`) , and so on.

There are a lot of different sorts of things you can store in 128 bits of memory. You can divide it up into fields any way you please, and store anything you like &mdash; that will fit &mdash; in those fields.

## The Node Processor hardware

I suggested in my earlier essay that the node processors could be off the shelf parts, probably ARM chips. But the router still needs to be custom silicon. If you were to do custom silicon for the node processor, what would it look like?

Well, firstly, although it could have a very small instruction set, I don't think it would count as strictly a RISC processor. The reason it wouldn't is that some of the instructions would be themselves recursive, meaning they could not complete in a single clock cycle.

So, what does it look like?

Firstly, it must have at least one register in which it can construct a complete cons space object, which is to say, 256 bits. 

It must have sufficient registers to represent the full content of a stack frame, which is to say eleven 64 bit cons pointers and one 32 bit argument counter, so at least 736 bits (but 768 probably makes more sense). But note that a function call with zero args needs only 160 bits, one with one arg needs only 224 bits, one with three, 288 bits register. So when evaluating functions with low numbers of arguments, it's at least potentially possible for the processor to use unused bits in the stack frame register as additional shipyards in which to assemble cons space objects.

H'mmm. You need two stack frame registers, one for the frame you're evaluating, and one for the frame you're assembling. I think you also need an additional cons space object shipyard, for the cons space object (VECP) which will point to the current frame when is released.

### Instructions



