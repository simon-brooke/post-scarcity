# Post Scarcity Software System, version 0

Very Nearly a Big Lisp Environment

tl,dr: look at the [[wiki]].

## State of play

### Version 0.0.4

Has working rational number arithmetic, as well as integer and real number arithmetic. The stack is now in vector space, but vector space is not yet properly garbage collected. `defun` does not yet work, so although Lisp functions can be defined the syntax is pretty clunky. So you *can* start to do things with this, but you should probably wait for at least a 0.1.0 release!

## Introduction

Long ago when the world was young, I worked on Xerox Dandelion and Daybreak machines which ran Interlisp-D, and Acorn Cambridge Workstation and Archimedes machines which ran Cambridge Lisp (derived from Portable Standard Lisp). At the same time, Lisp Machines Inc, Symbolics, Thinking Machines, Texas Instruments and probably various other companies I've either forgotten or didn't know about built other varieties of dedicated Lisp machines which ran Lisp right down to the metal, with no operating system under them. Those machines were not only far superior to any other contemporary machines; they were also far superior to any machines we've built since. But they were expensive, and UNIX machines with the same raw compute power became very much cheaper; and so they died.

But in the meantime hardware has become vastly more powerful while software has hardly advanced at all. We don't have software which will run efficiently on the machines of the future, we don't have tools to build it, and it often seems to me we're not even thinking about it.

Ten years ago I wrote [an essay](http://blog.journeyman.cc/2006/02/post-scarcity-software.html) on what software would look like if we treated our computers as though their power was unlimited (which, compared to what we had at the start of my career, it pretty much is); two years ago I wrote about the [hardware architecture](http://blog.journeyman.cc/2014/10/post-scarcity-hardware.html) which might in future support that hardware.

What I'm trying to do now is write a detailed low level specification of the underpinnings of the software system, and begin a trial implementation. Version 0 will run as a user-space program on UNIX, but very much with the intention that a later version will run on top of either a micro-kernel or perhaps even just a BIOS. However I've no real plans to build post scarcity hardware - I lack the skills. What I'm aiming for is to be able to run on 64 bit, multiple processor hardware.

Although I describe it as a 'Lisp environment', for reasons explained in Post Scarcity Software that doesn't mean you will program it in Lisp. It means that the underlying representation of things in the system is Lispy, not Unixy.

## License

Copyright © 2017 [Simon Brooke](mailto:simon@journeyman.cc)

Distributed under the terms of the
[GNU General Public License v2](http://www.gnu.org/licenses/gpl-2.0.html)
