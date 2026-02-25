# State of Play

## 20260224

Found a bug in subtraction, which I hoped might be a clue into the bignum bug;
but it proved just to be a careless bug in the small integer cache code (and
therefore a new regression). Fixed this one, easily.

In the process spotted a new bug in subtracting rationals, which I haven't yet
looked at.

Currently working on a bug which is either in `let` or `cond`, which is leading
to non-terminating recursion...

H'mmm, there are bugs in both.

#### `let`

The unit test for let is segfaulting. That's a new regression today, because in
last night's buildv it doesn't segfault. I don't know what's wrong, but to be
honest I haven't looked very hard because I'm trying to fix the bug in `cond`.

#### `cond`

The unit test for `cond` still passes, so the bug that I'm seeing is not 
triggered by it. So it's not necessarily a new bug. What's happening? Well,
`member` doesn't terminate.

The definition is as follows:

```lisp
(set! nil? 
    (lambda 
        (o) 
        "`(nil? object)`: Return `t` if object is `nil`, else `t`."
        (= o nil)))

(set! member 
    (lambda
        (item collection)
        "`(member item collection)`: Return `t` if this `item` is a member of this `collection`, else `nil`."
        (cond
            ((nil? collection) nil)
            ((= item (car collection)) t)
            (t (member item (cdr collection))))))
```

In the execution trace, with tracing of bind, eval and lambda enabled, I'm 
seeing this loop on the stack:

```
Stack frame with 1 arguments:
	Context:  <= (member item (cdr collection)) <= ((nil? collection) nil) <= (cond ((nil? collection) nil) ((= item (car collection)) t) (t (member item (cdr collection)))) <= "LMDA"
Arg 0:	CONS	count:          6	value: (member item (cdr collection))
Stack frame with 3 arguments:
	Context:  <= ((nil? collection) nil) <= (cond ((nil? collection) nil) ((= item (car collection)) t) (t (member item (cdr collection)))) <= "LMDA" <= (member item (cdr collection))
Arg 0:	CONS	count:          7	value: ((nil? collection) nil)
Arg 1:	CONS	count:          7	value: ((= item (car collection)) t)
Arg 2:	CONS	count:          7	value: (t (member item (cdr collection)))
Stack frame with 1 arguments:
	Context:  <= (cond ((nil? collection) nil) ((= item (car collection)) t) (t (member item (cdr collection)))) <= "LMDA" <= (member item (cdr collection)) <= ((nil? collection) nil)
Arg 0:	CONS	count:          8	value: (cond ((nil? collection) nil) ((= item (car collection)) t) (t (member item (cdr collection))))
Stack frame with 2 arguments:
	Context:  <= "LMDA" <= (member item (cdr collection)) <= ((nil? collection) nil) <= (cond ((nil? collection) nil) ((= item (car collection)) t) (t (member item (cdr collection))))
Arg 0:	STRG	count:         19	value: "LMDA"
Arg 1:	NIL 	count: 4294967295	value: nil
Stack frame with 1 arguments:
	Context:  <= (member item (cdr collection)) <= ((nil? collection) nil) <= (cond ((nil? collection) nil) ((= item (car collection)) t) (t (member item (cdr collection)))) <= "LMDA"
Arg 0:	CONS	count:          6	value: (member item (cdr collection))
Stack frame with 3 arguments:
	Context:  <= ((nil? collection) nil) <= (cond ((nil? collection) nil) ((= item (car collection)) t) (t (member item (cdr collection)))) <= "LMDA" <= (member item (cdr collection))
Arg 0:	CONS	count:          7	value: ((nil? collection) nil)
Arg 1:	CONS	count:          7	value: ((= item (car collection)) t)
Arg 2:	CONS	count:          7	value: (t (member item (cdr collection)))
Stack frame with 1 arguments:
	Context:  <= (cond ((nil? collection) nil) ((= item (car collection)) t) (t (member item (cdr collection)))) <= "LMDA" <= (member item (cdr collection)) <= ((nil? collection) nil)
Arg 0:	CONS	count:          8	value: (cond ((nil? collection) nil) ((= item (car collection)) t) (t (member item (cdr collection))))
Stack frame with 2 arguments:
	Context:  <= "LMDA" <= (member item (cdr collection)) <= ((nil? collection) nil) <= (cond ((nil? collection) nil) ((= item (car collection)) t) (t (member item (cdr collection))))
Arg 0:	STRG	count:         19	value: "LMDA"
Arg 1:	NIL 	count: 4294967295	value: nil
```

