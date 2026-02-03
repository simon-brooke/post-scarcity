# State of Play

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
