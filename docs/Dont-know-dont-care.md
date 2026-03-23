# Don't know, don't care

![The famous XKCD cartoon showing all modern digital infrastructure depending on a single person's spare-time project](https://imgs.xkcd.com/comics/dependency.png)

One of the key design principles of the Post Scarcity computing project since my 2006 essay, [Post Scarcity Software](Post-scarcity-software.md), has been "don't know, don't care."

The reason for this is simple. Modern computing systems are extremely complex. It is impossible for someone to be expert on every component of the system. To produce excellent work, it is necessary to specialise, to avoid being distracted by the necessary intricacies of the things on which your work depends, or of the (not yet conceived) intricacies of the work of other people which will ultimately depend on yours. It is necessary to trust.

Randal Munroe's graphic which I've used to illustrate this essay looks like a joke, but it isn't.

[Daniel Stenberg](https://en.wikipedia.org/wiki/Daniel_Stenberg) lives not in Nebraska, but in Sweden. He wrote what became [libcurl](https://curl.se/) in 1996, not 2003. He is still its primary maintainer. It pretty much is true to say that all modern digital infrastructure depends on it. It is a basic component which fetches data over a broad range of internet protocols, negotiating the appropriate security. There *are* alternatives to libcurl in (some) other software environments, but it is extremely widely used. Because it deals with security, it is critical; any vulnerability in it needs to be fixed quickly, because it has very major impact.

The current [post-scarcity software environment](https://git.journeyman.cc/simon/post-scarcity) depends on libcurl, because of course it does. You certainly use libcurl yourself, even if you don't know it. You probably used it to fetch this document, in order to read it.

I don't need to know the intricacies of URL schemae, or of Internet protocols, or of security, to the level of detail Daniel does. I've never even reviewed his code. I trust him to know what he's doing.

Daniel's not alone, of course. Linus Torvalds wrote Linux in a university dorm room in Finland; now it powers the vast majority of servers on the Internet, and the vast majority of mobile phones in the world, and, quite incidentally, a cheap Chinese camera drone I bought to film bike rides. Linux is now an enormous project with thousands of contributors, but Linus is still the person who holds it together. [Rasmus Lerdorf](https://en.wikipedia.org/wiki/Rasmus_Lerdorf), from Greenland, wrote PHP to run his personal home page (the clue is in the name); Mark Zuckerberg used PHP to write Facebook;  Michel Valdrighi used PHP to write something called b/cafelog, which Matt Mullenweg further developed into WordPress. 

There are thousands of others, of course; and, at the layer of hardware, on which all software depends, there are thousands of others whose names I do not even know. I'm vaguely aware of the architects of the ARM chip, but I had to look them up just now because I couldn't remember their names. I know that the ARM is at least a spiritual descendant of the 6502, but I don't know who designed that or anything of their story; and the antecedents behind that I don't know at all. The people behind all the many other chips which make up a working computer? I know nothing about them.

(In any case, if one seriously wanted to build this thing, it would be better to have custom hardware &mdash; one would probably have to have custom hardware at least for the router &mdash; and if one were to have custom hardware it would be nice if it ran something very close to Lisp right down on the silicon, as the [Symbolics Ivory](https://gwern.net/doc/cs/hardware/1987-baker.pdf) chips did; so you probably wouldn't use ARM cores at all.)

I have met and personally spoken with most of the people behind the Internet protocol stack, but I don't need to have done so in order to use it; and, indeed, the reason that [Jon Postel](https://en.wikipedia.org/wiki/Jon_Postel) bought me a beer was so that he could sit me down and very gently explain how badly I'd misunderstood something.

-----

But this is the point. We don't need to know, or have known, these people to build on their work. We don't have to, and cannot in detail, fully understand their work. There is simply too much of it, its complexity would overwhelm us.

We don't know. We don't care. And that is a protective mechanism, a mechanism which is necessary in order to allow us to focus on our own task, if we are to produce excellent work. If we are to create a meaningful contribution on which the creators of the future can build.

-----

But there is a paradox, here, one of many conceptual paradoxes that I have encountered working on the Post Scarcity project.

I am essentially a philosopher, or possibly a dilettante, rather than an engineer. When  [Danny Hillis](https://longnow.org/people/board/danny0/) came up with the conception of the [Connection Machine](), a machine which is consciously one of the precursors of the post-scarcity project, he sought expert collaborators &mdash; and was so successful in doing so that [he persuaded Richard Feynman to join the project](https://longnow.org/ideas/richard-feynman-and-the-connection-machine/). I haven't recruited any collaborators. I don't have the social skills. And I don't have sufficient confidence that my idea is even good in itself.

In building the first software prototype, I realised that I don't even properly understand what it means to [intern](http://www.ai.mit.edu/projects/iiip/doc/CommonLISP/HyperSpec/Body/fun_intern.html) something. I realised that I still don't understand how in many Common Lisp implementations, for any integer number `n`, `(eq n n)` can return true. I note that in practice it *does*, but I don't understand how it's done.

In the current post scarcity prototype, it *is* true for very small values of `n`, because I cache an array of small positive integers as an optimisation hack to prevent memory churn, but that's very special case and I cannot believe that Common Lisp implementations are doing it for significantly larger numbers of integers. I note that in SBCL, two bignums of equal value are not `eq`, so presumably SBCL is doing some sort of hack similar to mine, but I do not know how it works and I *shouldn't* care.

Platonically, two instances of the same number *should be* the same object; but we do not live in a Platonic world and I don't want to. I'm perfectly happy that `eq` (which should perhaps be renamed `identical?`) should not work for numbers.

What the behaviour is of the functions that we use, at whatever layer in the stack we work, does matter. We do need to know that. But what happens under the surface in order to deliver that behaviour? We don't need to know. We don't need to care. And we shouldn't, because that way leads to runaway recursion: behind every component, there is another component, which makes other compromises with physical matter which make good engineering sense to the people who understand that component well enough to design and to maintain it.

The stack is not of infinite depth, of course. At its base is silicon, and traces of metals on silicon, and the behaviour of electrons as they interact with individual atoms in those traces. That is knowable, in principle, by someone. But there are sufficiently many layers in the stack, and sufficient complexity in each layer, that to have a good, clear, understanding of every layer is beyond the mental capacity of anyone I know, and, I believe, is generally beyond the mental capacity of any single person.

-----

But this is the point. The point is I do need to know, and do need to care, if I am to complete this project on my own; and I don't have sufficient faith in the utility of the project (or my ability to communicate that utility) that I believe that anyone else will ever care enough to contribute to it. 

And I don't have the skills, or the energy, or, indeed, the remaining time, to build any of it excellently. If it is to be built, I need collaborators; but I don't have the social skills to attract collaborators, or probably to work with them; and, actually, if I did have expert collaborators there would probably be no place for me in the project, because I don't have excellence at anything.

-----

I realise that I don't even really understand what a hypercube is. I describe my architecture as a hypercube. It is a cube because it has three axes, even though each of those axes is conceptually circular. Because the axes are circular, the thing can only be approximated in three dimensional space by using links of flexible wire or glass fibres to join things which, in three dimensional topology, cannot otherwise be joined; it is therefore slightly more than three dimensional while being considerably less than four dimensional.

I *think* this is also Hillis' understanding of a hypercube, but I could be wrong on that.

Of course, my architecture could be generalised to have four, or five, or six, or more circular axes

[^1]: Could it? I'm reasonably confident that it could have *six* circular axes, but I cannot picture in my head how the grid intersections of a four-and-a-bit dimensional grid would work.

, and this would result in each node having more immediate neighbours, which would potentially speed up computation by shortening hop paths. But I cannot help feeling that with each additional axis there comes a very substantial increase in the complexity of physically routing the wires, so three-and-a-bit dimensions may be as good as you practically get.

I don't have the mathematical skill to mentally model how a computation would scale through this structure. It's more an 'if I build it I will find out whether this is computationally efficient' than an 'I have a principled idea of why this should be computationally efficient.' Intuitively, it *should be* more efficient than a [von Neumann architecture](https://en.wikipedia.org/wiki/Von_Neumann_architecture), and it's easy to give an account of how it can address (much) more memory than obvious developments of our current architectures. But I don't have a good feel of the actual time cost of copying data hoppity-hop across the structure, or the heuristics of when it will be beneficial to shard a computation between neighbours.

-----

Which brings me back to why I'm doing this. I'm doing it, principally, to quiet the noises in my brain; as an exercise in preventing my propensity for psychiatric melt-down from overwhelming me. It isn't, essentially, well-directed engineering. It is, essentially, self-prescribed therapy. There is no reason why anyone else should be interested.

Which is, actually, rather solipsistic. Not a thought I like!