This then just goes on, and on, and on. The longest instance I've got the trace of wound up more than a third of a million stack frames before I killed it. What appears to be happening is that the cond clause

```lisp
((nil? collection) nil)
```

Executes correctly and returns nil; but that instead of terminating the cond expression at that point it continues and executes the following two clauses, resulting in (infinite) recursion.

This is bad.

But what's worse is that the clause

```lisp
((= item (car collection)) t)
```

also doesn't terminate the `cond` expression, even when it should.

And the reason? From the trace, it appears that clauses *never* succeed. But if that's true, how come the unit tests are passing?

Problem for another day.

I'm not going to commit today's work to git, because I don't want to commit something I know segfaults.

## 20260220

### State of the build

The only unit tests that are failing now are the bignum tests, which I have 
consciously parked as a future problem, and the memory leak, similarly. The
leak is a lot less bad than it was, but I'm worried that stack frames
are not being freed.

If you run 

```
cat lisp/fact.lisp | target/psse -d 2>&1 |\
        grep 'Vector space object of type' | sort | uniq -c | sort -rn
```

you get a huge number (currently 394) of stack frames in the memory dump; they 
should all have been reclaimed. There's other stuff in the memory dump as well, 

```
    422 CONS    ;; cons cells, obviously
    394 VECP    ;; pointers to vector space objects -- specifically, the stack frames
    335 SYMB    ;; symbols
    149 INTR    ;; integers
     83 STRG    ;; strings
     46 FUNC    ;; primitive (i.e. written in C) functions
     25 KEYW    ;; keywords
     10 SPFM    ;; primitive special forms
      3 WRIT    ;; write streams: `*out*`, `*log*`, `*sink*` 
      1 TRUE    ;; t
      1 READ    ;; read stream: `*in*`
      1 NIL     ;; nil
      1 LMDA    ;; lambda function, specifically `fact`
```

Generally, for each character in a string, symbol or keyword there will be one 
cell (`STRG`, `SYMB`, or `KEYW`) cell, so the high number of STRG cells is not 
especially surprising. It looks as though none of the symbols bound in the 
oblist are being recovered on exit, which is undesirable but not catastrophic,
since it's a fixed burden of memory which isn't expanding.

But the fact that stack frames aren't being reclaimed is serious.

### Update, 19:31

Right, investigating this more deeply, I found that `make_empty_frame` was doing 
an `inc_ref` it should not have been, Having fixed that I'm down to 27 frames 
left in the dump. That's very close to the number which will be generated by
running `(fact 25)`, so I expect it is now only stack frames for interpreted 
functions which are not being reclaimed. This give me something to work on!


## 20260215

Both of yesterday's regressions are fixed. Memory problem still in much the 
same state.

> Allocation summary: allocated 1210; deallocated 10; not deallocated 1200.

