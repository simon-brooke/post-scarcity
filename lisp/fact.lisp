(set! fact
      (lambda (n)
      (cond ((= n 1) 1)
        (true (* n (fact (- n 1)))))))
