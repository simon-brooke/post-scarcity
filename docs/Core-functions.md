# Core functions

In the specifications that follow, a word in all upper case refers to a tag value, defined on either the [cons space](Cons-space.html) or the [vector space](Vector-space.html) page.

# (and args...)

Public. Takes an arbitrary number of arguments. Returns true if all are readable by the current user and evaluate to non-NIL. _Note that evaluation of args may be parallelised across a number of processors, so you *cannot* use this for flow control._ 

# (atom? arg)

Public. Takes one argument. Returns TRUE if that argument is neither a CONS cell, a VECP, nor a STRG cell, else NIL.

# (append args...)

Public. Takes an arbitrary number of arguments, which should either all be CONS cells or all STRG cells. In either case returns a concatenation of all those arguments readable by the current user.

# (assoc key store)

Public. Takes two arguments, a key and a store. The store may either be a CONS forming the head of a list formatted as an [assoc list](Assoc-list.html), or else a VECP pointing to a HASH. If the key is readable by the current user, returns the value associated with that key in the store, if it exists and is readable by the current user, else NIL.

# (car arg)

Public. Takes one argument. If that argument is a CONS cell and is readable by the current user, returns the value indicated by the first pointer of that cell; if the argument is an STRG and is readable by the user, a CHAR representing the first character in the string; else NIL. 

# (cdr arg)

Public. Takes one argument. If that argument is a CONS or STRG cell and is readable by the current user, returns the value indicated by the second pointer of that cell, else NIL.

# (cond args...)

Public. Takes an arbitrary number of arguments each of which are lists. The arguments are examined in turn until the first element of an argument evaluates to non-nil; then each of the remaining elements of that argument are evaluated in turn and the value of the last element returned. If no argument has a first element which evaluates to true, returns NIL. _Note: this is explicit flow control and clauses will not be evaluated in parallel._

# (cons a d)

Public. Takes two arguments, A, D. Returns a newly allocated cons cell whose first pointer points to A and whose second points to D.

# (eq? a b)

Public. Takes two arguments, A, B. Returns TRUE if both are readable by the current user and are the same cons space object (i.e. pointer equality), else NIL.

# (eval arg)

Public. Takes one argument.
* if that argument is not readable by the current user, returns NIL.
* if that argument is a CONS, returns the result of 
    (apply (car arg) (cdr arg))
* if that argument is an INTR, NIL, REAL, STRG, TRUE or VECP, returns the argument.
* if that argument is a READ or a WRIT, _probably_ returns the argument but I'm not yet certain.

# (intern str), (intern str namespace)

Public. 
* If one argument, being a STRG readable by the current user, interns that string as a symbol in the current namespace (by binding it to a special symbol *sys-intern*, which has its access control set NIL).
* if two arguments, being a STRG and a VECP pointing to a HASH, interns that string in the specified hash.

_Note: I'm not sure what happens if the STRG is already bound in the HASH. A normal everyday HASH ought to be immutable, but namespaces can't be immutable or else we cannot create new stuff._

# (lambda args forms...)

Public. Takes an arbitrary number of arguments. Considers the first argument ('args') as a set of formal parameters, and returns a function composed of the forms with those parameters bound. Where I say 'returns a function', this is in initial prototyping probably an interpreted function (i.e. a code tree implemented as an S-expression), but in a usable version will mean a VECP (see [cons space](Cons-space.html#VECP)) pointing to an EXEC (see [vector space#EXEC](Vector-space.html#EXEC)) vector.

# (nil? arg)

Public. Takes one argument. Returns TRUE if the argument is NIL. May also return TRUE if the argument is not readable by the current user (on the basis that what you're not entitled to read should appear not to exist) but this needs more thought.

# (not arg)

Public. Takes one argument. Returns TRUE if that argument is NIL, else NIL. _Note: Not sure what happens when the argument is not NIL but not readable by the current user. If we return NIL, as we usually do for an unreadable argument, then that's a clue that the object exists but is not readable. Generally, when an object is not readable, it appears as though it doesn't exist._

# (number? arg)

Public. Takes one argument. Returns TRUE if the argument is readable by the current user and is an INTR, REAL, or some other sort of number I haven't specified yet.

# (or args...)

Public. Takes an arbitrary number of arguments. Returns TRUE if at least one argument is readable by the current user and evaluates to non-NIL. _Note that evaluation of args may be parallelised across a number of processors, so you *cannot* use this for flow control._ 

# (print arg write-stream)

Public. Takes two arguments, the second of which must be a WRIT that the current user has access to. Writes the canonical printed form of the first argument to the second argument.

# (quote arg)

Public. Takes one argument. Returns that argument, protecting it from evaluation.

# (read arg)

Public. Takes one argument. If that argument is either an STRG or a READ, parses successive characters from that argument to construct an S-expression in the current environment and returns it.

# (type arg)

Public. Takes one argument. If that argument is readable by the current user, returns a string interned in the *core.types* namespace representing the tag value of the argument, unless the argument is a VECP in which case the value returned represents the tag value of the [vector space](Vector-space.html) object indicated by the VECP.

