In Lisp, there's an expectation that any object may act as a key in a hash table. What that means, in practice, is that if a list

```lisp
'(foo bar ban)
```

is a key, and we pass a list 

```lisp
'(foo bar ban)
```

as a query, we ought to get the value to which the first instance of `'(foo bar ban)' was bound, even if the two instances are not the same instance. Which means we have to compute the hash value by exploring the whole structure, no matter how deep and convoluted it may be.

## The cost of this

The cost of this, in the [post scarcity software environment](https://git.journeyman.cc/simon/post-scarcity) is potentially enormous: is potentially a blocker which could bring the whole project down. The post-scarcity architecture as currently conceived allows for 2<sup>64</sup> cons cells. Take the most obvious example of a wholly linear, non-branching structure, a string: it would be perverse, but possible to have a single string which occupied the entire address space. 

But to be less perverse, the text of English Wictionary is 3.9 billion words, so it's reasonable to assume that the text of the Encyclopaedia Brittanica is of the same order of magnitude. There are, on average, 4.7 characters in an English word, plus slightly more than one for punctuation, so we can round that up to six. So the string required to store the text of the Encyclopedia Brittanica would be approximately 24 billion characters long; and storing that in a string would not, in the context of the post-scarcity software environment, be utterly perverse.

But the cost of hashing it would be enormous: would be even greater in the hypercube architecture of the proposed [post scarcity hardware](Post-scarcity-hardware.html) than on one with a [von Neumann architecture](https://en.wikipedia.org/wiki/Von_Neumann_architecture), since we cannot even split the string into chunks to take advantage of parallelism (since the string will almost certainly be non-contiguous in memory), so the hash has to be computed in a single thread, and also since all the cells of the string -- inevitably the majority -- not native to the memory map of the processor node calculating the hash have to be fetched hop-de-hop across the lattice of the hypercube, with each hop costing a minimum of six clock ticks (about 260 clock ticks over a serial link).

A fully populated post scarcity hardware implementation -- i.e. one large enough to contain such perverse strings -- would be a hypercube of side 1625, which means the longest path between any pair of nodes is 812 hops, which means the average path is 406 hops. But, one in every four hops, if the machine is built as I currently conceive it, is a serial link. So the average cost of fetching a datum from an arbitrary node is (6 x 3 x (406 / 4)) + (260 x (406 / 4)), which is to say, roughly, 28,700 clock ticks.

So to fetch the whole string takes about 30,000 x 24,000,000,000, or 720,000,000,000,000 clock ticks, which, assuming a 3GHz clock, is about quarter of a million seconds, or three days. 

To make matters worse, suppose we now stored the hash value in a hash table as the value of that string so as to avoid having to compute it again, we could then not ever garbage collect the string, since that hash table would contain a pointer to it.

So clearly, hashing structures when required in the post scarcity software environment just will not work.

## Finding a solution

Necessarily, most data structures in the post scarcity software environment must be immutable, because most of the time we will be operating on copies of them in compute nodes remote from the node to which they are native. Thus, we can compute a hash value when the structure is first created, and cache it on the structure itself.

This option first occurred to me in the special case of string-like-things (strings, atoms, keywords). Because a wide (32 bit UTF) character sits in 32 bits of memory, and a string cell has to sit in the memory footprint of a cons cell, my string payload had 32 unused bits:

```c
/**
 * payload of a string cell. At least at first, only one UTF character will
 * be stored in each cell. The doctrine that 'a symbol is just a string'
 * didn't work; however, the payload of a symbol cell is identical to the
 * payload of a string cell.
 */
struct string_payload {
    wint_t character;           /* the actual character stored in this cell */
    uint32_t padding;           /* unused padding to word-align the cdr */
    struct cons_pointer cdr;
};
```

So it was straightforward to reassign that unused `padding` as a cache for the hash value:

```c
/**
 * payload of a string cell. At least at first, only one UTF character will
 * be stored in each cell. The doctrine that 'a symbol is just a string'
 * didn't work; however, the payload of a symbol or keyword cell is identical
 * to the payload of a string cell, except that a keyword may store a hash
 * of its own value in the padding.
 */
