# Stack

The C (and I assume but don't know) Rust stack are contiguous blocks of memory which grow down from the top of the virtual memory map allocated by the operating system to the process. The Lisp stack doesn't have to be the same as the C stack and in fact probably cannot be if I want to have multiple Lisp threads running concurrently in the same process.

If the Lisp stack and the implementation language stack are different, then it's more awkward for Lisp to call functions written in the implementation language and vice versa, but not impossible.

Past Lisps have implemented stack as lists and as vectors. Both work. My own guess is that it possibly best to have a standard sized stack frame allocated in vector space, so that each frame is a contiguous block of memory. A stack frame needs to contain parameters, a return pointer, and somewhere the caller will pick up the return value from. I think a stack frame should have the following:

    +-----------------+-----------------+---------------------------------------------------+
    | tag             | 0...31          | 'STCK'                                            |
    +-----------------+-----------------+---------------------------------------------------+
    | vecp-pointer    | 32...95         | cons-pointer to my VECP (or NIL?)                 |
    +-----------------+-----------------+---------------------------------------------------+
    | size            | 96...159        | 77                                                |
    +-----------------+-----------------+---------------------------------------------------+
    | tag             | 160...167       | 0                                                 |
    +-----------------+-----------------+---------------------------------------------------+
    | parameter 1     | 168...231       | cons-pointer to first param                       |
    +-----------------+-----------------+---------------------------------------------------+
    | parameter 2     | 232...295       | cons-pointer to second param                      |
    +-----------------+-----------------+---------------------------------------------------+
    | parameter 3     | 296...359       | cons-pointer to third param                       |
    +-----------------+-----------------+---------------------------------------------------+
    | more params     | 360...423       | cons-pointer to list of further params            |
    +-----------------+-----------------+---------------------------------------------------+
    | return pointer  | 424...487       | memory address of the instruction to return to    |
    +-----------------+-----------------+---------------------------------------------------+
    | return value    | 488...551       | cons pointer to return value                      |
    +-----------------+-----------------+---------------------------------------------------+
    | prior frame ptr | 552...615       | cons-pointer to preceding stack frame VECP        |
    +-----------------+-----------------+---------------------------------------------------+

Note that every argument to a Lisp function must be a [cons space object](Cons-space.html) passed by reference (i.e., a cons pointer). If the actual argument is actually a [vector space](Vector-space.html) object, then what we pass is a reference to the VECP object which references that vector.

I'm not certain we need a prior frame pointer; if we don't, we may not need a VECP pointing to a stack frame, since nothing can point to a stack frame other than the next stack frame(s) up the stack (if we parallelise *map*, *and* and so on) which to implement a multi-thread system we essentially must have, there may  be two or more successor frames to any frame. In fact to use a massively multiprocessor machine efficiently we must normally evaluate each parameter in a separate thread, with only special forms such as *cond* which impose explicit control flow evaluating their clauses serially in a single thread.

*Uhhhmmm... to be able to inspect a stack frame, we will need a pointer to the stack frame. Whether that pointer should be constructed when the stack frame is constructed I don't know. It would be overhead for something which would infrequently be used.*

However, modern systems with small numbers of processors and expensive thread construction and tear-down would perform **terribly** if all parameter evaluation was parallelised, so for now we can't do that, even though the semantics must be such that later we can.
    