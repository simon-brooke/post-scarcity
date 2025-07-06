# Vector Space

Vector space is what in conventional computer languages is known as 'the heap'. Because objects allocated in vector space are of variable size, vector space will fragment over time. Objects in vector space will become unreferenced, making them available for garbage collection and reallocation; but ultimately you will arrive at the situation where there are a number of small free spaces in vector space but you need a large one. Therefore there must ultimately be a mark-and-sweep garbage collector for vector space.

To facilitate this, reference to every vector space object will be indirected through exactly one VECP object in [[cons space]]. If a live vector space object has to be moved in memory in order to compact the heap and to allocate a new object, only one pointer need be updated. This saves enormously on mark-and-sweep time, at the expense of a small overhead on access to vector space objects.

Every vector space object must have a header, indicating that it is a vector space object and what sort of a vector space object it is. Each vector space object must have a fixed size, which is declared in its header. Beyond the header, the payload of a vector space object is undetermined.

Note that, if cons-pointers are implemented simply as memory addresses, the cost of moving a cons page becomes huge, so a rational garbage collector would know about cons pages and do everything possible to avoid moving them.

## The header

As each vector space object has an associated VECP object in cons space, a vector space object does not need to contain either a reference count or an access control list. It does need a cons-pointer to its associated VECP object; it does need a tag (actually it doesn't, since we could put all the tags in cons space, but it is convenient for memory allocation debugging that each should have a tag). It's probably convenient for it to have a mark bit, since if garbage collection of vector space is implemented at all it needs to be mark-and-sweep.

So the header looks like this

    +-----+--------------+------+------+--------------+
    | tag | vecp-pointer | size | mark | payload...  /
    +-----+--------------+------+------+------------+

**TODO:** I'm not satisfied with this header design. I think it should be a multiple of 64 bits, so that it is word aligned, for efficiency of fetch. Possibly it would be better to make the *size* field 31 bits with *mark* size one bit, and instead of having the value of *size* being the size of the object in bytes, it should be the size in 64 bit words, even though that makes the maximum allocatable object only 17 gigabytes. It should also be ordered *tag, size, mark, vecp-pointer*, in order to word align the *vecp-pointer* field.
    
### Tag

The tag will be a 32 bit unsigned integer in the same way and for the same reasons that it is in [[cons space]]: i.e., because it will be alternately readable as a four character ASCII string, which will aid memory debugging.

### Vecp-pointer

The vecp pointer is a back pointer to the VECP object in cons space which points to this vector space object. It is, therefore, obviously, the size of a [[cons pointer]], which is to say 64 bits.

### Size

Obviously a single vector space object cannot occupy the whole of memory, since there are other housekeeping things we need to get the system up and running. But there really should not be a reason why a program should not allocate all the remaining available memory as a single object if that's what it wants to do. So the size field should be the width of the address bus of the underlying machine; for the present, 64 bits. The value of the size field will be the whole size, in bytes, of the object including the header.

### Mark

It's probable that in version zero we won't implement garbage collection of vector space. C programs do not normally have any mechanism for compacting their heap; and vector space objects are much less likely than cons space objects to be transient. However, respecting the fact that in the long term we are going to want to be able to compact our vector space, I'll provide a mark field. This really only needs to be one bit, but, again for word alignment, we'll give it a byte.

So the header now looks like this:

    +-----+--------------+------+------+------------------------+
    | 0   | 32           | 96   | 160  | 168  ...(167 + size) /
    | tag | vecp-pointer | size | mark | payload...          /
    +-----+--------------+------+------+--------------------+

#### Alternative mark-bit strategy

A thought which has recently occurred to me is that the mark bit could be the case bit of the least significant byte of the tag. So that if the tag read 'IMAG' (a raster image), for example, when marked it would read 'IMAg'. This saves having a separately allocated mark in the header, but retains debugging clarity.

## Tags

I really don't at this point have any idea what sorts of things we'll want to store in vector space. This is a non-exhaustive list of things I can think of just now.

### BMAP

A bitmap; a monochrome raster; a two dimensional array of bits.

### EXEC

We definitely need chunks of executable code - compiled functions.

### HASH

We definitely need hashtables. A hashtable is implemented as a pointer to a hashing function, and an array of N cons-pointers each of which points to an [[assoc list]] acting as a hash bucket. A hashtable is immutable. Any function which 'adds a new key/value pair to' a hashtable in fact returns a new hashtable containing all the key value bindings from the old one, with the new one added. Any function which 'changes a key/value pair' in a hashtable in fact returns a new value with the same bindings of all the keys except the one which has changed as the old one.

In either case, anything which held a pointer to the old version still sees the old version, which continues to exist until everything which pointed to it has been deallocated. Only things which access the hashtable via a binding in a current namespace will see the new version.

### NMSP

A namespace. A namespace is a hashtable with some extra features. It has a parent pointer: NIL in the case of a namespace which was not created by 'adding to' or 'modifying' a pre-existing one, but where a pre-existing one was acted on, then that pre-existing one. It also must have an additional access control list, for users entitled to create new canonical versions of this namespace.

A lot of thinking needs to be done here. It's tricky. If I get it wrong, the cost to either performance or security or both will be horrible.

### RSTR

A raster; a two dimensional array of 32 bit integers, typically interpreted as RGBA colour values.

### VECT

An actual vector; an array with cells of a fixed type (where, obviously, a cons pointer is one type). Has a finite number of dimensions, but probably not more than 4,294,967,296 will be supported (i.e. 32 bits for `dimensions`).