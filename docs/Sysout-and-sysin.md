# Sysout and sysin

We need a mechanism to persist a running system to backing store, and restore it from backing store.

This might, actually, turn out not to be terribly hard, but is potentially horrendous, particularly if we're talking about very large (multi-terabyte) memory images.

If we use paged memory, as many UNIX systems do, then memory pages periodically get written to disk and the sum total of the memory pages on disk represent an image of the state of system memory. The problem with this is that the state of system memory is changing all the time, and if some pages are out of date with respect to others you don't have a consistent image.

However, the most volatile area of memory is at the outer end of [cons space](Cons-space.html), since that is where cons cells are most likely to die and consequently where new cons cells are most likely to be allocated. We could conceivably take advantage of this by maintaining a per-page [free list](Free-list.html), and preferentially allocating from the currently busiest page. Volatility in [vector space](Vector-space.html) is likely to be significantly lower, but significantly more distributed. However, if we stick to the general rule that objects aren't mutable, volatility happens only by allocating new objects or deallocating old ones. So it may be the case that if we make a practice of flushing vector space pages when that page is written to, and flushing the active cons space pages regularly, we may at any time achieve a consistent memory image on disk even if it misses the last few seconds worth of changes in cons space. 

Otherwise it's worth looking at whether we could journal changes between page flushes. This may be reasonably inexpensive.

If none of this works then persisting the system to backing media may mean halting the system, compacting vector space, writing the whole of active memory to a stream, and restarting the system. This is extremely undesirable because it means putting the system offline for a potentially extended period.

-----

Actually, I'm not sure the above works at all. To sysout a running system, you'd have to visit each node in turn and serialise its cons and vector pages. But if the system is still running when you do this, then you would probably end up with an inconsistent sysout. So you'd have to signal all nodes to halt before performing sysout. Further, you could not restore a sysout to a system with a smaller node count, or smaller node memory, to the system dumped.

This is tricky!