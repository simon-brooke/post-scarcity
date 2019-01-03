# All integers are potentially bignums

Each integer comprises at least one cell of type INTR, holding a signed 64 bit integer with a value in the range 0 ... MAX-INTEGER, where the actual value of MAX-INTEGER does not need to be the same as the C language LONG\_MAX, provided that it is less than this. It seems to me that a convenient number would be the largest number less than LONG\_MAX which has all bits set

LONG\_MAX is 0x7FFFFFFFFFFFFFFF, so the number we're looking for is 0x0FFFFFFFFFFFFFFF, which is 1,152,921,504,606,846,975, which is 2^60 - 1. This means we can use bit masking with 0xFFFFFFFFFFFFFFF to extract the part of **int64_t** which will fit in a single cell.

It also means that if we multiply two **int64_t**s into an **__int128_t**, we can then right-shift by 60 places to get the carry.
