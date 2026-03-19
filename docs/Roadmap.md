# Roadmap

With the release of 0.0.6 close, it's time to look at a plan for the future development of the project.

I have an almost-working Lisp interpreter, which, as an interpreter, has many of the features of the language I want. It runs in one thread on one processor.

Given how experimental this all is, I don't think I need it to be a polished interpreter, and polished it isn't. Lots of things are broken.

* garbage collection is pretty broken, and I'n beginning to doubt my whole garbage collection strategy;
* bignums are horribly broken;
* there's something very broken in shallow-bound symbols, and that matters and wil have to be fixed;
* there are undoubtedly many other bugs I don't know about.

However, while I will fix bugs where I can, it's good enough for other people to play with if they're mad enough, and it's time to move on.

## Next major milestones

### New substrate language?

I really don't feel competent to write the substrate in C, and I don't think 
that what exists of the substrate is of sufficient quality. It's too big and
too complex. I think what the system needs is a smaller substrate written in
a more modern language. 

I propose to evaluate both [Zig](https://ziglang.org/) and 
[Rust](https://rust-lang.org/), and see whether I can feel more productive in
either of those. 

### Smaller substrate

However, I also think the substrate ought to be smaller. I
do not think the substrate should include things like bignum or ratio 
arithmetic, for example. I'm not convinced that it should include things like
hashmaps. If these things are to be written in Lisp, though, it means that 
there have to be Lisp functions which manipulate memory a long way below the
'[don't know, don't care](Post-scarcity-software.md#store-name-and-value)' 
dictum; this means that these functions have to be system private. But they
can be, because access control lists on arbitrary objects have always been
part of this architecture.

### The 0.1.0 branch

I'm therefore proposing, immediately, to upversion the `develop` branch to
0.1.0, and restart pretty much from scratch. For now, the C code will remain in
the development tree, and I may fix bugs which annoy me (and possibly other
people), but I doubt there now will be a 0.0.7 release, unless I decide that
the new substrate languages are a bust. 

So release 0.1.0, which I'll target for 1<sup>st</sup> January 2027, will 
essentially be a Lisp interpreter running on the new substrate and memory
architecture, without any significant new features.

### Simulated hypercube

There is really no point to this whole project while it remains a single thread running on a single processor. Until I can pass off computation to peer neighbours, I can't begin to understand what the right strategies are for when to do so.

`cond` is explicitly sequential, since later clauses should not be executed at all if earlier ones succeed. `progn` is sort of implicitly sequential, since it's the value of the last form in the sequence which will be returned.

For `mapcar`, the right strategy might be to partition the list argument between each of the idle neighbours, and then reassemble the results that come bask.

For most other things, my hunch is that you pass args which are not self-evaluating to idle neighbours, keeping (at least) one on the originating node to work on while they're busy.

But before that can happen, we need a router on each node which can monitor concurrent traffic on six bidirectional links. I think at least initially what gets written across those links is just S-expressions.

I think a working simulated hypercube is the key milestone for version 0.1.1.

### Sysout, sysin, and system persistance

Doctrine is that the post scarcity computing environment doesn't have a file system, but nevertheless we need some way of making an image of a working system so that, after a catastrophic crash or a power outage, it can be brought back up to a known good state. This also really needs to be in 0.1.1. 

### Better command line experience

The current command line experience is embarrassingly poor. Recallable input history, input line editing, and a proper structure editor are all things that I will need for my comfort.

### Users, groups and ACLs

Allowing multiple users to work together within the same post scarcity computing environment while retaining security and privacy is a major goal. So working out ways for users to sign on and be authenticated, and to configure their own environment, and to set up their own access control lists on objects they create, needs to be another nearish term goal. Probably 0.1.2.

### Homogeneities, regularities, slots, migration, permeability

There are a lot of good ideas about the categorisation and organisation of data which are sketched in my original [Post scarcity software](Post-scarcity-software.md) essay which I've never really developed further because I didn't have the right software environment for them, which now I shall have. It would be good to build them.

### Compiler

I do want this system to have a compiler. I do want compiled functions to be the default. And I do want to understand how to write my own compiler for a system like this. But until I know what the processor architecture of the system I'm targetting is, worrying too much about a compiler seems premature.

### Graphical User Interface

Ultimately I want a graphical user interface at least as fluid and flexible as what we had on Interlisp machines 40 years ago. It's not a near term goal there.

### Real hardware

This machine would be **very** expensive to build, and there's no way I'm ever going to afford more than a sixty-four node machine. But it would be nice to have software which would run effectively on a four billion node machine, if one could ever be built. I think that has to be the target for version 1.0.0.