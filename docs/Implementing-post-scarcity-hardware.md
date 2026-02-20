# Implementing Post Scarcity Hardware

The address space hinted at by using 64 bit cons-space and a 64 bit vector space containing objects each of whose length may be up to 1.4e20 bytes (2^64 of 64 bit words) is so large that a completely populated post-scarcity hardware machine can probably never be built. But that doesn't mean I'm wrong to specify such an address space: if we can make this architecture work for machines that can't (yet, anyway) be built, it will work for machines that can; and, changing the size of the pointers, which one might wish to do for storage economy, can be done with a few edits to consspaceobject.h.

But, for the moment, let's discuss a potential 32 bit psh machine, and how it might be built.

## Pass one: a literal implementation

 Let's say a processing node comprises a two core 32 bit processor, such as an ARM, 4GB of RAM, and a custom router chip. On each node, core zero is the actual processing node, and core one handles communications. We arrange these on a printed circuit board that is 4 nodes by 4 nodes. Each node is connected to the nodes in front, behind, left and right by tracks on the board, and by pins to the nodes on the boards above and below. On the edges of the board, the tracks which have no 'next neighbour' lead to some sort of reasonably high speed bidirectional serial connection &mdash; I'm imagining optical fibre (or possibly pairs of optical fibre, one for each direction). These boards are assembled in stacks of four, and the 'up' pins on the top board and the 'down' pins (or sockets) on the bottom board connect to similar high speed serial connectors.

 This unit of 4 boards &mdash; 64 compute nodes &mdash; now forms both a logical and a physical cube. Let's call this cube module a crystal. Connect left to right, top to bottom and back to front, and you have a hypercube. But take another identical crystal, place it along side, connect the right of crystal A to the left of crystal B and the right of B to the left of A, leaving the tops and bottoms and fronts and backs of those crystals still connected to themselves, and you have a larger cuboid with more compute power and address space but slightly lower path efficiency. Continue in this manner until you have four layers of four crystals, and you have a compute unit of 4096 nodes. So the basic 4x4x4 building block &mdash; the 'crystal' &mdash; is a good place to start, and it is in some measure affordable to build &mdash; low numbers of thousands of pounds, even for a prototype.

 I imagine you could get away with a two layer board &mdash; you might need more, I'm no expert in these things, but the data tracks between nodes can all go on one layer, and then you can have a raster bus on the other layer which carries power, backup data, and common signals (if needed).

 So, each node has 4Gb of memory (or more, or less &mdash; 4Gb here is just illustrative). How is that memory organised? It could be treated as a heap, or it could be treated as four separate pages, but it must store four logical blocks of data: its own curated conspage, from which other nodes can request copies of objects; its own private housekeeping data (which can also be a conspage, but from which other nodes can't request copies); its cache of copies of data copied from other nodes; and its heap.

 Note that a crystal of 64 nodes each with 4Gb or RAM has a total memory of 256Gb, which easily fits onto a single current generation hard disk or SSD module. So I'm envisaging that either the nodes take turns to back up their memory to backing store all the time during normal operation. They (obviously) don't need to backup their cache, since they don't curate it.

 What does this cost? About £15 per processor chip, plus £30 for memory, plus the router, which is custom but probably still in tens of pounds, plus a share of the cost of the board; probably under £100 per node, or £6500 for the 'crystal'.

## Pass two: a virtual implementation

 OK, OK, this crystal cube is a pretty concept, but let's get real. Using one core of each of 64 chips makes the architecture very concrete, but it's not necessarily efficient, either computationally or financially.

 64 core ARM chips already exist:

 1. [Qualcom Hydra](https://eltechs.com/hydra-is-the-name-of-qualcomms-64-core-arm-server-processor/) - 64 of 64 bit cores;
 2. [Macom X-Gene](https://www.apm.com/products/data-center/x-gene-family/x-gene/) - 64 of 64 bit cores;
 2. [Phytium Mars](https://www.nextplatform.com/2016/09/01/details-emerge-chinas-64-core-arm-chip/) - 64 cores, but frustratingly this does not say whether cores are 32 or 64 bit

 There are other interesting chips which aren't strictly 64 core:

 1. [Cavium ThunderX](https://www.servethehome.com/exclusive-first-cavium-thunderx-dual-48-core-96-core-total-arm-benchmarks) - ARM; 96 cores, each 64 bit, in pairs of two, shipping now;
 2. [Sparc M8](https://www.servethehome.com/oracle-sparc-m8-released-32-cores-256-threads-5-0ghz/) - 32 of 64 bit cores each capable of 8 concurrent threads; shipping now.

## Implementing the virtual hypercube

 Of course, these chips are not designed as hypercubes. We can't route our own network of physical connections into the chips, so our communications channels have to be virtual. But we can implement a communications channel as a pair of buffers, an 'upstream' buffer writable by the lower-numbered processor and readable by the higher, and a 'downstream' buffer writable by the higher numbered processor and readable by the lower. Each buffer should be at least big enough to write a whole cons page object into, optionally including a cryptographic signature if that is implemented. Each pair of buffers also needs at least four bits of flags, in order to be able, for each direction, to be able to signal

 0. Idle &mdash; the processor at the receiving end is idle and can accept work;
 1. Busy writing &mdash; the processor at the sending end is writing data to the buffer, which is not yet complete;
 2. Ready to read &mdash; the processor at the sending end has written data to the buffer, and it is complete;
 3. Read &mdash; the processor at the receiving end has read the current contents of the buffer.

 Thus I think it takes at least six clock ticks to write the buffer (set busy-writing, copy four 64 bit words into the buffer, set ready-to-read) and five to read it out &mdash; again, more if the messages are cryptographically signed &mdash; for an eleven clock tick transfer (the buffers may be allocated in main memory, but in practice they will always live in L2 cache). That's probably cheaper than making a stack frame. All communications channels within the 'crystal' cost exactly the same.

 But note! As in the virtual design, a single thread cannot at the same time execute user program and listen to communications from neighbours. So a node has to be able to run two threads. Whether that's two threads on a single core, or two cores per node, is a detail. But it makes the ThunderX and Spark M8 designs look particularly interesting.

 But note that there's one huge advantage that this single-chip virtual crystal has over the literal design: all cores access the same memory pool. Consequently, vector space objects never have to be passed hop, hop, hop across the communications network, all can be accessed directly; and to pass a list, all you have to pass is its first cons cell. So any S-Expression can be passed from any node to any of its 6 proximal neighbours in one hop.

 There are downsides to this, too. While communication inside the crystal is easier and quicker, communication between crystals becomes a lot more complex and I don't yet even have an idea how it might work. Also, contention on the main address bus, with 64 processors all trying to write to and read from the same memory at the same time, is likely to be horrendous, leading to much lower speed than the solution where each node has its own memory.

 On a cost side, you probably fit this all onto one printed circuit board as against the 4 of the 'literal' design; the single processor chip is likely to cost around £400; and the memory will probably be a little cheaper than on the literal design; and you don't need the custom routers, or the connection hardware, or the optical transceivers. So the cost probably looks more like £5,000. Note also that this virtual crystal has 64 bit processors (although address bus contention will almost certainly burn all that advantage and more).

An experimental post-scarcity machine can be built now &mdash; and I can almost afford to build it. I don't have the skills, of course; but I can learn.



## Size of a fully populated machine

### Memory size

To fully implement the software specification as currently written, each node would need 128Gb of RAM for its curated cons space alone (since we can have 2<sup>32</sup> cons cells each of 32 bytes); an amount of memory for vector space; substantial cache of objects being processed by the node but curated by other nodes; and scratchpad space. 

How much memory for vector space? The current software specification allows for vectors up to 32 times the total address space of currently available 64 bit processors. But not only could such objects not easily be stored with current generation technology, they could also not be copied across the hypercube lattice in any useful sort of time. So functions which operate on large vector space objects would necessarily have to migrate to the node where the object is curated, rather than have the object migrate. I don't currently have an account of how this could be done.

However, obviously it is unaffordable to build a machine which can explore problems like that as a first prototype, so this is at present academic.

### Lattice size

If we hold to the doctrine of one cons page per node, which has the advantage of making addressing reasonably simple, then there can be up to 2<sup>32</sup>, or 4,294,967,296 nodes, forming a hypercube of 1625 x 1625 x 1625 nodes. The total address space of this machine would be of the order of 79,228,162,514,264,337,593,543,950,336 bytes, or 7.9x10<sup>28</sup>. This is about 7 brontobytes - far beyond the zetabytes of my original sketch.

Hello, I seem to have designed a computer which would terrify even the [Magratheans](https://hitchhikers.fandom.com/wiki/Magrathea).

