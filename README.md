# Post Scarcity Software System, version 0

tl,dr: look at the [wiki](wiki).

## State of play

### Version 0.0.5

Has working Lisp interpreter, more or less complete, with functions and symbols as defined under [[#Bindings currently available]] below. Features include hash maps.

#### Known bugs

At the time of writing, big number arithmetic is completely failing. It has worked in the past, but it doesn't now.

There are ludicrous memory leaks. Essentially the garbage collection strategy isn't yet really working. However, if we are to implement the hypercube architecture in future, a mark and sweep garbage collector will not work, so it's important to get the reference counter working properly.

#### Unknown bugs

There are certainly MANY unknown bugs. Please report those you find.

#### Not yet implemented

1. There is as yet no **compiler**, and indeed it isn't yet certain what a compiler would even mean. Do all nodes in a machine necessarily share the same processor architecture?
2. There's the beginnings of a narrative about how **namespaces** are going to work, but as yet they aren't really implemented.
3. There is as yet no implementation of the concept of **users**. Access Control Lists exist but are not used. Related, there's no concept of a **session**.
4. There is as yet no **multiprocessor architecture**, not even a simulated one. As it is intended that threading will be implemented by handing off parts of a computation to peer processors, this means there no **threads** either.
5. There's no **user interface** beyond a REPL. There isn't even an **editor**, or **history**.
6. **Printing to strings** does not work.
7. The **exception system**, while it does exist, needs to be radically rethought.

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
2. these implementations will remain available in the namespace `/:bootstrap`.

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

### Functions

#### (absolute *n*)

Return the absolute value of a number.

#### (add *n1* *n2* ...), (+ *n1* *n2* ...)

Return the result of adding together all the (assumed numeric) arguments supplied.

#### (append *s1* *s2* ...)

Return a new sequence comprising all the elements of *s1* followed by all the elements of *s2* and so on for an indefinite number of arguments. All arguments must be sequences of the same type.  

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

#### (equal *o1* *o2*), (= *o1* *o2*)

Returns true (`t`) if *o1* and *o2* are structurally identical to one another, else `nil`.

#### (exception *message*)

Throws (returns) an exception, with the specified *message*. Note that this doesn't really work at all well, and that it is extremely likely this signature will change.

#### (get-hash *key* *hashmap*)

Like 'assoc', but the store must be a hashmap. Deprecated.

#### (hashmap *n* *f* *store*)

Create a hashmap with *n* buckets, using *f* as its hashing function, and initialised with the key/value pairs from *store*. All arguments are optional; if none are passed, will create an empty hashmap with 32 keys and the default hashing function.

#### (inspect *o*)

Prints detailed structure of the object *o*. Primarily for debugging.

#### (keys *store*)

Returns a list of the keys in *store*, which may be either an [*association list*](#Association_list), or a hashmap.

#### (let *bindings* *form*...)

Evaluates each of the *forms* in an environment to which ally of these *bindings* have been added. *bindings* must be an [*association list*](#Association_list), and, additionally, all keys in *bindings* must be symbols. Values in the association list will be evaluated before being bound, and this is done sequentially, as in the behaviour of Common Lisp `let*` rather than of Common Lisp `let`.

#### (list *o*...)

Returns a list of the values of all of its arguments in sequence.

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

#### (print *o* [*stream*])

Prints the print-name of object *o* to the output stream which is the value of *stream*, or to the value of \*out\* in the current environment if no *stream* is provided.

#### (put! *map* *key* *value*)

Puts *value* as the value of *key* in hashmap *map*, destructively modifying it, and returns the map. Note that in future this will work only if the current user has write access to the specified map.

#### (put-all! *map* *assoc*)

Puts each (+key* . *value*) pair from the association list *assoc* into this *map*, destructively modifying it, and returns the map. Note that in future this will work only if the current user has write access to the specified map.

#### (read [*stream*])

Reads a single Lisp form from the input stream which is the value of *stream*, or from the value of \*in\* in the current environment if no *stream* is provided.

#### (read-char [*stream*])

Return the next character from the stream indicated by *stream*, or from the value of \*in\* in the current environment if no *stream* is provided; further arguments are ignored.

#### (repl [*prompt* *input* *output*))

Initiate a new Read/Eval/Print loop with this *prompt*, reading from this *input* stream and writing to this *output* stream. All arguments are optional and default sensibly if omitted. TODO: doesn't actually work yet.

#### (reverse *seq*)

Return a new sequence of the same type as *seq*, containing the same elements but in the reverse order.

#### (slurp *in*)

Reads all available characters on input stream *in* into a string, and returns the string.

#### (source *fn*)

Should return the source code of the function or special form *fn*, but as we don't yet
have a compiler, doesn't.

#### (subtract *n1* *n2*), (- *n1* *n2*)

Subtracts the numeric value *n2* from the numeric value *n1*, and returns the difference.

#### (throw *message*)

Throws an exception, with the payload *message*. While *message* is at present most usefully a string, it doesn't have to be. Returns the exception, but as exceptions are handled specially by `eval`, it is returned to the catch block of the nearest `try` expression on the stack.

#### (time [*milliseconds-since-epoch*])

Returns a time object whose value is the specified number of *milliseconds-since-epoch*, where the Post Scarcity Software Environment epoch is 14 billion years prior to the UN*X epoch. If *milliseconds-since-epoch* is not specified, returns a time object representing the UTC time when the function was executed.

#### (type *o*)

Returns a string representing the type -- actually the tag value -- of the object *o*.

### Special forms

#### (cond (test value) ...)

Evaluates a series of *(test value)* clauses in turn until a test returns non-nil, when the corresponding value is returned and further tests are not evaluated. This is the same syntax as Common Lisp's `cond` implementation, and different from Clojure's.

It's conventional in Lisp to have a final clause in a `cond` block with the test `t`; however, since we have keywords which are always truthy, it would be equally valid to use `:else` or `:default` as final fallback tests.

#### (lambda (arg ...) form ...), (&lambda; (arg ...) form ...)

Returns an anonymous fuction which evaluates each of the *form*s sequentially in an environment in which the specified *arg*s are bound, and returns the value of the last such form.

#### (let ((*var* . *val*) ...) form ...)

Evaluates each of these *form*s sequentially in an environment in which each *var* is bound to the respective *val* in the bindings specified, and returns the value of the last form.

#### (nlambda (arg ...) form ...), (n&lambda; (arg ...) form ...)

Returns an anonymous special form which evaluates each of the *form*s sequentially in an environment in which the specified *arg*s are bound, and returns the value of the last such form.

#### (progn *f* ...)

Evaluates each of the forms which are its arguments in turn and returns the value of the last.

#### (quote *o*), '*o*

Returns *o*, unevaluated.

#### (set! *name* *value* [*namespace*])

Sets (destructively modifies) the value of *name* this *value* in the root namespace. The *namespace* argument is currently ignored but in future is anticipated to be a path specification of a namespace to be modified.

#### (try (*form* ...) (*handler* ...))

Attempt to evaluate, sequentially, each of the *form*s in the first sequence, and return the value of the last of them; however, if any of them cause an exception to be thrown, then evaluate sequentially each of the *handler*s in the second sequence.

It is recommended that you structure this as follows:

`lisp
  (try
    (:body
      (print "hello")
      (/ 1 'a)
      (print "goodbye"))
    (:catch
      (print "Well, that failed.")
      5))
`

Here, `:body` and `:catch` are syntactic sugar which will not affect the final value. 

### Type values

The following types are known. Further types can be defined, and ultimately it should be possible to define further types in Lisp, but these are what you have to be going on with. Note that where this documentation differs from `memory/consspaceobject.h`, this documentation is *wrong*.

#### CONS

An ordinary cons cell: that is to say, a pair.

#### EXEP

An exception

#### FREE

An unallocated memory cell. User programs should never see this.

#### FUNC

A primitive or compiled Lisp function \-- one whose arguments are pre-evaluated.

#### HASH

A hash map (in vector space)

#### INTR

An arbitrarily large integer number.

#### KEYW

A keyword - an interned, self-evaluating string.

#### LMBA

A lambda cell. Lambdas are the interpretable (source) versions of functions.

#### LOOP

Internal to the workings of the ••loop** function. User functions should never see this.

#### NIL 

The special cons cell at address {0,0} whose **car** and **cdr** both point to itself. The canonical empty set. Generally, treated as being indicative of falsity.

#### NLMD

An nlambda cell. NLambdas are the interpretable (source) versions of special forms.

#### RTIO

A rational number, stored as pointers two integers representing dividend and divisor respectively.

#### READ

An open read stream.

#### REAL

A real number, represented internally as an IEEE 754-2008 `binary64`.

#### SPFM

A compiled or primitive special form - one whose arguments are not pre-evaluated but passed as provided.

#### STAK

A stack frame. In vector space. 

#### STRG

A string of [UTF-32](https://en.wikipedia.org/wiki/UTF-32) characters, stored as a linked list. Self evaluating.

#### SYMB

A symbol is just like a string except not self-evaluating. Later, there may be some restrictions on what characters are legal in a symbol, but at present there are not.

#### TIME

A time stamp. The epoch for the Post Scarcity Software Environment is 14 billion years before the UN*X epoch, and is chosen as being a reasonable estimate for the birth of the universe, and thus of the start of time.

#### TRUE

The special cell at address {0,1} which is canonically different from NIL.

#### VECP

A pointer to an object in vector space. User functions shouldn't see this, they should see the type of the vector-space object indicated.

#### VECT

A vector of objects. In vector space.

#### WRIT

An open write stream.



## License

Copyright © 2017 [Simon Brooke](mailto:simon@journeyman.cc)

Distributed under the terms of the
[GNU General Public License v2](http://www.gnu.org/licenses/gpl-2.0.html)