That left the add ratios problem which was deeper. I had unintended unterminated 
recursion happening there. :-(

It burned through 74 cons pages each of 1,024 cons cells, total 76,800 cells, 
and 19,153 stack frames. before it got there; and then threw the exception back
up through each of those 19,153 stack frames. But the actual exception message
was `Unrecognised tag value 0 (    )`, which is not enormously helpful.
S
However, once I had recognised what the problem was, it was quickly fSixed, with
the added bonus that the new solution will automatically work for bignum 
fractions once bignums are working.

So we're down to eight unit tests failing: the memory leak, one unimplemented 
feature, and the bignum problem.

At the end of the day I decided to chew up some memory by doing a series of 
moderately large computations, to see how much memory is being successfully 
deallocated.

```lisp
:: (mapcar fact '(1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20))

(1 2 6 24 120 720 5,040 40,320 362,880 3,628,800 39,916,800 479,001,600 
1,932,053,504 1,278,945,280 2,004,310,016 2,004,189,184 4,006,445,056 
3,396,534,272 109,641,728 2,192,834,560)
:: 

Allocation summary: allocated 10136; deallocated 548; not deallocated 9588.
```

So, about 5%. This is still a major problem, and is making me doubt my reference
counting strategy. Must do better!

Note that the reason that the numbers become eratic past about two billion is 
the bignum arithmetic bug.

## 20260214

### Memory leaks

The amount I'm leaking memory is now down by an order of magnitude, but the problem is not fixed. 
Better, not good enough. And although I'm aware of the amount to which Lisp objects are not being
reclaimed, there may also be transient C objects &mdash; cheifly strings &mdash; which are also
not being freed. This is an ongoing process. 

But you'll remember that a week ago my base case was:

> Allocation summary: allocated 19986; deallocated 245; not deallocated 19741.

Now it's 

> Allocation summary: allocated 1188; deallocated 10; not deallocated 1178.

That is better.

### Unit tests

The unit test system got into a mess because the bignum tests are failing. But because I know
some tests are failing, and the bignum problem feels so intractable that I don't want to 
tackle it, I've been ignoring the fact that tests are failing; which means I've
missed regressions &mdash; until I started to get an 'Attempt to take value of unbound symbol' 
exception for `nil`, which is extremely serious and broke a lot of things.

That arose out of work on the 'generalised key/value stores' feature, logged under 
[#20260203](20260203), below. However, because I wasn't paying attention to failing tests, it
took me a week to find and fix it.

But I've fixed that one. And I've put a lot of work into [cleaning up the unit tests](https://git.journeyman.cc/simon/post-scarcity/commit/222368bf640a0b79d57322878dee42ed58b47bd6).
There is more work to do on this.

### Documentation

I'm also gradually working through cleaning up documentation.

### Regressions

Meantime we have some regressions which are serious, and must be resolved.

#### equals

The core function `equals` is now failing, at least for integers. Also.

```lisp
(= 0.75 3/4)
```

fails because I've never implemented a method for it, which I ought.

#### cond

The current unit test for `cond` and that for `recursion` both fail but *I think* this is because `equals` is failing.

#### rational arithmetic

I have a horrible new regression in rational arithmetic which looks as though something is being freed when it shouldn't be.

#### All tests failing as at 20260214

As follows:

1. unit-tests/bignum-expt.sh => (expt 2 61): Fail: expected '2305843009213693952', got ''
2. unit-tests/bignum-expt.sh => (expt 2 64): Fail: expected '18446744073709551616', got ''
3. unit-tests/bignum-expt.sh => (expt 2 65): Fail: expected '36893488147419103232', got ''
4. unit-tests/bignum-print.sh => unit-tests/bignum-print.sh => printing 576460752303423488: Fail: expected '576460752303423488', got '0'
5. unit-tests/bignum-print.sh => printing 1152921504606846976: Fail: expected '1152921504606846976', got '0'
6. unit-tests/bignum-print.sh => printing 1152921504606846977: Fail: expected '1152921504606846977', got '1'
7. unit-tests/bignum-print.sh => printing 1329227995784915872903807060280344576: Fail: expected '1329227995784915872903807060280344576', \n           got '0'
8. unit-tests/bignum.sh => unit-tests/bignum.sh => Fail: expected '1,152,921,504,606,846,976', got '0'
9. unit-tests/bignum-subtract.sh => unit-tests/bignum-subtract.sh => subtracting 1 from 1152921504606846976: Fail: expected '1152921504606846975', got '4294967295'
10. unit-tests/bignum-subtract.sh => subtracting 1 from 1152921504606846977: Fail: expected '1152921504606846976', got '0'
11. unit-tests/bignum-subtract.sh => subtracting 1 from 1152921504606846978: Fail: expected '1152921504606846977', got '1'
12. unit-tests/bignum-subtract.sh => subtracting 1152921504606846977 from 1: Fail: expected '-1152921504606846976', got '0'
13. unit-tests/bignum-subtract.sh => subtracting 10000000000000000000 from 20000000000000000000: Fail: expected '10000000000000000000', got '2313682944'
14. unit-tests/cond.sh => unit-tests/cond.sh: cond with one clause... Fail: expected '5', got 'nil'
15. unit-tests/memory.sh => Fail: expected '1188', got '10'
16. unit-tests/ratio-addition.sh => Fail: expected '1/4', got 'Error: Unrecognised tag value 4539730 ( REE)'
17. unit-tests/recursion.sh => Fail: expected 'nil 3,628,800', got ''

### New master version

I haven't done a 'release' of Post Scarcity since September 2021, because I've 
been so despondent about the bignum problem. But actually a lot of this *is*
usable, and it's at least sufficiently intereting that other people might want
to play with it, and possibly even might fix some bugs.

So I'm currently planning to release a new master before the end of this month,
and publicise it.

## 20260204

### Testing what is leaking memory

#### Analysis

If you just start up and immediately abort the current build of psse, you get:

> Allocation summary: allocated 19986; deallocated 245; not deallocated 19741.

Allocation summaries from the current unit tests give the following ranges of values:

|                 | Min   | Max   |      |
| --------------- | ----- | ----- | ---- |
| Allocated       | 19991 | 39009 |      |
| Deallocated     |   238 |  1952 |      |
| Not deallocated | 19741 | 37057 |      |

The numbers go up broadly in sinc with one another &mdash; that is to say, broadly, as the number allocated rises, so do both the numbers deallocated and the numbers not deallocated. But not exactly.

#### Strategy: what doesn't get cleaned up?

Write a test wrapper which reads a file of forms, one per line, from standard input, and passes each in turn to a fresh invocation of psse, reporting the form and the allocation summary.

```bash
#1/bin/bash

while IFS= read -r form; do
    allocation=`echo ${form} | ../../target/psse 2>&1 | grep Allocation`
    echo "* ${allocation}: ${form}"
done
```

So, from this:

* Allocation summary: allocated 19986; deallocated 245; not deallocated 19741.: 
* Allocation summary: allocated 19990; deallocated 249; not deallocated 19741.: ()
* Allocation summary: allocated 20019; deallocated 253; not deallocated 19766.: nil

Allocating an empty list allocates four additional cells, all of which are deallocated. Allocating 'nil' allocates a further **29** cells, 25 of which are not deallocated. WTF?

Following further work I have this, showing the difference added to the base case of cells allocated, cells deallocated, and, most critically, cells not deallocated.

From this we see that reading and printing `nil` allocates an additional 33 cells, of which eight are not cleaned up. That's startling, and worrying.

But the next row shows us that reading and printing an empty list costs only four cells, each of which is cleaned up. Further down the table we see that an empty map is also correctly cleaned up. Where we're leaking memory is in reading (or printing, although I doubt this) symbols, either atoms, numbers, or keywords (I haven't yet tried strings, but I expect they're similar.)

| **Case**                          | **Delta Allocated** | **Delta Deallocated** | **Delta Not Deallocated** |
| --------------------------------- | ------------------- | --------------------- | ------------------------- |
| **Basecase**                      | 0                   | 0                     | 0                         |
| **nil**                           | 33                  | 8                     | 25                        |
| **()**                            | 4                   | 4                     | 0                         |
| **(quote ())**                    | 39                  | 2                     | 37                        |
| **(list )**                       | 37                  | 12                    | 25                        |
| **(list 1)**                      | 47                  | 14                    | 33                        |
| **(list 1 1)**                    | 57                  | 16                    | 41                        |
| **(list 1 1 1)**                  | 67                  | 18                    | 49                        |
| **(list 1 2 3)**                  | 67                  | 18                    | 49                        |
| **(+)**                           | 36                  | 10                    | 26                        |
| **(+ 1)**                         | 44                  | 12                    | 32                        |
| **(+ 1 1)**                       | 53                  | 14                    | 39                        |
| **(+ 1 1 1)**                     | 62                  | 16                    | 46                        |
| **(+ 1 2 3)**                     | 62                  | 16                    | 46                        |
| **(list 'a 'a 'a)**               | 151                 | 33                    | 118                       |
| **(list 'a 'b 'c)**               | 151                 | 33                    | 118                       |
| **(list :a :b :c)**               | 121                 | 15                    | 106                       |
| **(list :alpha :bravo :charlie)** | 485                 | 15                    | 470                       |
| **{}**                            | 6                   | 6                     | 0                         |
| **{:z 0}**                        | 43                  | 10                    | 33                        |
| **{:zero 0}**                     | 121                 | 10                    | 111                       |
| **{:z 0 :o 1}**                   | 80                  | 11                    | 69                        |
| **{:zero 0 :one 1}**              | 210                 | 14                    | 196                       |
| **{:z 0 :o 1 :t 2}**              | 117                 | 12                    | 105                       |

Looking at the entries, we see that

1. each number read costs ten allocations, of which only two are successfully deallocated;
2. the symbol `list` costs 33 cells, of which 25 are not deallocated, whereas the symbol `+` costs only one cell fewer, and an additional cell is not deallocated. So it doesn't seem that cell allocation scales with the length of the symbol;
3. Keyword allocation does scale with the length of the keyword, apparently, since `(list :a :b :c)` allocates 121 and deallocates 15, while `(list :alpha :bravo :charlie)` allocates 485 and deallocates the same 15;
4. The fact that both those two deallocate 15, and a addition of three numbers `(+ 1 2 3)` or `(+ 1 1 1)` deallocates 16 suggest to me that the list structure is being fully reclaimed but atoms are not being. 
5. The atom `'a` costs more to read than the keyword `:a` because the reader macro is expanding `'a` to `(quote a)` behind the scenes.

### The integer allocation bug

Looking at what happens when we read a single digit  number, we get the following:

```
2
Entering make_integer
Allocated cell of type 'INTR' at 19, 507 
make_integer: returning
        INTR (1381256777) at page 19, offset 507 count 0
                Integer cell: value 0, count 0
Entering make_integer
Allocated cell of type 'INTR' at 19, 508 
make_integer: returning
        INTR (1381256777) at page 19, offset 508 count 0
                Integer cell: value 10, count 0
Entering make_integer
Allocated cell of type 'INTR' at 19, 509 
make_integer: returning
        INTR (1381256777) at page 19, offset 509 count 0
                Integer cell: value 2, count 0
Entering make_integer
Allocated cell of type 'INTR' at 19, 510 
make_integer: returning
        INTR (1381256777) at page 19, offset 510 count 0
                Integer cell: value 0, count 0
Entering make_integer
Allocated cell of type 'INTR' at 19, 506 
make_integer: returning
        INTR (1381256777) at page 19, offset 506 count 0
                Integer cell: value 0, count 0
Entering make_integer
Allocated cell of type 'INTR' at 19, 505 
make_integer: returning
        INTR (1381256777) at page 19, offset 505 count 0
                Integer cell: value 0, count 0
Entering make_integer
Allocated cell of type 'INTR' at 19, 504 
make_integer: returning
        INTR (1381256777) at page 19, offset 504 count 0
                Integer cell: value 0, count 0

Allocated cell of type 'STRG' at 19, 503 
Freeing cell    STRG (1196577875) at page 19, offset 503 count 0
                String cell: character '2' (50) with hash 0; next at page 0 offset 0, count 0
                 value: "2"
Freeing cell    INTR (1381256777) at page 19, offset 504 count 0
                Integer cell: value 2, count 0
2
Allocated cell of type 'SYMB' at 19, 504 
Allocated cell of type 'SYMB' at 19, 503 
Allocated cell of type 'SYMB' at 19, 502 
Allocated cell of type 'SYMB' at 19, 501 
Freeing cell    SYMB (1112365395) at page 19, offset 501 count 0
                Symbol cell: character '*' (42) with hash 485100; next at page 19 offset 502, count 0
                 value: *in*
Freeing cell    SYMB (1112365395) at page 19, offset 502 count 0
                Symbol cell: character 'i' (105) with hash 11550; next at page 19 offset 503, count 0
                 value: in*
Freeing cell    SYMB (1112365395) at page 19, offset 503 count 0
                Symbol cell: character 'n' (110) with hash 110; next at page 19 offset 504, count 0
                 value: n*
Freeing cell    SYMB (1112365395) at page 19, offset 504 count 0
                Symbol cell: character '*' (42) with hash 0; next at page 0 offset 0, count 0
                 value: *
```

Many things are worrying here.

1. The only thing being freed here is the symbol to which the read stream is bound &mdash; and I didn't see where that got allocated, but we shouldn't be allocating and tearing down a symbol for every read! This implies that when I create a string with `c_string_to_lisp_string`, I need to make damn sure that that string is deallocated as soon as I'm done with it &mdash; and wherever I'm dealing with symbols which will be referred to repeatedly in `C` code, I need either
   1.  to bind a global on the C side of the world, which will become messy;
   2. or else write a hash function which returns, for a `C` string, the same value that the standard hashing function will return for the lexically equivalent `Lisp` string, so that I can search hashmap structures from C without having to allocate and deallocate a fresh copy of the `Lisp` string;
   3. In reading numbers, I'm generating a fresh instance of `Lisp zero` and `Lisp ten`, each time `read_integer` is called, and I'm not deallocating them.
   4. I am correctly deallocating the number I did read, though!

## 20260203

I'm consciously avoiding the bignum issue for now. My current thinking is that if the C code only handles 64 bit integers, and bignums have to be done in Lisp code, that's perfectly fine with me.

### Hashmaps, assoc lists, and generalised key/value stores

I now have the oblist working as a hashmap, and also hybrid assoc lists which incorporate hashmaps working. I don't 100% have consistent methods for reading stores which may be plain old assoc lists, new hybrid assoc lists, or hashmaps working but it isn't far off. This also takes me streets further towards doing hierarchies of hashmaps, allowing my namespace idea to work &mdash; and hybrid assoc lists provide a very sound basis for building environment structures.

Currently all hashmaps are mutable, and my doctrine is that that is fixable when access control lists are actually implemented. 

#### assoc

The function `(assoc store key) => value` should be the standard way of getting a value out of a store.  

#### put!

The function `(put! store key value) => store` should become the standard way of setting a value in a store (of course, if the store is an assoc list or an immutable map, a new store will be returned which holds the additional key/value binding).

### State of unit tests

Currently:

> Tested 45, passed 39, failed 6

But the failures are as follows:
```
unit-tests/bignum-add.sh => checking a bignum was created: Fail
unit-tests/bignum-add.sh => adding 1152921504606846977 to 1: Fail: expected 't', got 'nil'
unit-tests/bignum-add.sh => adding 1 to 1152921504606846977: Fail: expected 't', got 'nil'
unit-tests/bignum-add.sh => adding 1152921504606846977 to 1152921504606846977: Fail: expected 't', got 'nil'
unit-tests/bignum-add.sh => adding 10000000000000000000 to 10000000000000000000: Fail: expected 't', got 'nil'
unit-tests/bignum-add.sh => adding 1 to 1329227995784915872903807060280344576: Fail: expected 't', got 'nil'
unit-tests/bignum-add.sh => adding 1 to 3064991081731777716716694054300618367237478244367204352: Fail: expected 't', got 'nil'
unit-tests/bignum-expt.sh => (expt 2 60): Fail: expected '1152921504606846976', got '1'
unit-tests/bignum-expt.sh => (expt 2 61): Fail: expected '2305843009213693952', got '2'
unit-tests/bignum-expt.sh => (expt 2 64): Fail: expected '18446744073709551616', got '16'
unit-tests/bignum-expt.sh => (expt 2 65): Fail: expected '36893488147419103232', got '32'
unit-tests/bignum-print.sh => printing 1152921504606846976: Fail: expected '1152921504606846976', got '1'
unit-tests/bignum-print.sh => printing 1152921504606846977: Fail: expected '1152921504606846977', got '2'
unit-tests/bignum-print.sh => printing 1329227995784915872903807060280344576: Fail: expected '1329227995784915872903807060280344576', \n           got '1151321504605245376'
unit-tests/bignum.sh => unit-tests/bignum.sh => Fail: expected '1,152,921,504,606,846,976', got '1'
unit-tests/bignum-subtract.sh => unit-tests/bignum-subtract.sh => subtracting 1 from 1152921504606846976: Fail: expected '1152921504606846975', got '0'
unit-tests/bignum-subtract.sh => subtracting 1 from 1152921504606846977: Fail: expected '1152921504606846976', got '1'
unit-tests/bignum-subtract.sh => subtracting 1 from 1152921504606846978: Fail: expected '1152921504606846977', got '2'
unit-tests/bignum-subtract.sh => subtracting 1152921504606846977 from 1: Fail: expected '-1152921504606846976', got '1'
unit-tests/bignum-subtract.sh => subtracting 10000000000000000000 from 20000000000000000000: Fail: expected '10000000000000000000', got '-376293541461622793'
unit-tests/memory.sh
```

In other words, all failures are in bignum arithmetic **except** that I still have a major memory leak due to not decrefing somewhere where I ought to.

### Zig

I've also experimented with autotranslating my C into Zig, but this failed. Although I don't think C is the right language for implementing my base Lisp in, it's what I've got; and until I can get some form of autotranslate to bootstrap me into some more modern systems language, I think I need to stick with it.

## 20250704

Right, I'm getting second and subsequent integer cells with negative values, which should not happen. This is probably the cause of (at least some of) the bignum problems. I need to find out why. This is (probably) fixable.

```lisp
:: (inspect 10000000000000000000)

        INTR (1381256777) at page 3, offset 873 count 2
                Integer cell: value 776627963145224192, count 2
                BIGNUM! More at:
        INTR (1381256777) at page 3, offset 872 count 1
                Integer cell: value -8, count 1
```

Also, `print` is printing bignums wrong on ploughwright, but less wrong on mason, which implies a code difference. Investigate.

## 20250314

Thinking further about this, I think at least part of the problem is that I'm storing bignums as cons-space objects, which means that the integer representation I can store has to fit into the size of a cons pointer, which is 64 bits. Which means that to store integers larger than 64 bits I need chains of these objects.

If I stored bignums in vector space, this problem would go away (especially as I have not implemented vector space yet). 

However, having bignums in vector space would cause a churn of non-standard-sized objects in vector space, which would mean much more frequent garbage collection, which has to be mark-and-sweep because unequal-sized objects, otherwise you get heap fragmentation.

So maybe I just have to put more work into debugging my cons-space bignums.

Bother, bother.

There are no perfect solutions.

However however, it's only the node that's short on vector space which has to pause to do a mark and sweep. It doesn't interrupt any other node, because their reference to the object will remain the same, even if it is the 'home node' of the object which is sweeping. So all the node has to do is set its busy flag, do GC, and clear its busy flag, The rest of the system can just be carrying on as normal.

So... maybe mark and sweep isn't the big deal I think it is?

## 20250313

OK, the 60 bit integer cell happens in `int128_to_integer` in `arith/integer.c`. It seems to be being done consistently; but there is no obvious reason. `MAX_INTEGER` is defined in `arith/peano.h`. I've changed both to use 63 bits, and this makes no change to the number of unit tests that fail.

With this change, `(fact 21)`, which was previously printing nothing, now prints a value, `11,891,611,015,076,642,816`. However, this value is definitively wrong, should be `51,090,942,171,709,440,000`. But, I hadn't fixed the shift in `integer_to_string`; have now... still no change in number of failed tests...

But `(fact 21)` gives a different wrong value, `4,974,081,987,435,560,960`. Factorial values returned by `fact` are correct (agree with SBCL running the same code) up to `(fact 20)`, with both 60 bit integer cells and 63 bit integer cells giving correct values.

Uhhhmmm... but I'd missed two other places where I'd had the number of significant bits as a numeric literal. Fixed those and now `(fact 21)` does not return a printable answer at all, although the internal representation is definitely wrong. So we may be seeing why I chose 60 bits.

Bother.

## 20250312

Printing of bignums definitely doesn't work; I'm not persuaded that reading of bignums works right either, and there are probably problems with bignum arithmetic too.

The internal memory representation of a number rolls over from one cell to two cells at 1152921504606846976, and I'm not at all certain why it does because this is neither 2<sup>63</sup> nor 2<sup>64</sup>.

|                |                      |      |
| -------------- | -------------------- | ---- |
| 2<sup>62</sup> | 4611686018427387904  |      |
| 2<sup>63</sup> | 9223372036854775808  |      |
| 2<sup>64</sup> | 18446744073709551616 |      |
| Mystery number | 1152921504606846976  |      |

In fact, our mystery number turns out (by inspection) to be 2<sup>60</sup>. But **why**?
