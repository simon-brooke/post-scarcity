# Cons space

*See [cons_space_object.h](consspaceobject_8h.html), [cons_page.h](conspage_8h.html).*

Cons space is a space which contains cons cells, and other objects whose memory representation fits into the footprint of a cons cell. A cons cell comprises:

    +-----+-------+------+----------------+--------------+--------------+
    | header                              | payload                     |
    +                                     +--------------+--------------+
    |                                     | car          | cdr          |
    +-----+-------+------+----------------+--------------+--------------+
    | tag | count | mark | access-control | cons-pointer | cons-pointer |
    +-----+-------+------+----------------+--------------+--------------+
    
The size of each of these components needs to be fixed in a running system. Traditional Lisps have made each as small as possible, but part of the point of thinking about post-scarcity software is to get away from thinking parsimoniously. While obviously we do still have finite store and at some stage in development it will be time to ask what are the sensible compromises to make, that time is not now. To get up and running, and to make debugging of memory sllocation easy, we'll be expansive.

## Header

Every cons space object has a header and each header shall be the same size with the same subdivisions.

### Tag

The tag identifies the type of the cons space object. There must be at least sufficient tag values for all the possible types of cons space objects. Actually the number of potential types of cons space objects is quite low, but at present I don't know how many I'll need. Sixteen values (4 bits) may be enough, but for the time being we'll reserve 32 bits. We'll think of these bits both as an unsigned 32 bit integer, and as a string of four ASCII characters; that is, we'll assign for now only numeric values which, if considered as an ASCII string, result in useful mnemonics. This will make memory dumps easy to read, which will aid in debugging memory allocation.

### Count

Either the count or the mark are redundant. The count is for reference counting garbage collection; the mark is for mark-and-sweep (including generational mark-and-sweep) garbage collection. Ultimately we probably need only one of these; conventional wisdom is that generational mark and sweep will win. But I want to banchmark both systems and see how they perform, so for now we'll have both.

A reference count counts how many other objects point to this object. When the reference count decrements to zero, the object may safely be garbage collected. However, when a reference count can no longer be safely incremented, neither can it ever be safely decremented. Suppose we had three bits - eight values including zero, 0...7 - for the reference count. Suppose six other objects point to this object, so the reference count is 6. Now suppose one of those objects is freed, so no longer points to this object. Our reference count is decremented to 5, and that's OK.

But, suppose seven objects already point to this object; our reference count is now 7. If an eigth object is created which points to this object, we cannot increment the reference count because we no longer have bits to store the incremented value. So we have to leave it at 7. Now, suppose another object which points to this object is freed: do we decrement the reference counter? No: we can't, because we can't know whether the actual number of objects which point to it is seven, or eight, or one hundred.

