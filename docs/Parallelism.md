# Parallelism

If this system doesn't make reasonably efficient use of massively parallel processors, it's failed. The sketch hardware for which it's designed is [[Post Scarcity Hardware]]; that system probably won't ever exist but systems somewhat like it almost certainly will, because we're up against the physical limits on the performance of a von Neumann machine, and the only way we can increase performance now is by going increasingly parallel.

So on such a system, every function invocation may normally delegate every argument to a different processor, if there is another processor free (which there normally will be). Only special forms, like *cond*, which implement explicit flow control, should serialise evaluation.

Therefore the semantics of every function must assume that the order of the evaluation of arguments is undetermined, and that the environment in which each argument is evaluated cannot be influenced by the evaluation of any other argument. Where the semantics of a function violate this expectation, this must be made explicit in the documentation and there should probably be a naming convention indicating this also.

This means, for example, that `and` and `or` cannot be used for flow of control.

## Determining when to hand off computation of an argument to another node

There's obviously a cost of transferring an argument to another node. The argument must be passed to the peer, and, if the argument points to something which isn't already in the peer's cache, the peer will need to fetch that something; and finally, the peer will have to pass the result back.

Communications links to other nodes will be of varying speeds. Communication with a peer which is on the same piece of silicon will be fast, with a peer accessed over a parallel slower, with a peer accessed over a serial bus slower still. So one of the things each node must do in boot is to handshake with each of its proximal neighbours and determine the connection speed. This gives us a feeling for the cost of handoff.

The other variable is the cost of computation.

Suppose we are evaluating 

```
(add 2 2)
```

Then there's clearly no point in handing anything off to a peer, since the arguments each evaluate to themselves.

If we're evaluating

```
(add (cube-root pi) (multiply 2 2) (factorial 1000))
```

Then clearly the first and third arguments are going to be costly to compute. As a programmer, I can see that by inspection; the goal has to be for the compiler to be able to assign functions to cost classes.

If the node which starts the evaluation evaluates the most costly argument itself, then it's reasonable to suppose that by the time it's finished that the neighbours to whom it handed off the other two will have completed and returned their results (or will do so sooner than the original node could compute them). So the heuristics seem to be

1. Don't hand off anything which evaluates to itself;
2. Don't hand off the argument the compiler predicts will be most expensive;
3. Only hand off to a slow to reach neighbour arguments the compiler predicts will be very expensive.

Note that there will be different costs here depending whether we have one memory map per node (i.e. each node curates one cons page and its own vector space, and must copy objects from other nodes into its own space before it can compute on them), or one memory map per crystal, in which case nodes on the same crystal do not need to copy data across.

Copying data, especially if it comes from a node further than a proximal neighbour, will be extremely expensive.