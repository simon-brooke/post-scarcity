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
(inspect (+ z z z z z z z z z z))
