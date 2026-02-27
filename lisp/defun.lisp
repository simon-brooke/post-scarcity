(set! symbol? (lambda (x) (equal (type x) "SYMB")))

(set! defun!
      (nlambda
      "`(defun name arg-list forms...)`: Define an interpreted Lambda function with this `name` and this `arg-list`, whose body is comprised of these `forms`."
       form
       (eval (list 'set! (car form) (cons 'lambda (cdr form))))))

(defun! square (x) (* x x))

(set! defsp!
      (nlambda
       form
       (cond (symbol? (car form))
         (set! (car form) (apply nlambda (cdr form))))))

(defun! cube (x) (* x x x))

(set! p 5)

(square 5) ;; should work

(square p) ;; should work

(cube 5) ;; should work

(cube p) ;; should fail: unbound symbol