Consequently, for any size of reference counter, when it hits its maximum value it can no longer be decremented, and consequently a reference counting garbage collector can no longer free that object - ever. It is possible to write a hybrid reference-counting/mark-and-sweep garbage collector, but that is both expensive and complicated. We need a size of reference count which will very, very rarely overflow in practice. That's probably still quite small, but I'm proposing to reserve 24 bits (16,777,216 values) (in fact the current implementation reserves 32 bits - see [consspaceobject.h](https://github.com/simon-brooke/post-scarcity/blob/master/src/consspaceobject.h)).

### Mark

A mark and sweep garbage collector actually only needs one mark bit, but for now it will sit in the same space as the reference count, since we're only using one or other, never both.

### Access control

Access control is a [cons pointer](cons pointer.html), see below; and is consequently the size of a cons pointer, which is presently 64 bits. An access control value of NIL means only system processes may access the cell; an access control value of TRUE means any user can access the cell; otherwise, the access control pointer points to the first cons cell of a list of allowed users/groups. The access control list is thus an ordinary list in ordinary cons space, and cells in an access control list can have access control lists of their own. As cons cells are immutable, infinite recursion is impossible; but it is nevertheless probably a good thing if access control list cells normally have an access control list of either TRUE or NIL.

### Car, Cdr: Cons pointers

A [cons pointer](cons pointer.html) is simply a pointer to a cons cell, and the simplest way to implement this is exactly as the memory address of the cons cell. 

We have a fixed size vector of total memory, which we address in eight bit words (bytes) because that's the current convention. Our cons cell size is 32 bytes. So 31/32 of the possible values of a cons pointer are wasted - there cannot be a valid cons cell at that address. Also, our total memory must be divided between cons space, vector space and stack (actually stack could be implemented in either cons space or vector space, and ultimately may end up being implemented in cons space, but that's a highly non-trivial detail which will be addressed much later). In practice it's likely that less than half of the total memory available will be devoted to cons space. So 63/64 of the possible values of a cons pointer are wasted.

Is there a better way? Yes, there is, but as in all engineering matters it's a trade off.

One of the things I absolutely hate about modern computers is their tendency to run out of one 'sort' of memory while there is actually plenty of memory free. For example, it's childishly easy to run any JVM program out of stack space, because the JVM on startup reserves a fixed size block of memory for stack, and cannot extend this block. When it's exhausted, execution halts, and you've had your chips. There is no recovery.

That was acceptable when the JVM was a special purpose platform for developing software for small embedded devices, which is what it was originally designed for. But it's one of the compromises the JVM makes in order to work well on small embedded devices which is completely unacceptable for post-scarcity computing. And we won't accept it.

But be that as it may, we don't know at system initialisation time how much memory to reserve for cons space, and how much for vector space ('the heap'). If we reserve too much for cons space, we may run out of vector space while there's still cons space free, and vice versa. So we'll reserve cons space in units: [cons pages](cons pages.html). If our cons pointers are absolute memory addresses, then it becomes very expensive to move a cons page in memory, because all the pointers in the whole system to any cell on the page need to be updated.

(**NOTE**: As my thinking has developed, I'm now envisaging one cons page per compute node, which means that on each node the division between cons space and vector space will have to be fixed)

If, however, we divide our cons pointer into two elements, a page number and a page offset. Suppose we have 40 bits of page number (up to 1,099,511,627,776 - one trillion - pages) and 24 bits of page offset (up to 16,777,216 cons cells on a page), then suddenly we can address not 2^64 bytes of memory in cons space, but 32x2^64. Furthermore, this cons space addressing is now independent of vector space addressing, allowing even further address space.

Obviously it also means that to fetch any cell, the processor has to first fetch the address of the page, then compute the address of the offset of the cell in the page, then fetch the cell at the computed address, making three processor cycles instead of just one. We're post-scarcity: at this stage, we don't worry about such things. The time to worry about run-time performance is far beyond version 0.

So our cons cell is now 32 bytes, 256 bits:

    +-----+-------+------+----------------+--------------+--------------+
    | header                              | payload                     |
    +                                     +--------------+--------------+
    |                                     | car          | cdr          |
    +-----+-------+------+----------------+--------------+--------------+
    | 0   | 32    | 56   | 64             | 128          | 192   ...255 |
    | tag | count | mark | access-control | cons-pointer | cons-pointer |
    +-----+-------+------+----------------+--------------+--------------+

## Types of cons space object

This is a non-exhaustive list of types of things which may be stored in cons space; each has a memory representation which is 128 bits or less, and will thus fit in the memory footprint of a cons cell. There will be others I have not yet thought of, but this is enough to get us started.

### CONS

A cons cell. The tag value of a CONS cell is that unsigned 32 bit integer which, when considered as an ASCII string, reads 'CONS'. The count of a CONS cell is always non-zero. The mark is up to the garbage collector. The Car of a CONS cell is a pointer to another cons-space object, or NIL (address zero).

### FREE

An unassigned cons cell. The tag value of a FREE cell is that unsigned 32 bit integer which, when considered as an ASCII string, reads 'FREE'. The count of a FREE cell is always zero. The mark of a free cell is always zero. The access control value of a FREE cell is always NIL. The Car of a FREE cell is always NIL (address zero). The Cdr of a FREE cell is a cons-pointer to the next FREE cell (the [free list](free list.html) pointer).

### INTR

An integer; possibly an integer which isn't a big integer.  The tag value of a INTR cell is that unsigned 32 bit integer which, when considered as an ASCII string, reads 'INTR'. The count of a INTR cell is always non-zero. The mark is up to the garbage collector.

There's fundamentally two ways to do this; one is we store up to 128 bit signed integers in the payload of an INTR cell, and have some other tag for an integer ('[bignum](bignum.html)') which overflows 128 bits and must thus be stored in another data structure; or else we treat one bit as a 'bignum' flag. If the bignum flag is clear we treat the remaining 127 bits as an unsigned 127 bit integer; if set, we treat the low 64 bits of the value as a cons pointer to the data structure which represents the bignum.

### NIL

The canonical empty list. May not actually exist at all: the cell-pointer whose value is zero is deemed to point to the canonical empty list. However, if zero is a valid cell-pointer, the cell at pointer zero will be initialised with the tag "NIL " (i.e. the 32 bit unsigned integer which, when considered as an ASCII string, reads "NIL "). The count of the NIL cell is the maximum reference count value - that is, it can never be garbage collected. The mark is always 1 - that is, it can never be garbage collected. The access control value is TRUE - any user can read NIL. The payload is zero.

### READ

A stream open for reading. The tag value of a READ cell is that unsigned 32 bit integer which, when considered as an ASCII string, reads 'READ'. The count of a READ cell is always non-zero. The mark is up to the garbage collector.

I'm not yet certain what the payload of a READ cell is; it is implementation dependent and, at least in version zero, will probably be a file handle from the underlying system.

### REAL

A real number. The tag value of a REAL cell is that unsigned 32 bit integer which, when considered as an ASCII string, reads 'REAL'. The count of a REAL cell is always non-zero. The mark is up to the garbage collector. The payload is a IEEE 754R 128-bit floating point number.

### STRG

A string. The tag value of a STRG cell is that unsigned 32 bit integer which, when considered as an ASCII string, reads 'STRG'. The count of a STRG cell is always non-zero. The mark is up to the garbage collector. The Car of an STRG cell contains a single UTF character. The Cdr of an STRG cell contains a cons-pointer to the remainder of the string, or NIL if this is the end of the string.

Note that in this definition a string is not an atom, which is probably right. But we also at this stage don't have an idea of a [symbol](Interning-strings.html). Very likely we'll end up with the idea that a string which is bound to a value in a namespace is for our purposes a symbol.

Note, however, that there's a risk that we might have two instances of strings comprising identical characters in identical order, one of which was bound in a namespace and one of which wasn't; string equality is something to worry about.

### TIME

At nanosecond resolution (if I've done my arithmetic right), 128 bits will represent a span of 1 x 10<sup>22</sup> years, or much longer than from the big bang to the [estimated date of fuel exhaustion of all stars](https://en.wikipedia.org/wiki/Timeline_of_the_far_future). So I think I'll arbitrarily set an epoch 14Bn years before the UNIX epoch and go with that. The time will be unsigned - there is no time before the big bang.

### TRUE

The canonical true value. May not actually exist at all: the cell-pointer whose value is one is deemed to point to the canonical true value. However, if one is a valid cell-pointer, the cell at pointer zero will be initialised with the tag "TRUE" (i.e. the 32 bit unsigned integer which, when considered as an ASCII string, reads "TRUE"). The count of the TRUE cell is the maximum reference count value - that is, it can never be garbage collected. The mark is always 1 - that is, it can never be garbage collected. The access control value is TRUE: any user can read the canonical true value. The payload is zero.

### VECP

A pointer into vector space. The tag value of a VECP cell is that unsigned 32 bit integer which, when considered as an ASCII string, reads 'VECP'. The count of a VECP cell is always non-zero. The mark is up to the garbage collector. The payload is the a pointer to a vector space object. On systems with an address bus up to 128 bits wide, it's simply the address of the vector; on systems with an address bus wider than 128 bits, it's probably an offset into an indirection table, but that really is a problem for another day.

As an alternate implementation on hardware with a 64 bit address bus, it might be sensible to have the Car of the VECP cell simply the memory address of the vector, and the Cdr a pointer to the next VECP cell, maintained automatically in the same way that a [free list](Free-list.html) is maintained. This way we automatically hold a list of all live vector space objects, which would help in garbage collecting vector space.

Every object in vector space shall have exactly one VECP cell in cons space which refers to it. Every other object which wished to hold a reference to that object shall hold a cons pointer to VECP cell that points to the object. Each object in vector space shall hold a backpointer to the VECP cell which points to it. This means that if vector space needs to be shuffled in order to free memory, for each object which is moved only one pointer need be updated.

When the reference count of a VECP cell is decremented to zero, the backpointer on the vector to which it points will be set to NIL (zero), marking it as available for garbage collection.

### WRIT

A stream open for writing. The tag value of a WRIT cell is that unsigned 32 bit integer which, when considered as an ASCII string, reads 'WRIT'. The count of a WRIT cell is always non-zero. The mark is up to the garbage collector.

I'm not yet certain what the payload of a WRIT cell is; it is implementation dependent and, at least in version zero, will probably be a file handle from the underlying system.


## Cons pages

Cons cells will be initialised in cons pages. A cons page is a fixed size array of cons cells. Each cell is initialised as FREE, and each cell, as it is initialised, is linked onto the front of the system [free list](Free-list.html). Cons pages will exist in [vector space](Vector-space.html), and consequently each cons page will have a vector space header.