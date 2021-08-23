# Post Scarcity Software System, version 0

tl,dr: look at the [wiki](wiki).

## State of play

### Version 0.0.4

Has working rational number arithmetic, as well as integer and real number arithmetic. The stack is now in vector space, but vector space is not yet properly garbage collected. `defun` does not yet work, so although Lisp functions can be defined the syntax is pretty clunky. So you *can* start to do things with this, but you should probably wait for at least a 0.1.0 release!

## Introduction

Long ago when the world was young, I worked on Xerox Dandelion and Daybreak machines which ran Interlisp-D, and Acorn Cambridge Workstation and Archimedes machines which ran Cambridge Lisp (derived from Portable Standard Lisp). At the same time, Lisp Machines Inc, Symbolics, Thinking Machines, Texas Instruments and probably various other companies I've either forgotten or didn't know about built other varieties of dedicated Lisp machines which ran Lisp right down to the metal, with no operating system under them. Those machines were not only far superior to any other contemporary machines; they were also far superior to any machines we've built since. But they were expensive, and UNIX machines with the same raw compute power became very much cheaper; and so they died.

But in the meantime hardware has become vastly more powerful while software has hardly advanced at all. We don't have software which will run efficiently on the machines of the future, we don't have tools to build it, and it often seems to me we're not even thinking about it.

Ten years ago I wrote [an essay](http://blog.journeyman.cc/2006/02/post-scarcity-software.html) on what software would look like if we treated our computers as though their power was unlimited (which, compared to what we had at the start of my career, it pretty much is); two years ago I wrote about the [hardware architecture](http://blog.journeyman.cc/2014/10/post-scarcity-hardware.html) which might in future support that hardware.

What I'm trying to do now is write a detailed low level specification of the underpinnings of the software system, and begin a trial implementation. Version 0 will run as a user-space program on UNIX, but very much with the intention that a later version will run on top of either a micro-kernel or perhaps even just a BIOS. However I've no real plans to build post scarcity hardware - I lack the skills. What I'm aiming for is to be able to run on 64 bit, multiple processor hardware.

Although I describe it as a 'Lisp environment', for reasons explained in Post Scarcity Software that doesn't mean you will program it in Lisp. It means that the underlying representation of things in the system is Lispy, not Unixy.

## Bindings currently available

The following symbols are bound in the bootstrap layer. It is anticipated that

1. Most of the functions will be overridden by versions of the same function written in Lisp; but
2. these implementations will remain available in the package `/:bootstrap`.

### Values

Note that symbols delimited by asterisks, as in `*in*`, invite rebinding; it is expected, for example, that users will want to rebind input and output streams in their current environment. Rebinding some other symbols, for example `nil`, is unwise.

#### nil

The canonical empty list.

#### t

The canonical true value.

#### \*in\*

The input stream.

#### \*out\*

The output stream.

#### \*log\*

The logging stream (equivalent to `stderr`).

#### \*sink\*

The sink stream (equivalent to `/dev/null`).

####  \*prompt\*

The REPL prompt.

#### (absolute *n*)

Return the absolute value of a number.

#### (add *n1* *n2* ...), (+ *n1* *n2* ...)

Return the result of adding together all the (assumed numeric) arguments supplied.

#### (append *s1* *s2*)

Return a new sequence comprising all the elements of *s1* followed by all the elements of *s2*. *s1* and *s2* must be sequences of the same type.  At a later stage this function will accept arbitrary numbers of arguments, but by that time it will be written in Lisp.

#### (apply *f* *s*)

Apply the function *f* to the arguments that form the sequence *s*, and return the result.

#### (assoc *key* *store*)

Return the value associated with *key* in *store*. *key* may be an object of any type, but keywords, symbols and strings are handled most efficiently. *store* may be an [*association list*](#Association_list), or may be a hashmap.

#### (car *s*)

Return the first element of the sequence *s*.

#### (cdr *s*)

Return a sequence of all the elements of the sequence *s* except the first.

#### (close *stream*)

Closes the indicates stream. Returns `nil`.

#### (cons *a* *b*)

Returns a new pair comprising *a* and *b*. If *b* is a list, this has the effect of creating a new list with the element *a* prepended to all the elements of *b*. If *b* is `nil`, this has the effect creating a new list with *a* as the sole element. Otherwise, it just creates a pair.

#### (divide *n1* *n2*), (/ *n1* *n2*)

Divides the number *n1* by the number *n2*. If *n1* and *n2* are both integers, it's likely that the result will be a rational number.

#### (eq *o1* *o2*)

Returns true (`t`) if *o1* and *o2* are identically the same object, else `nil`.

#### (equal *o1* *o2*)

Returns true (`t`) if *o1* and *o2* are structurally identical to one another, else `nil`.

#### (exception *message*)

Throws (returns) an exception, with the specified *message*. Note that it is extremely likely this signature will change.

#### (get-hash *key* *hashmap*)

Like 'assoc', but the store must be a hashmap. Deprecated.

#### (hashmap *n* *f* *store*)

Create a hashmap with *n* buckets, using *f* as its hashing function, and initialised with the key/value pairs from *store*. All arguments are optional; if none are passed, will create an empty hashmap with 32 keys and the default hashing function.

#### (inspect *o*)

Prints detailed structure of the object *o*. Primarily for debugging.

#### (keys *store*)

Returns a list of the keys in *store*, which may be either an [*association list*](#Association_list), or a hashmap.

#### (mapcar *f* *s*)

Applies the function *f* to each element of the sequence *s*, and returns a new sequence of the results.

#### (meta *o*), (metadata *o*)

Returns metadata on *o*.

#### (multiply *n1* *n2* ...), (\*  *n1* *n2* ...)

Returns the product of multiplying together all of its numeric arguments.

#### (negative? n1)

Returns `t` if its argument is a negative number, else `nil`.

#### (oblist)

Returns a sequence of all the names bound in the root of the naming system.

#### (open *url* *read?*)

Opens a stream to the specified *url*. If a second argument is present and is non-`nil`, the stream is opened for reading; otherwise, it's opened for writing.



## License

Copyright © 2017 [Simon Brooke](mailto:simon@journeyman.cc)

Distributed under the terms of the
[GNU General Public License v2](http://www.gnu.org/licenses/gpl-2.0.html)
