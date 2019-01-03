(set! expt (lambda
            (n x)
            "Return the value of `n` raised to the `x`th power."
            (cond
              ((= x 1) n)
              (t (* n (expt n (- x 1)))))))

(expt 2 65)
