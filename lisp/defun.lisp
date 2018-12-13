;; Because I don't (yet) have syntax for varargs, the body must be passed
;; to defun as a list of sexprs.
(set! defun!
      (nlambda
       (name args body)
       (cond (symbolp name)
         (set! name (apply lambda (cons args body))))))

(defun! square (x) ((* x x)))

(set! defsp!
      (nlambda
       (name args body)
       (cond (symbolp name)
         (set! name (nlambda args body)))))

(defsp! cube (x) ((* x x x)))

(set! p 5)

(square 5) ;; should work

(square p) ;; should work

(cube 5) ;; should work

(cube p) ;; should fail: unbound symbol
