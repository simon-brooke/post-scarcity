# Change log

## Version 0.0.6

The **MY MONSTER, SHE LIVES** release. But also, the *pretend the problems aren't there* release.

You can hack on this. It mostly doesn't blow up. Bignum arithmetic is broken, but doesn't either segfault or go into non-terminating guru meditations. A lot of garbage isn't getting collected and probably in a long session you will run out of memory, but I haven't yet really characterised how bad this problem is. Subtraction of rationals is broken, which is probably a shallow bug. Map equality is broken, which is also probably fixable.

### There is no hypercube

The hypercube router is not yet written. That is the next major milestone, although it will be for a simulated hypercube running on a single conventional UN*X machine rather than for an actual hardware hypercube.

### There is no compiler

No compiler has been written. That's partly because I don't really know how to write a computer, but it's also because I don't yet know what processor architecture the compiler needs to target.

### There's not much user interface

The user interface is just a very basic REPL. You can't currently even persist your session. You can't edit the input line. You can't save or load files. There is no editor and no debugger. There's certainly no graphics. Exit the REPL by typing [ctrl]-D.

### So what is there?

However, there is a basic Lisp environment in which you can write and evaluate functions. It's not as good as any fully developed Lisp, you won't want to use this for anything at all yet except just experimenting with it and perhaps hacking on it.

### Unit tests known to fail at this release

Broadly, all the bignum unit tests fail. There are major problems in the bignum subsystem, which I'm ashamed of but I'm stuck on, and rather than bashing my head on a problem on which I was making no progress I've decided to leave that for now and concentrate on other things.

Apart from the bignum tests, the following unit tests fail:

| Test | Comment |
| ---- | ------- |
| unit-tests/equal.sh: maps... Fail: expected 't', got 'nil' | Maps in which the same keys have the same values should be equal. Currently they're not. This is a bug. It will be fixed |
| unit-tests/memory.sh => Fail: expected '7106', got '54' | Memory which should be being recovered currently isn't, and this is a major issue. It may mean my garbage collection strategy is fundamentally flawed and may have to be replaced. |
| unit-tests/subtract.sh: (- 4/5 5)... Fail: expected '-3/5', got '3/5' | Subtraction of rational numbers is failing. This is a bug. It will be fixed. |

There are probably many other bugs. If you find them, please report them [here]()