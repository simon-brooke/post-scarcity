# On the topology of the hardware of the deep future

![HAL 9000 - a vision of the hardware of the deep future](https://vignette4.wikia.nocookie.net/2001/images/5/59/Hal_console.jpg/revision/latest?cb=20090823025755)

In thinking about how to write a software architecture that won't quickly become obsolescent, I find that I'm thinking increasingly about the hardware on which it will run.

In [Post Scarcity Hardware](Post-scarcity-hardware.html) I envisaged a single privileged node which managed main memory. Since then I've come to thing that this is a brittle design which will lead to bottle necks, and that each cons page will be managed by a separate node. So there needs to be a hardware architecture which provides the shortest possible paths between nodes.

Well, actually... from a software point of view it doesn't matter. From a software point of view, provided it's possible for any node to request a memory item from any other node, that's enough, and, for the software to run (slowly), a linear serial bus would do. But part of the point of this thinking is to design hardware which is orders of magnitude faster than the [von Neumann architecture](https://en.wikipedia.org/wiki/Von_Neumann_architecture) allows. So for performance, cutting the number of hops to a minimum is important.

I've been reading Danny Hillis' [thesis](https://dspace.mit.edu/bitstream/handle/1721.1/14719/18524280-MIT.pdf?sequence=2) and his book [The Connection Machine](https://books.google.co.uk/books/about/The_Connection_Machine.html?id=xg_yaoC6CNEC&redir_esc=y&hl=en) which, it transpires, is closely based on it. Danny Hillis was essentially trying to do what I am trying to do, but forty years ago, with the hardware limitations of forty years ago (but he was trying to do it in the right place, and with a useful amount of money that actually allowed him to build something physical, which I'm never likely to have).

Hillis' solution to the topology problem, as I understand it (and note - I may not understand it very well) is as follows:

![Second generation connection machine in use](http://www.digibarn.com/collections/parts/connection-machine-chip/cm2-500.jpg)

If you take a square grid and place a processor at every intersection, it has at most four proximal neighbours, and, for a grid which is `x` cells in each direction, the longest path between two cells is `2x`. If you join the nodes on the left hand edge of the grid to the corresponding nodes on the right hand edge, you have a cylinder, and the longest path between two nodes is 1.5x. If you then join the nodes on the top of the grid to the nodes on the bottom, you have a torus - a figure like a doughnut or a bagel. Every single node has four proximal neighbours, and the longest path between any two nodes is `x`.

So far so good. Now, let's take square grids and stack them. This gives each node at most six proximal neighbours. We form a cube, and the longest distance between two nodes is `3x`. We can link the nodes on the left of the cube to the corresponding nodes on the right and form a (thick walled) cylinder, and the longest distance between two nodes is `2.5x`. Now join the nodes at the top of the cube to the corresponding nodes at the bottom, and we have a thick walled torus. The maximum distance between is now `2x`.

Let's stop for a moment and think about the difference between logical and physical topology. Suppose we have a printed circuit board with 199 processors on it in a regular grid. We probably could physically bend the circuit board to form a cylinder, but there's no need to do so. We achieve exactly the same connection architecture simply by using wires to connect the left side to the right. And if we use wires to connect those at the top with those at the bottom, we've formed a logical torus even though the board is still flat.

It doesn't even need to be a square board. We could have each processor on a separate board in a rack, with each board having four connectors probably all along the same edge, and use patch wires to connect the boards together into a logical torus.

So when we're converting our cube into a torus, the 'cube' *could* consist of a vertical stack of square boards each of which has a grid of processors on it. But it could also consist of a stack of boards in a rack, each of which has six connections, patched together to form the logical thick-walled torus. So now lets take additional patch leads and join the nodes that had been on the front of the logical cube to the corresponding nodes on the back of the logical cube, and we have a topology which has some of the properties of a torus and some of the properties of a sphere, and is just mind-bending if you try to visualise it.

This shape is what I believe Hillis means by a [hypercube](https://en.wikipedia.org/wiki/Hypercube), although I have to say I've never found any of the visualisations of a hypercube in books or on the net at all helpful, and they certainly don't resemble the torusy-spherey thing I which visualise.

It has the very useful property, however, that the longest distance between any two nodes is `1.5x`.

Why is `1.5x` on the hypercube better than `1x` on the torus? Suppose you want to build a machine with about 1000 nodes. The square root of a thousand is just less than 32, so let's throw in an extra 24 nodes to make it a round 32. We can lay out 1024 nodes on a 32 x 32 square, join left to right, top to bottom, and we have a maximum path between two of 1024 nodes of 32 hops. Suppose instead we arrange our processors on ten boards each ten by ten, with vertical wires connecting each processor with the one above it and the one below it, as well tracks on the board linking each with those east, west, north and south. Connect the left hand side to the right, the front to the back and the top to the bottom, and we have a maximum path between any two of 1000 nodes of fifteen hops. That's twice as good.

Obviously, if you increase the number of interconnectors to each processor above six, the paths shorten further but the logical topology becomes even harder to visualise. This doesn't matter - it doesn't actually have to be visualised - but wiring would become a nightmare.

I've been thinking today about topologies which would allow higher numbers of connections and thus shorter paths, and I've come to this tentative conclusion.

I can imagine topologies which tesselate triangle-tetrahedron-hypertetrahedron and pentagon-dodecahedron-hyperdodecahedron. There are possibly others. But the square-cube-hypercube model has one important property that those others don't (or, at least, it isn't obvious to me that they do). In the square-cube-hypercube model, every node can be addressed by a fixed number of coordinates, and the shortest path from any node to any other is absolutely trivial to compute.

From this I conclude that the engineers who went before me - and who were a lot more thoughtful and expert than I am - were probably right: the square-cube-hypercube model, specifically toruses and hypercubes, is the right way to go.