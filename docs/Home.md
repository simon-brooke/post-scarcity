# Post Scarcity Software Environment: general documentation

Work towards the implementation of a software system like that described in [Post Scarcity Software](https://www.journeyman.cc/blog/posts-output/2006-02-20-postscarcity-software/).

## Note on canonicity

*Originally most of this documentation was on a wiki attached to the [GitHub project](https://github.com/simon-brooke/post-scarcity); when that was transferred to [my own foregejo instance](https://git.journeyman.cc/simon/post-scarcity) the wiki was copied. However, it's more convenient to keep documentation in the project with the source files, and version controlled in the same Git repository. So while both wikis still exist, they should no longer be considered canonical. The canonical version is in `/docs`, and is incorporated by [Doxygen](https://www.doxygen.nl/) into the generated documentation &mdash; which is generated into `/doc` using the command `make doc`.*

## State of Play

You can read about the current [state of play](State-of-play.md).

## Roadmap

There is now a [roadmap](Roadmap.md) for the project.

## AWFUL WARNING 1

This does not work. It isn't likely to work any time soon. If you want to learn Lisp, don't start here; try Clojure, Scheme or Common Lisp (in which case I recommend Steel Bank Common Lisp). If you want to learn how Lisp works, still don't start here. This isn't ever going to be anything like a conventional Lisp environment.

What it sets out to be is a Lisp-like system which:

* Can make use (albeit not, at least at first, very efficiently) of machines with at least [Zettabytes](http://highscalability.com/blog/2012/9/11/how-big-is-a-petabyte-exabyte-zettabyte-or-a-yottabyte.html) of RAM;
* Can make reasonable use of machines with at least billions of processors;
* Can concurrently support significant numbers of users, all doing different things, without them ever interfering with one another;
* Can ensure that users cannot escalate privilege;
* Can ensure users private data remains private.

When Linus Torvalds sat down in his bedroom to write Linux, he had something usable in only a few months. BUT:

* Linus was young, energetic, and extremely talented; I am none of those things.
* Linus was trying to build a clone of something which already existed and was known to work. Nothing like what I'm aiming for exists.
* Linus was able to adopt the GNU user space stack. There is no user space stack for this idea; I don't even know what one would look like.

## AWFUL WARNING 2

This project is necessarily experimental and exploratory. I write code, it reveals new problems, I think about them, and I mutate the design. This documentation does not always keep up with the developing source code.

## Building

The substrate of this version is written in plain old fashioned C and built with a Makefile. I regret this decision; I think either Zig or Rust would have been better places to start; but neither of them were sufficiently well developed to support what I wanted to do when I did start.

To build, you need a C compiler; I use GCC, others may work. You need a make utility; I use GNU Make. You need [libcurl](https://curl.se/libcurl/).

With these dependencies in place, clone the repository from [here](https://git.journeyman.cc/simon/post-scarcity/), and run `make` in the resulting project directory. If all goes well you will find and executable, `psse`, in the target directory.

This has been developed on Debian but probably builds on any 64 bit UN*X; however I do **not** guarantee this.

### Make targets

#### default

The default `make` target will produce an executable as `target/psse`.

#### clean

`make clean` will remove all compilation detritus; it will also remove temporary files.

#### doc

`make doc` will generate documentation in the `doc` directory. Depends on `doxygen` being present on your system.

#### format

`make format` will standardise the formay of C code. Depends on the GNU `indent` program being present on your system.

#### REPL

`make repl` will start a read-eval-print loop. `*log*` is directed to `tmp/psse.log`.

#### test

`make test` will run all unit tests.

## In use

What works just now is a not very good, not very efficient Lisp interpreter which does not conform to any existing Lisp standard. You can start a REPL, and you can write and evaluate functions. You can't yet save or load your functions. It's interesting mainly because of its architecture, and where it's intended to go, rather than where it is now.

### Documentation

There is [documentation](https://www.journeyman.cc/post-scarcity/doc/html/).

### Invoking

The binary is canonically named `psse`. When invoking the system, the following invocation arguments may be passed:
```
        -d      Dump memory to standard out at end of run (copious!);
        -h      Print this message and exit;
        -p      Show a prompt (default is no prompt);
        -s LIMIT
                Set a limit to the depth the stack can extend to;
        -v LEVEL
                Set verbosity to the specified level (0...1024)
                Where bits are interpreted as follows:
                1       ALLOC;
                2       ARITH;
                4       BIND;
                8       BOOTSTRAP;
                16      EVAL;
                32      INPUT/OUTPUT;
                64      LAMBDA;
                128     REPL;
                256     STACK;
                512     EQUAL.
```

Note that any verbosity level produces a great deal of output, and although standardising the output to make it more legible is something I'm continually working on, it's still hard to read the output. It is printed to stderr, so can be redirected to a file for later analysis, which is the best plan.

### Functions and symbols

The following functions are provided as of release 0.0.6:

| Symbol | Type | Documentation |
| ------ | ---- | ------------- |
| `*` | FUNC | `(* args...)` Multiplies these `args`, all of which should be numbers, and return the product. |
| `*in*` | READ | The standard input stream. |
| `*log*` | WRIT | The standard logging stream (stderr). |
| `*out*` | WRIT | The standard output stream. |
| + | FUNC | `(+ args...)`: If `args` are all numbers, returns the sum of those numbers. |
| - | FUNC | `(- a b)`: Subtracts `b` from `a` and returns the result. Expects both arguments to be numbers. |
| / | FUNC | `(/ a b)`: Divides `a` by `b` and returns the result. Expects both arguments to be numbers. |
| = | FUNC | `(equal? args...)`: Return `t` if all args have logically equivalent value, else `nil`. |
| absolute | FUNC | `(absolute arg)`: If `arg` is a number, return the absolute value of that number, else `nil`. |
| add | FUNC | `(+ args...)`: If `args` are all numbers, return the sum of those numbers. |
| and | FUNC | `(and args...)`: Return a logical `and` of all the arguments and return `t` only if all are truthy, else `nil`. |
| append | FUNC | `(append args...)`: If `args` are all sequences, return the concatenation of those sequences. |
| apply | FUNC | `(apply f args)`: If `f` is usable as a function, and `args` is a collection, apply `f` to `args` and return the value. |
| assoc | FUNC | `(assoc key store)`: Return the value associated with this `key` in this `store`. |
| car | FUNC | `(car arg)`: If `arg` is a sequence, return the item which is the head of that sequence. |
| cdr | FUNC | `(cdr arg)`: If `arg` is a sequence, return the remainder of that sequence with the first item removed. |
| close | FUNC | `(close stream)`: If `stream` is a stream, close that stream. |
| cond | SPFM | `(cond clauses...)`: Conditional evaluation, `clauses` is a sequence of lists of forms such that if evaluating the first form in any clause returns non-`nil`, the subsequent forms in that clause will be evaluated and the value of the last returned; but any subsequent clauses will not be evaluated. |
| cons | FUNC | `(cons a b)`: Return a cons cell whose `car` is `a` and whose `cdr` is `b`. |
| count | FUNC | `(count s)`: Return the number of items in the sequence `s`. |
| divide | FUNC | `(/ a b)`: If `a` and `b` are both numbers, return the numeric result of dividing `a` by `b`. |
| eq? | FUNC | `(eq? args...)`: Return `t` if all args are the exact same object, else `nil`. |
| equal? | FUNC | `(equal? args...)`: Return `t` if all args have logically equivalent value, else `nil`. |
| eval | FUNC | `(eval form)`: Evaluates `form` and returns the result. |
| exception | FUNC | `(exception message)`: Return (throw) an exception with this `message`. |
| get-hash | FUNC | `(get-hash arg)`: Returns the natural number hash value of `arg`. This is the default hash function used by hashmaps and namespaces, but obviously others can be supplied. |
| hashmap | FUNC | `(hashmap n-buckets hashfn store write-acl)`: Return a new hashmap, with `n-buckets` buckets and this `hashfn`, containing the content of this `store`, and protected by the write access control list `write-acl`. All arguments are optional. The intended difference between a namespace and a hashmap is that a namespace has a write acl and a hashmap doesn't (is not writable), but currently (0.0.6) this functionality is not yet written. |
| inspect | FUNC | `(inspect object ouput-stream)`: Print details of this `object` to this `output-stream`, or `*out*` if no `output-stream` is specified. |
| keys | FUNC | `(keys store)`: Return a list of all keys in this `store`. |
| lambda | SPFM | `(lambda arg-list forms...)`: Construct an interpretable λ funtion. |
| let | SPFM | `(let bindings forms)`: Bind these `bindings`, which should be specified as an association list, into the local environment and evaluate these forms sequentially in that context, returning the value of the last. |
| list | FUNC | `(list args...)`: Return a list of these `args`. |
| mapcar | FUNC | `(mapcar function sequence)`: Apply `function` to each element of `sequence` in turn, and return a sequence of the results. |
| meta | FUNC | `(meta symbol)`: If the binding of `symbol` has metadata, return that metadata, else `nil`. |
| metadata | FUNC | `(metadata symbol)`: If the binding of `symbol` has metadata, return that metadata, else `nil`. |
| multiply | FUNC | `(multiply args...)` Multiply these `args`, all of which should be numbers, and return the product. |
| negative? | FUNC | `(negative? n)`: Return `t` if `n` is a negative number, else `nil`. |
| nlambda | SPFM | `(nlamda arg-list forms...)`: Construct an interpretable special form. When the form is interpreted, arguments specified in the `arg-list` will not be evaluated. |
| not | FUNC | `(not arg)`: Return `t` only if `arg` is `nil`, else `nil`. |
| nλ | SPFM | `(nlamda arg-list forms...)`: Construct an interpretable special form. When the form is interpreted, arguments specified in the `arg-list` will not be evaluated. |
| oblist | FUNC | `(oblist)`: Return the current top-level symbol bindings, as a map. |
| open | FUNC | `(open url write?)`: Open a stream to this `url`. If `write?` is present and is non-nil, open it for writing, else reading. |
| or | FUNC | `(or args...)`: Return a logical `or` of all the arguments and return `t` if any is truthy, else `nil`. |
| print | FUNC | `(print object stream)`: Print `object` to `stream`, if specified, else to `*out*`. |
| progn | SPFM | `(progn forms...)`: Evaluate these `forms` sequentially, and return the value of the last. |
| put! | FUNC | `(put! store key value)`: Stores a value in a namespace; currently (0.0.6), also stores a value in a hashmap, but in future if the `store` is a hashmap then `put!` will return a clone of that hashmap with this `key value` pair added.  Expects `store` to be a hashmap or namespace; `key` to be a symbol or a keyword; `value` to be  any value. |
| put-all! | FUNC | `(put-all! dest source)`: If `dest` is a namespace and is writable, copies all key-value pairs from `source` into `dest`. At present (0.0.6) it does this for hashmaps as well, but in future if `dest` is a hashmap or a namespace which the user does not have permission to write, will return a copy of `dest` with all the key-value pairs from `source` added. `dest` must be a hashmap or a namespace; `source` may be either of those or an association list. |
| quote | SPFM | `(quote form)`: Returns `form`, unevaluated. More idiomatically expressed `'form`, where the quote mark is a reader macro which is expanded to `(quote form)`. |
| ratio->real | FUNC | `(ratio->real r)`: If `r` is a rational number, return the real number equivalent. |
| read | FUNC | `(read stream)`: read one complete lisp form and return it. If `stream` is specified and is a read stream, then read from that stream, else the stream which is the value of  `*in*` in the environment. |
| read-char | FUNC | `(read-char stream)`: Return the next character. If `stream` is specified and is a read stream, then read from that stream, else the stream which is the value of  `*in*` in the environment. |
| repl | FUNC | `(repl prompt input output)`: Starts a new read-eval-print-loop. All arguments are optional. If `prompt` is present, it will be used as the prompt. If `input` is present and is a readable stream, takes input from that stream. If `output` is present and is a writable stream, prints output to that stream. |
| reverse | FUNC | `(reverse sequence)` Returns a sequence of the top level elements of this `sequence`, which may be a list or a string, in the reverse order. |
| set | FUNC | `(set symbol value namespace)`: Binds the value `symbol` in the specified  `namespace` to the value of `value`, altering the namespace in so doing, and returns `value`. If `namespace` is not specified, it defaults to the default namespace. |
| set! | SPFM | `(set! symbol value namespace)`: Binds `symbol` in  `namespace` to the value of `value`, altering the namespace in so doing, and returns `value`. If `namespace` is not specified, it defaults to the default namespace. |
| slurp | FUNC | `(slurp read-stream)` Read all the characters from `read-stream` to the end of stream, and return them as a string. |
| source | FUNC | `(source  object)`: If `object` is an interpreted function or interpreted special form, returns the source code; else nil. Once we get a compiler working, will also return the source code of compiled functions and special forms. |
| subtract | FUNC | `(- a b)`: Subtracts `b` from `a` and returns the result. Expects both arguments to be numbers. |
| throw | FUNC | `(throw message cause)`: Throw an exception with this `message`, and, if specified, this `cause` (which is expected to be an exception but need not be).|
| time | FUNC | `(time arg)`: Return a time object. If an `arg` is supplied, it should be an integer which will be interpreted as a number of microseconds since the big bang, which is assumed to have happened 441,806,400,000,000,000 seconds before the UNIX epoch. |
| try | SPFM | `(try forms... (catch symbol forms...))`: Doesn't work yet! |
| type | FUNC | `(type object)`: returns the type of the specified `object`. Currently (0.0.6) the type is returned as a four character string; this may change. |
| λ | SPFM | `(lamda arg-list forms...)`: Construct an interpretable &lambda; function. |

## Known bugs 

The following bugs are known in 0.0.6:

1. bignum arithmetic does not work (returns wrong answers, does not throw exception);
2. subtraction of ratios is broken (returns wrong answers, does not throw exception);
3. equality of hashmaps is broken (returns wrong answers, does not throw exception);
4. The garbage collector doesn't work at all well.

There are certainly very many unknown bugs.


