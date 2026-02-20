(set! fact
      (lambda (n)
      "Compute the factorial of `n`, expected to be a natural number."
      (cond ((= n 1) 1)
        (t (* n (fact (- n 1)))))))

(fact 25)


