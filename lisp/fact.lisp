(set! fact
      (lambda (n)
      (cond ((= n 1) 1)
        (t (* n (fact (- n 1)))))))

(fact 20)