struct string_payload {
    /** the actual character stored in this cell */
    wint_t character;
    /** a hash of the string value, computed at store time. */
    uint32_t hash;
    /** the remainder of the string following this character. */
    struct cons_pointer cdr;
};
```

But new items can be consed onto the front of lists, and that means, in practice, new characters can be consed onto the front of strings, too. What this means is that

```lisp
(hash "the quick brown fox jumped over the lazy dog")
```

and

```lisp
(hash (append "the" "quick brown fox jumped over the lazy dog"))
```

must return the same value. But, obviously, we don't want to have to walk the whole structure to compute the hash, because we cannot know in principle, when passed a string, whether or not it is extremely long.

The answer that occurred to me, for strings, is as follows:

```c
/**
 * Return a hash value for this string like thing.
 * 
 * What's important here is that two strings with the same characters in the
 * same order should have the same hash value, even if one was created using
 * `"foobar"` and the other by `(append "foo" "bar")`. I *think* this function 
 * has that property. I doubt that it's the most efficient hash function to 
 * have that property.
 * 
 * returns 0 for things which are not string like.
 */
uint32_t calculate_hash(wint_t c, struct cons_pointer ptr)
{
    struct cons_space_object *cell = &pointer2cell(ptr);
    uint32_t result = 0;

    switch (cell->tag.value)
    {
    case KEYTV:
    case STRINGTV:
    case SYMBOLTV:
        if (nilp(ptr))
        {
            result = (uint32_t)c;
        }
        else
        {
            result = ((uint32_t)c *
                      cell->payload.string.hash) &
                     0xffffffff;
        }
    }

    return result;
}

```

That is to say, the hash value is the least significant 32 bits of the product of multiplying the hash of the tail of the string by the character code of the character added. This means we have a very small fixed cost -- one 32 bit integer multiplication -- every time a character is added to a string, rather than an enormous cost every time the hash value of a string is required. I *believe* this is a good trade-off, just as I believe using reference counting rather than mark-and-sweep for garbage collection is a good trade-off: it's better, in my opinion, to have steady is slightly slower than optimal performance from the machine than for it to intermittently lock up for no apparent reason.

In any case, what this means is that getting the hash value -- by the standard hashing function -- of a string cell is absurdly cheap: it's just a bit mask on the cell.

Is this a good hash function? Probably not. A hash function should ideally distribute arbitrary values pretty evenly between hash buckets, and this one is probably biased. Perhaps at some stage someone will propose a better one. But in practice, I believe that this will do for now. It is, after all, extremely cheap.

## To generalise, or not to generalise?

Certainly in Clojure practice, keys in hash maps are almost always 'keywords', a particular variety of string-like-thing. Nevertheless, Clojure, like Common Lisp (and many, perhaps all, other lisps) allows, in principal, that any value can be used as a key in a hash map.

The hack outlined above of using 32 bits of previously unused space in the string payload works because there were 32 unused bits in the string payload. While a hash function with similar properties can be imagined for cons cells, there are not currently any unused bits in a cons payload. To add a hash value would require adding more bits to every cons space object.

Our current cons space object is 256 bits:

```
+-----+--------------+----------------+--------------+--------------+
| header                              | payload                     |
+                                     +--------------+--------------+
|                                     | car          | cdr          |
+-----+--------------+----------------+--------------+--------------+
| tag | count / mark | access-control | cons-pointer | cons-pointer |
+-----+--------------+----------------+--------------+--------------+
| 32  | 32           | 64             | 64           | 64           |
+-----+--------------+----------------+--------------+--------------+
```

This means that cells are word aligned in a 64 bit address space, and a perfect fit for a reasonably predictable future general purpose processor architecture with a 256 bit address bus. If we're now going to add more bits for hash cache, it would be perverse to add less than 64 bits because we'd lose word alignment; and it's not particularly likely, I think, that we'll see future processors with a 320 bit address bus.

It would be possible to steal some bits out of either the tag, the count, or both -- 32 bits for the tag, in particular, is absurdly generous, but it does help greatly in debugging.

For now, I'm of a mind to cache hash values only for string-like-things, and, if users want to use other types of values as keys in hash maps, use access-time hashing functions. After all, this works well enough in Common Lisp. 