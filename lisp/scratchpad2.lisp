"This demonstrates that although the print representation of three cell bignums blows up, the internal representation is sane"

"We start by adding 8 copies of 2^60 - i.e. the first two-cell integer"

(set! a
  (+
    1152921504606846976
    1152921504606846976
    1152921504606846976
    1152921504606846976
    1152921504606846976
    1152921504606846976
    1152921504606846976
    1152921504606846976))

"Then repeatedly add eight copies of the previous generation"

(set! b (+ a a a a a a a a))

(set! c (+ b b b b b b b b))

(set! d (+ c c c c c c c c))

(set! e (+ d d d d d d d d))

(set! f (+ e e e e e e e e))

(set! g (+ f f f f f f f f))

(set! h (+ g g g g g g g g))

(set! i (+ h h h h h h h h))

(set! j (+ i i i i i i i i))

(set! k (+ j j j j j j j j))

(set! l (+ k k k k k k k k))

(set! m (+ l l l l l l l l))

(set! n (+ m m m m m m m m))

(set! o (+ n n n n n n n n))

"p"
(set! p (+ o o o o o o o o))

"q"
(set! q (+ p p p p p p p p))

"r"
(set! r (+ q q q q q q q q))

"s"
(inspect
 (set! s (+ r r r r r r r r)))

"t - first three cell integer. Printing blows up here"
(inspect
 (set! t (+ s s s s s s s s)))

"u"
(inspect
 (set! u (+ t t t t t t t t)))

"v"
(inspect
 (set! v (+ u u u u u u u u)))

"w"
(inspect
 (set! w (+ v v v v v v v v)))

(inspect
 (set! x (+ w w w w w w w w)))

(inspect
 (set! y (+ x x x x x x x x)))

(inspect
 (set! z (+ y y y y y y y y)))

(inspect
 (set! final (+ z z z z z z z z)))
