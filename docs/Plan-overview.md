# Plan overview

1. Specify enough of memory arrangement and core functions that I can build something that can start, read (parse) a stream, allocate some memory, print what it's got, and exit. *(achieved)*
2. Build the above. *(achieved)*
3. Fully specify eval/apply, lambda, and def; make a decision whether to eval compiled code, interpreted code, or (at this stage) both. In the long term I want to eval compiled code only, but that requires working out how to generate the right code! *(achieved)*
4. Build enough of the system that I can write and evaluate new functions. *(achieved)*
5. Build enough of the system that two users can log in simultaneously with different identities, so as to be able to test user separation and privacy, but also sharing.
6. Try seriously to get compilation working.
7. Get system persistence working.
8. Get an emulated multi-node system working.
9. Get a bare-metal node working.
10. Get a hypercube (even if of only 2x2x2 nodes) working.
