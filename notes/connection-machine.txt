This document describes my understanding of the connection machine CM-1, and the differences between CM-1 and the architecture I'm starting to envisage.

I'm working from figure 1.8 on page 21, and chapter 4.

CM-1 comprises 64k processor nodes each with 4096 bits(?) of memory. The topology is I think that each processor has six proximal neighbours.

Physically there are 16 processors to a (custom) chip. Each chip has, in addition to the 16 processors arranged in a 4x4 grid, a single router. The processors on a single chip are each connected to their neighbours, so those in the middle of the chip connect to four neighbours, those on the edge three, and those on corners 2; I'm guessing edge processors connect to neighbouring chips, but I haven't yet found that explicitly. There's also a 'cube pin' which is something to do with topology, but there seems to be just one of these per chip rather than the two per processor that would be needed for six-way interconnect?

There are 4096 routers "connected by 24576 bidirectional wires" (page 78). Thus each *router* is directly connected to 6 others - the routers form a spherey-torusy thing, even if the individual processors don't.

There are 4, 4k x 8 bit(?), memory chips associated with each processor chip, and 32 copies of this grouping of five chips (512 processors) per board, called a module. 16 modules are plugged into each backplane, and two backplanes form a 'rack'. Each rack thus has 16k processors, and 4 racks comprise the machine. Total heat dissipation is 12 Kw!

The 4096 bits of memory do not store program, they only store data; so unlike other Lisp systems (and unlike what I'm envisaging), programs are not data.

Instead, there is single a privileged node known as the 'host'. The host is not a node in the array; it's a physically separate machine with a conventional von Neumann architecture. The host broadcasts instructions to all processors; at every clock tick, each processor is performing the same instruction as every other. The clock speed is a surprisingly sedate 4MHz

Each processor has 8 1 bit state flags and 2 1 bit registers. Each instruction causes it to read two bits from local memory, 1 flag, perform an operation on them, and write 1 bit back to memory and on flag. Thus the speed of the machine is actually 1.3 million instructions per second, not 4 million. Each instruction is 53 bits wide, comprising 12 bits each 'A' and 'B' address, 4 bits each read,  write and condition flags, (addressing 16 flags? but I thought he said there were only eight? - no, on page 74 he says 8, but on page 77 he says 16, and lists them), 1 bit condition sense, 8 bits 'memory truth table' (which I think is the op code determining which operation to use to modify memory), 8 bits 'flag truth table (similar, but for flags), 2 bits 'NEWS', which select which of four proximal neighbours to dispatch the result to.

The 'condition flag' and 'condition sense' elements determine whether the processor should execute the instruction at all. If the flag identified by the condition flag address has the same value as the condition sense bit, then the instruction is executed; else it's treated as no-op.

The reason the processors are so small is cost. Hillis preferred more, simpler processors than fewer, more complex ones.

