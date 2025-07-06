# Post Scarcity Software Environment: general documentation

Work towards the implementation of a software system like that described in [Post Scarcity Software](http://blog.journeyman.cc/2006/02/post-scarcity-software.html).

## Note on canonicity

*Originally most of this documentation was on a wiki attached to the [GitHub project](https://github.com/simon-brooke/post-scarcity); when that was transferred to [my own foregejo instance](https://git.journeyman.cc/simon/post-scarcity) the wiki was copied. However, it's more convenient to keep documentation in the project with the source files, and version controlled in the same Git repository. So while both wikis still exist, they should no longer be considered canonical. The canonical version is in `/docs`, and is incorporated by [Doxygen](https://www.doxygen.nl/) into the generated documentation &mdash; which is generated into `/doc` using the command `make doc`.*

## AWFUL WARNING 1

This does not work. It isn't likely to work any time soon. If you want to learn Lisp, don't start here; try Clojure, Scheme or Common Lisp (in which case I recommend Steel Bank Common Lisp). If you want to learn how Lisp works, still don't start here. This isn't ever going to be anything like a conventional Lisp environment.

What it sets out to be is a Lisp-like system which:

* Can make use (albeit not, at least at first, very efficiently) of machines with at least [Zettabytes](http://highscalability.com/blog/2012/9/11/how-big-is-a-petabyte-exabyte-zettabyte-or-a-yottabyte.html) of RAM;
* Can make reasonable use of machines with at least tens of thousands of processors;
* Can concurrently support significant numbers of concurrent users, all doing different things, without them ever interfering with one another;
* Can ensure that users cannot escalate privilege;
* Can ensure users private data remains private.

When Linus Torvalds sat down in his bedroom to write Linux, he had something usable in only a few months. BUT:

* Linus was young, energetic, and extremely talented; I am none of those things.
* Linus was trying to build a clone of something which already existed and was known to work. Nothing like what I'm aiming for exists.
* Linus was able to adopt the GNU user space stack. There is no user space stack for this idea; I don't even know what one would look like.

## AWFUL WARNING 2

This project is necessarily experimental and exploratory. I write code, it reveals new problems, I think about them, and I mutate the design. The documentation in this wiki does not always keep up with the developing source code.

