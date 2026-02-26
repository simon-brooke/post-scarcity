(set! i
  (+
    10000000000000000000
    10000000000000000000
    10000000000000000000
    10000000000000000000
    10000000000000000000
    10000000000000000000
    10000000000000000000
    10000000000000000000
    10000000000000000000
    10000000000000000000))

(set! j (+ i i i i i i i i i i))

(set! k (+ j j j j j j j j j j))

(set! l (+ k k k k k k k k k k))

(set! m (+ l l l l l l l l l l))

(set! n (+ m m m m m m m m m m))

(set! o (+ n n n n n n n n n n))

(set! p (+ o o o o o o o o o o))

(set! q (+ p p p p p p p p p p))

(set! r (+ q q q q q q q q q q))

(set! s (+ r r r r r r r r r r))

(set! t (+ s s s s s s s s s s))

(set! u (+ t t t t t t t t t t))

(set! v (+ u u u u u u u u u u))

(set! x (+ v v v v v v v v v v))

(set! y (+ x x x x x x x x x x))

"we're OK to here: 10^36, which is below the 2^120 barrier so represented as two cells"
(inspect (set! z (+ y y y y y y y y y y)))

"This blows up: 10^37, which is a three cell bignum."
(inspect (set! final (+ z z z z z z z z z z)))

(mapcar (lambda (n) (list (:name (meta n)) (:documentation (meta n)))) (keys (oblist)))

((keys "`(keys store)`: Return a list of all keys in this `store`.") (set nil) (let nil) (quote nil) (nil nil) (read nil) (nil nil) (nil nil) (oblist "`(oblist)`: Return the current symbol bindings, as a map.") (cons "`(cons a b)`: Return a cons cell whose `car` is `a` and whose `cdr` is `b`.") (source nil) (cond nil) (nil nil) (eq? "`(eq? args...)`: Return `t` if all args are the exact same object, else `nil`.") (close "`(close stream)`: If `stream` is a stream, close that stream.") (meta "`(meta symbol)`: If the binding of `symbol` has metadata, return that metadata, else `nil`.") (nil nil) (not "`(not arg)`: Return`t` only if `arg` is `nil`, else `nil`.") (mapcar "`(mapcar function sequence)`: Apply `function` to each element of `sequence` in turn, and return a sequence of the results.") (negative? "`(negative? n)`: Return `t` if `n` is a negative number, else `nil`.") (open "`(open url read?)`: Open a stream to this `url`. If `read` is present and is non-nil, open it for reading, else writing.") (subtract nil) (nil nil) (nil nil) (nil nil) (or "`(or args...)`: Return a logical `or` of all the arguments and return `t` if any is truthy, else `nil`.") (nil nil) (and "`(and args...)`: Return a logical `and` of all the arguments and return `t` only if all are truthy, else `nil`.") (count "`(count s)`: Return the number of items in the sequence `s`.") (eval nil) (nλ nil) (nil nil) (nil nil) (nil nil) (nil nil) (cdr "`(cdr arg)`: If `arg` is a sequence, return the remainder of that sequence with the first item removed.") (equal? "`(equal? args...)`: Return `t` if all args have logically equivalent value, else `nil`.") (set! nil) (nil nil) (nil nil) (reverse nil) (slurp nil) (try nil) (assoc "`(assoc key store)`: Return the value associated with this `key` in this `store`.") (nil nil) (add "`(+ args...)`: If `args` are all numbers, return the sum of those numbers.") (list "`(list args...): Return a list of these `args`.") (time nil) (car "`(car arg)`: If `arg` is a sequence, return the item which is the head of that sequence.") (nil nil) (nil nil) (nil nil) (absolute "`(absolute arg)`: If `arg` is a number, return the absolute value of that number, else `nil`.") (append "`(append args...)`: If args are all collections, return the concatenation of those collections.") (apply "`(apply f args)`: If `f` is usable as a function, and `args` is a collection, apply `f` to `args` and return the value.") (divide "`(/ a b)`: If `a` and `b` are both numbers, return the numeric result of dividing `a` by `b`.") (exception "`(exception message)`: Return (throw) an exception with this `message`.") (get-hash "`(get-hash arg)`: returns the natural number hash value of `arg`.") (hashmap "`(hashmap n-buckets hashfn store acl)`: Return a new hashmap, with `n-buckets` buckets and this `hashfn`, containing the content of this `store`.") (inspect "`(inspect object ouput-stream)`: Print details of this `object` to this `output-stream` or `*out*`.") (metadata "`(metadata symbol)`: If the binding of `symbol` has metadata, return that metadata, else `nil`.") (multiply "`(* args...)` Multiply these `args`, all of which should be numbers.") (print "`(print object stream)`: Print `object` to `stream`, if specified, else to `*out*`.") (put! nil) (put-all! nil) (ratio->real "`(ratio->real r)`: If `r` is a rational number, return the real number equivalent.") (read-char nil) (repl nil) (throw nil) (type nil) (+ "`(+ args...)`: If `args` are all numbers, return the sum of those numbers.") (* nil) (- nil) (/ nil) (= "`(equal? args...)`: Return `t` if all args have logically equivalent value, else `nil`.") (lambda nil) (λ nil) (nlambda nil) (progn nil) (nil nil) (nil nil) (nil nil) (nil nil) (nil nil) (nil nil) (nil nil) (nil nil) (nil nil) (nil nil))
