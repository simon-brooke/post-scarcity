# Design decisions for 0.1.0

This is a document that is likely to be revisited, probably frequently.

## Retire the 0.0.X codebase

Move the existing codebase out of the compile space altogether; it is to be 
treated as a finished rapid prototype, not extended further, and code largely
not copied but learned from.

## Remain open to new substrate languages, but continue in C for now

I'm disappointed with [Zig](https://ziglang.org/). While the language 
concepts are beautiful, and if it were stable it would be an excellent tool, it
isn't stable. I'm still open to build some of the 0.1.X prototype in Zig, but 
it isn't the main tool.

I haven't yet evaluated [Nim](https://nim-lang.org/). I'm prejudiced against 
its syntax, but, again, I'm open to using it for some of this prototype.

But for now, I will continue to work in C.

## Substrate is shallow

In the 0.0.X prototype, I tried to do too much in the substrate. I tried to 
write bignums in C, and in this I failed; I would have done much better to 
get a very small Lisp working well sooner, and build new features in that.

In 0.1.X the substrate will be much less feature rich, but support the creation
of novel types of data object in Lisp.

## Paged Space Objects

Paged space objects will be implemented largely in line with [this document](Paged-space-objects.md).

## Tags

Tags will continue to be 32 bit objects, which can be considered as unsigned 
integer values or as four bytes. However, only the first three bytes will be 
mnemonic. The fourth byte will indicate the size class of the object; where 
the size class represents the allocation size, *not* the payload size. The 
encoding is as in this table:

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

Consequently, an object of size class F will have an allocation size of 32,768 words, but a payload size of 32,766 words. This obviously means that size classes 0 and 1 will not exist, since they would not have any payload.

## Page size

Every page will be 1,048,576 bytes.

## Namespaces

Namespaces will be implemented; in addition to the root namespace, there will be at least the following namespaces:

### :bootstrap

Functions written in the substrate language, intended to be replaced for all normal purposes by functions written in Lisp which may call these bootstrap functions. Not ever available to user code.

### :substrate

Functions written in the substrate language which *may* be available to user-written code.

### :system

Functions, written either in Lisp or in the substrate language, which modify system memory in ways that only trusted and privileged users are permitted to do.

## Access control

Obviously, for this to work, access control lists must be implemented and must work.