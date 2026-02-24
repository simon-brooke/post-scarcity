(set! > (lambda (a b)
    "`(> a b)`: Return `t` if `a` is a number greater than `b`, else `nil`."
    (not (negative? (- a b)))))