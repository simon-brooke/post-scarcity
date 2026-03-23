# Paged space objects

*Antecedents for this essay:

1. [Reference counting, and the garbage collection of equal sized objects](https://www.journeyman.cc/blog/posts-output/2013-08-25-reference-counting-and-the-garbage-collection-of-equal-sized-objects/);
2. [Vector space, Pages, Mark-but-don't-sweep, and the world's slowest ever rapid prototype](https://www.journeyman.cc/blog/posts-output/2026-03-13-The-worlds-slowest-ever-rapid-prototype/).

The post-scarcity software environment needs to store data in objects. Much of the data will be in objects which will fit in the memory footpring ot a cons cell, but some won't, and those that won't will be in a variety of sizes. 

Conventionally, operating systems allocate memory as a heap. If you allocate objects of differing sizes from a heap, the heap becoms fragmented, like a [Sierpiński carpet] or [Cantor dust](https://en.wikipedia.org/wiki/Cantor_set#Cantor_dust) &mdash; there are lots of holes in it, but it becomes increasingly difficult to find a hole which will fit anything large.

If we store our objects in containers of standardised sizes, then, for each of those standardised sizes, we can maintain a freelisp of currently unused containers, from which new containers can be allocated. But we still don't want those relatively small objects floating around independently in memory, because we'll still get the fragmentation problem.

This was the initial motivation behind [cons pages](https://www.journeyman.cc/post-scarcity/html/conspage_8h.html#structcons__page). However, quite early in the development of the prototype, it became obvious that we were allocating and deallocating very many stack frames, and many hash tables, neither of which fit in the memory footprint of a cons cell; and that, going forward, it was likely that we would generate many other sorts of larger objects.

My first thought was to generalise the cons page idea, and generate pages of equal sized objects; that is, one set of pages for objects (like cons cells) with a two word payload, one for objects with a four word payload, one for objects with an eight word payload, and so on. The key idea was that each of these pages would be of equal size, so that if, say, we needed to allocate more eight word objects and there was a page for two word objects currently empty, the memory footprint could be reassigned: the hole in the carpet would be the right size.

If we have to allocate an object which needs a five word payload, it will have to be allocated as an eight word object in an eight word object page, which wastes some memory, for the lifetime of that object; but that memory can be efficiently recovered at the end of life, and the heap doesn't fragment. Any page will, at any time, be partly empty, which wastes more memory, but again, that memory can later be efficiently reused.

The potential problem is that you might end up, say, with many pages for two word objects each of which were partly empty, and have nowhere to allocate new eight word objects; and if this does prove in practice to be a problem, then a mark and sweep garbage collector &mdash; something I *really* don't want &mdash; will be needed. But that is not a problem for just now.

## Efficiently allocating pages

I cannot see how we can efficiently manage pages without each page having some housekeeping data, as every other data object in the system must have a header for housekeeping data. It may be that I am just stuck in my thinking and that the header for pages is not needed, but I *think* it is, and I am going to proceed for now as though it were.

The problem here is that, on an essentially binary machine, it makes sense to allocate things in powers of two; and, as that makes sense at the level of allocating objects in pages, so it makes sense at the level of the basic heap allocator. I'm proposing to allocate objects in standardised containers of these payload sizes:

| Tag  |             |     | Size of payload |                 |
| ---- | ----------- | --- | --------------- | --------------- |
| Bits | Field value | Hex | Number of words | Number of bytes |
| ---- | ----------- | --- | --------------- | --------------- |
| 0000 |           0 |   0 |               1 |               8 |
| 0001 |           1 |   1 |               2 |              16 |
| 0010 |           2 |   2 |               4 |              32 |
| 0011 |           3 |   3 |               8 |              64 |
| 0100 |           4 |   4 |              16 |             128 |
| 0101 |           5 |   5 |              32 |             256 |
| 0110 |           6 |   6 |              64 |             512 |
| 0111 |           7 |   7 |             128 |            1024 |
| 1000 |           8 |   8 |             256 |            2048 |
| 1001 |           9 |   9 |             512 |            4096 |
| 1010 |          10 |   A |            1024 |            8192 |
| 1011 |          11 |   B |            2048 |           16384 |
| 1100 |          12 |   C |            4096 |           32768 |
| 1101 |          13 |   D |            8192 |           65536 |
| 1110 |          14 |   E |           16384 |          131072 |
| 1111 |          15 |   F |           32768 |          262144 |

This scheme allows me to store the allocation payload size of an object, and consequently the type of a page intended to store objects of that size, in four bits, which is pretty economic. But it's not nothing, and there's a cost to this. The irreducable minimum size of header that objects in the system need to have &mdash; in my current design &mdash; is two words. So the allocation size of an object with a payload of two words, is four words; but the allocation size of an object with a payload size of thirty two thousand, seven hundred and sixty eight words, is thirty two thousand, seven hundred and seventy words.

Why does that matter?

Well, suppose we allocate pages of a megabyte, and we take out of that megabyte a two word page header. Then we can fit 262,143 objects with a payload size of two into that page, and waste only two words. But we can fit only three objects of size 262,144 into such a page, and we waste 262,138 words, which feels bad.

When I first realised this, I thought, well, the idea was nice, but it doesn't work. There are three potential solutions, each of which feel inelegant to me:

1. We simply ignore the wasted space;
2. Given that the overwhelming majority of objects used by the system, especially of transient objects, will be of payload size two (allocation size four), we fill all 'spare' space in pages with objects of payload size two, and push them all onto the freelist of objects of payload size two;
    (this feels ugly to me because it breaks the idea that all objects on a given page should be of the same size)
3. We treat the size signature of the page &mdash; that four bit value &mdash; as being related not to the payload size of the ojects to be allocated into the page, but to the allocation size; so that cons cells, with a payload size of two and thus an allocation size of four, would be allocated into pages with a size tag of 0001 and not a size tag of 0010; and we store the housekeeping data for the page itself (waves hands vaguely) somewhere else;
    (this feels ugly to me because, for me, the size of an object is its payload size, and I'm deeply bothered by things foating about randomly in memory without identifying information).

There's a wee bit of autistic insistence on order in my design choices there, that I should not get hung up on. Some objects really do need allocation sizes in memory which are powers of two, but most in fact don't. Currently, the only objects which I commonly allocate and deallocate which are not cons-space objects &mdash; not objects with a payload size of two &mdash; are stack frames (current payload size 12) and hash tables (current payload size variable, but defaults to 34). 

If we're storing the (encoded) allocation size of each object in the tag of the object &mdash; which I think that in the 0.1.0 prototype we will, and if every object on any given page is of the same size, which seems to me a good plan, then I'm not sure that we actually need to store any other housekeeping data on the page, because the header of every object is the same size, and the header of every object in the page holds the critical bit of housekeeping information about the page, so we can always get that value from the header of the first object in the page.

If we take these two pragmatic compromises together &mdash; that the size encoded in the tag of an object is its allocation saize not its payload size, and that the allocation size in the first object on a page is the allocation size for that page &mdash; then every page can fit an exact number of objects with no space wasted.

That's not beautiful but I think it's sensible.
