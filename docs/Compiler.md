# Towards a Compiler

Abdulaziz Ghuloum's paper [An Incremental Approach to Compiler Construction](https://bernsteinbear.com/assets/img/11-ghuloum.pdf) starts with the observation:

> Compilers are perceived to be magical artifacts, carefully crafted
> by the wizards, and unfathomable by the mere mortals. Books on
> compilers are better described as wizard-talk: written by and for
> a clique of all-knowing practitioners. Real-life compilers are too
> complex to serve as an educational tool. And the gap between
> real-life compilers and the educational toy compilers is too wide.
> The novice compiler writer stands puzzled facing an impenetrable
> barrier, “better write an interpreter instead.”

Well, yes. That *is* what I feel. But the thing is, I've written two Lisp interpreters (and interpreters for a few other languages into one dialect of Lisp or another) now. I still feel [imposter syndrome](https://en.wikipedia.org/wiki/Impostor_syndrome) &mdash; that my interpreters are not as good as they should be, that I haven't understood the ideas clearly enough or implemented them cleanly enough, but [Beowulf](https://git.journeyman.cc/simon/beowulf) works (and evaluates Lisp) very well; the [`0.0.6` Post Scarcity](https://git.journeyman.cc/simon/post-scarcity) prototype works, after a fashion; and, after only a week of work, the `0.1.0`  Post Scarcity prototype is close to working now.

Further back in my history, the [MicroWorld rule language](https://git.journeyman.cc/simon/mw-parser) is still easily buildable and works well; and, long before that, my LemonADE adventure game writing language did work well; and KnacqTools suite of rule 'compilers,' which although not strictly speaking either interpreters or compilers in this sense were very similar technology, also worked extremely well. Interpreters &mdash; even reasonably good interpreters &mdash; are a done problem, but I have really no idea where to start building a compiler.

So why bother?

Beowulf is *mostly* written in Lisp &mdash; which is to say, it is mostly written in itself. If you check the [list of functions](https://git.journeyman.cc/simon/beowulf#functions-and-symbols-implemented), you'll see that the overwhelming majority of them are described as 'Lisp lambda functions'. This means, they're Beowulf functions written in Beowulf &mdash; and you can read the source code of them [here](https://git.journeyman.cc/simon/beowulf/src/branch/master/resources/lisp1.5.lsp).

But Post Scarcity `0.0.6` is written almost entirely in C. It never got to the point, as Beowulf did, where you could start a Lisp session, hack up a few functions, and save out your system to persistent storage to start again later with the work you'd written already incorporated. And this is mainly because I tried to do too many of the hard parts, like the sophisticated reader and bignum arithmetic, in C. 

I'm not a confident C programmer. Post Scarcity `0.0.6`'s bignum arithmetic doesn't work, and I've failed to make it work. Post Scarcity `0.0.6`'s garbage collector works unacceptably poorly. My goal, in `0.1.0`, is to write far less in the substrate and far more in Lisp.

Which means, the Lisp must be as performant as possible. Which means, I think, that I need a compiler. Which means I need to learn to be (more of a) wizard.

So, where do I start? Where is my grimoire?

## Online tutorials on Lisp compilers

### Ghuloum

I've mentioned Abdulaziz Ghuloum's [An Incremental Approach to Compiler Construction](https://bernsteinbear.com/assets/img/11-ghuloum.pdf) at the top. It's PDF, of course. Why do people publish things as PDF? It makes them *so hard* to read! 

However, I very much like his approach: small incremental steps. He writes mainly in Scheme, which is similar enough to Post Scarcity Lisp that it should be reasonably simple to carry over ideas; he targets what he describes as 'Intel-x86' assembler, but I don't yet know whether that means 16, 32 or 64 bit &mdash; since the paper dates from 2006 I'm guessing 32 bit. However, his method is to write a C fragment that implements a small step of his process, and then examine assembler output from GCC; that's an approach I could follow.

He uses test driven development, which should make things easy to reproduce.

He implements tail-call optimisation.

The paper is quite brief, and does not include source code; I have not found source code relating to it.

The paper contains a link to the author's home page at Indiana.edu, but that link is now dead. Archive.org has snapshots dated from [18th September 2006](https://web.archive.org/web/20060918162504/https://www.cs.indiana.edu/~aghuloum/) (the paper is dated from the 16th) to [March 10th 2011](https://web.archive.org/web/20110310092701/http://www.cs.indiana.edu/~aghuloum/). Although the lecture notes appear in both the listed snapshots, the paper itself is not in the first of them.

Ghuloum appears to have recently been teaching at the American University of Kuwait; he has a [GitHub presence](https://github.com/azizghuloum), but his Scheme compiler is not listed there. He published [a number of technical papers on Scheme](https://scholar.google.com/citations?user=5rd6dWUAAAAJ&hl=en) between 2006 and 2009, but does not appear to have published anything since.

### Healey

This blog post by [Andrew Healey](https://github.com/healeycodes), [Compiling Lisp to Bytecode and Running It](https://healeycodes.com/compiling-lisp-to-bytecode-and-running-it) is essentially 'write your own virtual machine,' which, given that I've been thinking about the ideal instruction set for the Post Scarcity processor, isn't a bad idea. [This repository](https://github.com/healeycodes/lisp-to-js) appears to be his implementation.

His code has virtually no internal documentation, and is in a language I don't even recognise (it might be Rust &mdash; it builds and tests with `cargo`); however, it's clearly written in nice small functions, and there is really surprisingly little of it. It does build, and all its tests pass.

Healey is still active on GitHub, and currently works for Vercel, an 'AI Cloud' company, apparently as a software engineer.

### Bernstein

There's a [blog series](https://bernsteinbear.com/blog/lisp/) by [Max Bernstein](https://github.com/tekknolagi) which is nicely clear. He references Ghuloum's work (and indeed the link I found to Ghuloum's paper is on his site), but builds his compiler in C. His repository for the compiler posts appears to be [this one](https://github.com/tekknolagi/ghuloum).

His code is mainly in C, with a test harness in Python. Again, his code is internally largely undocumented, but builds cleanly, and all his unit tests pass. The way he implements his unit tests is new to me, and worth studying; it's certainly better than the scrappy mess of shell scripts I used for the `0.0.X` series.

### Others

That's the list of things I've found so far that look useful to me. If I find others, I'll add them here.

## Things which inevitably make the Post Scarcity compiler different

### Tag location

Objects in Lisp have to know that they are. This is what makes it possible to compute with an 'untyped' language: the type is not encoded in the program but in the data. In most conventional Lisp systems, things are typed by having a tag. Back in the day, when we had hardware specially built to run Lisp, Lisp specific hardware often had a word size &mdash; and thus registers, and a data bus &mdash; wider than the address bus, wider by the number of bits in the tag, and stored the tag on the pointer.

Modern Lisps still, I think, mostly store the tag on the pointer, but they run on commodity hardware which doesn't have those extra bits in the word size. That means that the size of an integer, or the precision of a real, that you can store in one word of memory is much less. It also means either that they can address much less memory than other programming languages on the same hardware, because for every bit you steal out of the address bus you halve the amount of memory you can address; or else that they bit shift up every address before they fetch it. 

The bit shift works if all memory objects are powers of two words wide, which, in Post Scarcity `0.1.0` they are, see [Paged Space Objects](Paged-space-objects.md); but as I am already doing the upshifting trick so that I can address more than 64 (actually 104, on the current sketch of how memory works) 'bits wide' of memory, this doesn't help me.

Consequently, in both the `0.0.X` series of prototypes and now in the `0.1.0` prototype, I have the tag in the object, not in the pointer.

#### Is that a good decision?

There's a really big inefficiency in that decision. In early versions of Java, numbers (and a few other things) were not objects, but 'primitives'. That is to say, the word of memory which, for objects, would be a pointer, is, for primitives, the actual data; and thus you can operate on it without doing an  additional fetch. In modern Java, those primitives still exist, as [unboxed types](https://en.wikipedia.org/wiki/Boxing_(computer_programming)). Java can do this because it is a typed language. Every method knows the type of its arguments.

In Lisp we don't. So we either have the tag on the pointer, reducing, as I pointed out above, the number of addresses that can be addressed and the amount of data that can be stored in each object, or we have the tag on the object, meaning that (the header of) every object has to be fetched before we even know what it is, and thus how to despatch it further. And, in the Post Scarcity architecture as I conceive it now, in the case of an object which is curated on a node somewhere far distant across the hypercube and not yet in local cache, that means it has to be fetched hoppity hop across the mesh, which is extremely costly.

But, not only does Post Scarcity need a bigger tag than most Lisps in order to have user extensible types, it also needs to have an access control list on every object in order to have security between users; and, although I failed to make the reference counting garbage collector work in `0.0.X`, and although the thinking I've been doing about the 'mark but don't sweep' garbage collector may make it unnecessary, I still want to experiment with reference counting. So I need space in every header for a reference count. 

So I can't really have unboxed objects, I think[^1]  &mdash; at least, allowing unboxed integers, reals, and characters would need a very thorough rethink of the security model.

[^1]: except that, in compiled functions, local variables could potentially be the equivalent of unboxed. That's one of the main speed increases I hope to get from compiling.

All decisions in engineering are compromises. At present, I am content to proceed with this compromise.

### Reifying compiled functions

I don't honestly know where most modern Lisps allocate space for compiled functions, but I suspect that it's on the heap. In the `0.1.0` prototype I'm really trying to limit the use of 'raw' heap allocation, to prevent heap fragmentation, to reduce garbage collection problems. So I want to put each compiled function into a paged space object. Which means they have to be relocatable in memory.

And certainly, when a compiled function is copied from the node on which it is curated to another node where it will be cached, it will be at a different place in the memory of that node.

*(Question: should we copy only source functions across the mesh, and compile them 'just in time' on the node where they will be used? Doing that would allow each compiled function to incorporate raw pointers to every other function it called, which would greatly speed execution. However, if any of those functions were subsequently redefined, it would not update to use the new definition without recompilation.)*

I don't *think* relocatability is a problem. Lisps which use heap-allocated compiled functions and run mark and sweep garbage collectors on their heap, as I'm almost certain Portable Standard Lisp does and imagine most other conventional Lisps must, must have relocatable functions. 

However, it may be. I certainly need to think about relocatability in this design. 

## Conclusion

Post Scarcity's compiler won't be &mdash; can't be &mdash; a straight lift of anyone else's Lisp compiler. Post Scarcity is just inevitably a very different beast. The whole idea of a multiple instruction, multiple data, massively parallel processor is one that has not been very much explored because it is hard; and I don't have the technical or mathematical understanding to demonstrate whether, even if a Post Scarcity machine really could use four billion processor nodes petabytes of memory, it could do so efficiently.

But the compiler is doable; none of the peculiarities of the architecture is a blocker. And even if this won't be a conventional compiler, there is a great deal that can be learned from conventional compilers.