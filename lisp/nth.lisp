(set! nth (lambda (n l) 
    "Return the `n`th member of this list `l`, or `nil` if none."
    (cond ((= nil l) nil)
        ((= n 1) (car l))
        (t (nth (- n 1) (cdr l))))))

