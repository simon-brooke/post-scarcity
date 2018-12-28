(set! list (lambda l l))

(set! symbolp (lambda (x) (equal (type x) "SYMB")))

(set! defun!
      (nlambda
       form
       (cond ((symbolp (car form))
              (set (car form) (apply 'lambda (cdr form))))
         (t nil))))

(defun! square (x) (* x x))

(set! defsp!
      (nlambda
       form
       (cond (symbolp (car form))
         (set! (car form) (apply nlambda (cdr form))))))

(defsp! cube (x) ((* x x x)))

(set! p 5)

(square 5) ;; should work

(square p) ;; should work

(cube 5) ;; should work

(cube p) ;; should fail: unbound symbol
