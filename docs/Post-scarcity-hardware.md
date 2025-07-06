# Implementing post scarcity hardware

_I wrote this essay in 2014; it was previously published on my blog, [here](https://www.journeyman.cc/blog/posts-output/2017-09-19-implementing-postscarcity-hardware/)_

Eight years ago, I wrote an essay which I called [Post Scarcity Software](Post-scarcity-software.html). It's a good essay; there's a little I'd change about it now - I'd talk more about the benefits of immutability - but on the whole it's the nearest thing to a technical manifesto I have. I've been thinking about it a lot the last few weeks. The axiom on which that essay stands is that modern computers - modern hardware - are tremendously more advanced than modern software systems, and would support much better software systems than we yet seem to have the ambition to create.

That's still true, of course. In fact it's more true now than it was then, because although the pace of hardware change is slowing, the pace of software change is still glacial. So nothing I'm thinking of in terms of post-scarcity computing actually needs new hardware.

Furthermore, I'm a software geek. I know very little about hardware; but I'm very much aware that as parallelism increases, the problems of topology in hardware design get more and more difficult. I've no idea how physically to design the machines I'm thinking of. But nevertheless I have been thinking more and more, recently, about the design of post-scarcity hardware to support post-scarcity software.

And I've been thinking, particularly, about one issue: process spawning on a new processor, on modern hardware, with modern operating systems, is ridiculously expensive.

# A map of the problem

What got me thinking about this was watching the behaviour of the [Clojure](http://clojure.org/) map function on my eight core desktop machine.

Mapping, in a language with immutable data, in inherently parallelisable. There is no possibility of side effects, so there is no particular reason for the computations to be run serially on the same processor. MicroWorld, being a cellular automaton, inherently involves repeatedly mapping a function across a two dimensional array. I was naively pleased that this could take advantage of  my modern hardware - I thought - in a way in which similarly simple programs written in Java couldn't...

...and then was startled to find it didn't. When running, the automaton would camp on a single core, leaving the other seven happily twiddling their thumbs and doing minor Unixy background stuff.

What?

It turns out that Clojure's default *map* function simply serialises iterations in a single process. Why? Well, one finds out when one investigates a bit. Clojure provides two different versions of parallel mapping functions, *pmap* and *clojure.core.reducers/map*. So what happens when you swap *map* for *pmap*? Why, performance improves, and all your available cores get used!

Except...

Performance doesn't actually improve very much. Consider this function, which is the core function of the [MicroWorld](https://www.journeyman.cc/blog/posts-output/2014-08-26-modelling-settlement-with-a-cellular-automaton/) engine:

<pre>
    (defn map-world
      "Apply this `function` to each cell in this `world` to produce a new world.
       the arguments to the function will be the world, the cell, and any
   `   additional-args` supplied. Note that we parallel map over rows but
       just map over cells within a row. That's because it isn't worth starting
       a new thread for each cell, but there may be efficiency gains in
       running rows in parallel."
      ([world function]
        (map-world world function nil))
      ([world function additional-args]
        (into []
           (pmap (fn [row]
                    (into [] (map
                             #(apply function
                                     (cons world (cons % additional-args)))
                             row)))
                  world))))
</pre>

As you see, this maps across a two dimensional array, mapping over each of the rows of the array, and, within each row, mapping over each cell in the row. As you can see, in this current version, I parallel map over the rows but serial map over the cells within a row.

Here's why:

## Hybrid parallel/non-parallel version

This is the current default version. It runs at about 650% processor loading - i.e. it maxes out six cores and does some work on a seventh. The eighth core is doing all the Unix housekeeping.

    (time (def x1 (utils/map-world
        (utils/map-world w heightmap/tag-altitude (list hm))
        heightmap/tag-gradient)))
    "Elapsed time: 24592.327364 msecs"
    #'mw-explore.optimise/x1

## Pure parallel version

Runs at about 690% processor loading - almost fully using seven cores. But, as you can see, fully one third slower.

    (time (def x2 (utils/map-world-p-p
       (utils/map-world-p-p w heightmap/tag-altitude (list hm))
        heightmap/tag-gradient)))
    "Elapsed time: 36762.382725 msecs"
    #'mw-explore.optimise/x2

(For completeness, the *clojure.core.reducers/map* is even slower, so is not discussed in any further detail)

## Non parallel version

Maxes out one single core, takes about 3.6 times as long as the hybrid version. But, in terms of processor cycles, that's a considerable win - because 6.5 cores for 24 seconds is 156 seconds, so there's a 73% overhead in running threads across multiple cores.

    (time (def x2 (utils/map-world-n-n
       (utils/map-world-n-n w heightmap/tag-altitude (list hm))
        heightmap/tag-gradient)))
    "Elapsed time: 88412.883849 msecs"
    #'mw-explore.optimise/x2

Now, I need to say a little more about this. It's obvious that there's a considerable set-up/tear-down cost for threads. The reason I'm using *pmap* for the outer mapping but serial *map* for the inner mapping rather than the other way round is to do more work in each thread.

However, I'm still simple-mindedly parallelising the whole of one map operation and serialising the whole of the other. This particular array is 2048 cells square - so over four million cells in total. But, by parallelising the outer map operation, I'm actually asking the operating system for 2048 threads - far more than there are cores. I have tried to write a version of map using [Runtime.getRuntime().availableProcessors()](http://stackoverflow.com/questions/1980832/java-how-to-scale-threads-according-to-cpu-cores) to find the number of processors I have available, and then partitioned the outer array into that number of partitions and ran the parallel map function over that partitioning:

    (defn adaptive-map 
      "An implementation of `map` which takes note of the number of available cores."
      [fn list]
      (let [cores (.availableProcessors (. Runtime getRuntime ))
            parts (partition-all (/ (count list) cores) list)]
        (apply concat (pmap #(map fn %) parts))))

Sadly, as [A A Milne wrote](http://licoricelaces.livejournal.com/234435.html), 'It's a good sort of brake But it hasn't worked yet.'

But that's not what I came to talk about. I came to talk about the draft...

We are reaching the physical limits of the speed of switching a single processor. That's why our processors now have multiple cores. And they're soon going to have many more cores. Both Oracle ([SPARC](http://www.theregister.co.uk/2014/08/18/oracle_reveals_32core_10_beeellion_transistor_sparc_m7/)) and [ARM](http://www.enterprisetech.com/2014/05/08/arm-server-chips-scale-32-cores-beyond/) are demoing chips with 32 cores, each 64 bits wide, on a single die. [Intel and MIPS are talking about 48 core, 64 bit wide, chips](http://www.cpushack.com/2012/11/18/48-cores-and-beyond-why-more-cores/). A company called [Adapteva is shipping a 64 core by 64 bit chip](http://www.adapteva.com/products/silicon-devices/e64g401/), although I don't know what instruction set family it belongs to. Very soon we will have more; and, even if we don't have more cores on a physical die, we will have motherboards with multiple dies, scaling up the number of processors even further.

# The Challenge

The challenge for software designers - and, specifically, for runtime designers - is to write software which can use these chips reasonably efficiently. But the challenge, it seems to me, for hardware designers, is to design hardware which makes it easy to write software which can use it efficiently.

## Looking for the future in the past, part one

Thinking about this, I have been thinking about the [Connection Machine](http://en.wikipedia.org/wiki/Connection_Machine). I've never really used a Connection Machine, but there was once one in a lab which also contained a Xerox Dandelion I was working on, so I know a little bit about them. A Connection Machine was a massively parallel computer having a very large number - up to 65,536 - of very simple processors (each processor had a register width of one bit). Each processor node had a single LED lamp; when in use, actively computing something, this lamp would be illuminated. So you could see visually how efficient your program was at exploiting the computing resource available.

\[Incidentally while reading up on the Connection Machine I came across this [delightful essay](http://longnow.org/essays/richard-feynman-connection-machine/) on Richard Feynman's involvement in the project - it's of no relevance to my argument here, but nevertheless I commend it to you\]

The machine was programmed in a pure-functional variant of Common Lisp. Unfortunately, I don't know the details of how this worked. As I understand it each processor had its own local memory but there was also a pool of other memory known as 'main RAM'; I'm guessing that each processor's memory was preloaded with a memory image of the complete program to run, so that every processor had local access to all functions; but I don't know this to be true. I don't know how access to main memory was managed, and in particular how contention on access to main memory was managed.

What I do know from reading is that each processor was connected to twenty other processors in a fixed topology known as a hypercube. What I remember from my own observation was that a computation would start with just one or a small number of nodes lit, and flash across the machine as deeply recursive functions exploded from node to node. What I surmise from what I saw is that passing a computation to an unoccupied adjacent node was extremely cheap.

A possibly related machine from the same period which may also be worth studying but about which I know less was the [Meiko Computing Surface](http://www.new-npac.org/projects/cdroms/cewes-1999-06-vol1/nhse/hpccsurvey/orgs/meiko/meiko.html). The Computing Surface was based on the [Transputer T4](http://en.wikipedia.org/wiki/Transputer#T4:_32-bit) processor, a 32 bit processor designed specifically for parallel processing. Each transputer node had its own local store, and very high speed serial links to its four nearest neighbours. As far as I know there was no shared store. The Computing Surface was designed to be programmed in a special purpose language, [Occam](http://en.wikipedia.org/wiki/Occam_(programming_language)). Although I know that Edinburgh University had at one time a Computing Surface with a significant number of nodes, I don't know how many 'a significant number' is. It may have been hundreds of nodes but I'm fairly sure it wasn't thousands. However, each node was of course significantly more powerful than the Connection Machine's one bit nodes.

## A caveat

One of the lessons we learned in those high, far off, arrogant days was that special purpose hardware that could do marvellous things but was expensive lost out to much less capable but cheaper general purpose hardware. There's no point in designing fancy machines unless there's some prospect that they can be mass produced and widely used, because otherwise they will be too expensive to be practical; which presumes not only that they have the potential to be widely used, but also that you (or someone else related to the project) is able to communicate that potential to people with enough money to back the project.

# Hardware for Post Scarcity software

Before going forward with this argument, lets go back. Let's go back to the idea of the Clojure map function. In fact, let's go back to the idea of a function.

If a processor is computing a function, and that function has an argument, then before the function can be computed the value of the argument must be computed; and, as the function cannot be computed until the value of the argument has been computed, there is no point in handing off the processing of the argument to another processor, because the first processor will then necessarily be idle until the value is returned. So it may just as well recurse up the stack itself.

However, if a function has two arguments and values of both must be computed, then if the first processor can hand off processing of one of them to another, similar, processor, potentially the two can be processed in the time in which the original processor could process just one. Provided, that is, that the cost of handing off processing to another processor is substantially less than the cost of evaluating the argument - which is to say, as a general thing, the closer one can get the cost of handing off to another processor to the cost of allocating a stack frame on the current processor, the better. And this is where current-generation hardware is losing out: that cost of handing off is just way too high.

Suppose, then, that our processor is a compute node in a Connection-Machine-like hypercube, able to communicate directly at high speed with twenty close neighbours (I'll come back to this point in detail later). Suppose also that each neighbour-connection has a 'busy' line, which the neighbour raises when it is itself busy. So our processor can see immediately without any need for doing a round-robin which of its neighbours are available to do new work.

Our processor receives a function call with seven arguments, each of which is a further function call. It hands six of these off to idle neighbours, pushes one onto its own local stack, computes it, and recurses back to the original stack frame, waits for the last of the other six to report back a value, and then carries on with its processing.

The fly in the ointment here is memory access. I assume all the processors have significant read-only cache (they don't need read-write cache, we're dealing with immutable data; and they only need a very small amount of scratchpad memory). If all six of the other processors find the data they need (for these purposes the executable function definition is also data) in local cache, all is good, and this will be very fast. But what if all have cache misses, and have to request the data from main memory?

This comes down to topology. I'm not at all clear how you even manage to have twenty separate data channels from a single node. To have a data channel from each node, separately, to main memory simply isn't possible - not if you're dealing with very large numbers of compute nodes. So the data bus has to be literally a bus, available to all nodes simultaneously. Which means, each node that wants some data from main memory must ask for it, and then sit watching the bus, waiting for it to be delivered. Which also means that as data is sent out on the bus, it needs to be tagged with what data it is.

## Looking for the future in the past, part two

In talking about the Connection Machine which lurked in the basement of Logica's central London offices, I mentioned that it lurked in a lab where one of the [Xerox 1108 Dandelions](http://en.wikipedia.org/wiki/Interlisp) I was employed to work on was also located. The Dandelion was an interesting machine in itself. In typical computers - typical modern computers, but also typical computers of thirty years ago - the microcode has virtually the status of hardware. While it may technically be software, it is encoded immutably into the chip when the chip is made, and can never be changed.

The Dandelion and its related machines weren't like that. Physically, the Dandelion was identical to the Star workstations which Xerox then sold for very high end word processing. But it ran different microcode. You could load the microcode; you could even, if you were very daring, write your own microcode. In its Interlisp guise, it had all the core Lisp functions as single opcodes. It had object oriented message passing - with full multiple inheritance and dynamic selector-method resolution - as a single opcode. But it also had another very interesting instruction: [BITBLT](http://en.wikipedia.org/wiki/Bit_blit), or 'Bit Block Transfer'.

This opcode derived from yet another set, that developed for an earlier version of the same processor on which Smalltalk was first implemented. It copied an arbitrary sized block of bits from one location in memory to another location in memory, without having to do any tedious and time consuming messing about with incrementing counters (yes, of course counters were being incremented underneath, but they were in registers only accessible to the the microcode and which ran, I think, significantly faster than the 'main' registers). This highly optimised block transfer routine allowed a rich and responsive WIMP interface on a large bitmapped display on what weren't, underneath it all, actually terribly powerful machines.

## BITBLT for the modern age

Why is BITBLT interesting to us? Well, if we can transfer the contents of only one memory location over the bus in a message, and every message also needs a start-of-message marker and an object reference, then clearly the bus is going to run quite slowly. But if we can say, OK, here's an object which comprises this number of words, coming sequentially after this header, then the amount of overhead to queuing messages on the bus is significantly reduced. But, we need not limit ourselves to outputting as single messages on the bus, data which was contiguous in main memory.

Most of the things which will be requested will be either vectors (yes, Java fans, an object is a vector) or lists. Vectors will normally point to other objects which will be needed at the same time as the vector itself is needed; list structures will almost always do so. Vectors will of course normally be contiguous in memory but the things they point to won't be contiguous with them; lists are from this point of view like structures of linked vectors such that each vector has only two cells.

So we can envisage a bus transfer language which is in itself like a very simple lisp, except decorated with object references. So we might send the list '(1000 (2000) 3000) over the bus as notionally

[ #00001 1000 [ #00002 2000 ] 3000 ]

where '[' represents start-of-object, '#00001' is an object reference, '1000' is a numeric value, and ']' is end-of-object. How exactly is this represented on the bus? I'll come back to that; it isn't the main problem just now.

## Requesting and listening

Each processor can put requests onto the 'address bus'. Because the address bus is available to every processing node, every processing node can listen to it. And consequently every processing node does listen to it, noting every request that passes over the bus in a local request cache, and removing the note when it sees the response come back over the data bus.

When a processing node wants a piece of data, it first checks its local memory to see whether it already has a copy. If it does, fine, it can immediately process it. If not, it checks to see whether the piece of data has already been requested. If it has not, it requests it. Then it waits for it to come up the bus, copies it off into local store and processes it.

That all sounds rather elaborate, doesn't it? An extremely expensive way of accessing shared storage?

Well, actually, no. I think it's not. Let's go back to where we began: to map.

Mapping is a very fundamental computing operation; it's done all the time. Apply this same identical function to these closely related arguments, and return the results.

So, first processor gets the map, and passes a reference to the function and arguments, together with indices indicating which arguments to work on, to each of its unemployed neighbours. One of the neighbours then makes a request for the function and the list of arguments. Each other processor sees the request has been made, so just waits for the results. While waiting, each in this second tier of processors may sub-partition its work block and farm out work to unemployed third tier neighbours, and so on. As the results come back up the bus, each processor takes its local copy and gets on with its partition, finally passing the results back to the neighbour who originally invoked it.

## The memory manager

All this implies that somewhere in the centre of this web, like a fat spider, there must be a single agent which is listening on the address bus for requests for memory objects, and fulfilling those requests by writing the objects out to the data bus. That agent is the memory manager; it could be software running on a dedicated processor, or it could be hardware. It really doesn't matter. It's operating a simple fundamental algorithm, maintaining a garbage collected heap of memory items and responding to requests. It shouldn't be running any 'userspace' code.

Obviously, there has to be some way for processor nodes to signal to the memory manager that they want to store new persistent objects; there needs to be some way of propagating back which objects are still referenced from code which is in play, and which objects are no longer referenced and may be garbage collected. I know I haven't worked out all the details yet. Furthermore, of course, I know that I know virtually nothing about hardware, and have neither the money nor the skills to build this thing, so like my enormous game engine which I really know I'll never finish, it's really more an intellectual exercise than a project.

But... I do think that somewhere in these ideas there are features which would enable us to build higher performance computers which we could actually program, with existing technology. I wouldn't be surprised to see systems fairly like what I'm describing here becoming commonplace within twenty years.

\[Note to self: when I come to rework this essay it would be good to reference [Steele and Sussman, Design of LISP-based Processors](http://repository.readscheme.org/ftp/papers/ai-lab-pubs/AIM-514.pdf).\]