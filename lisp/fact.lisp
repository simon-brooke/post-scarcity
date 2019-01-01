(set! fact
      (lambda (n)
      "Compute the factorial of `n`, expected to be an integer."
      (cond ((= n 1) 1)
        (t (* n (fact (- n 1)))))))

(fact 21)
