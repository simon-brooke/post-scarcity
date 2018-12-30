(set! or (lambda values
                 "True if any of `values` are non-nil."
                 (cond
                   ((nil? values) nil)
                   ((car values) t)
                   (t (eval (cons 'or (cdr values)))))))
