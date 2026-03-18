(set! nil? (lambda (o) (= (type o) "NIL ")))

(set! CDR (lambda (o) 
  (print (list "in CDR; o is: " o) *log*) 
  (let ((r . (cdr o))) 
    (print (list "; returning: " r) *log*) 
    (println *log*) 
    (println *log*) 
    r)))

(set! member? 
  (lambda
    (item collection)
    (print (list "in member?: " 'item item 'collection collection) *log*)(println *log*)
    (cond
      ((nil? collection) nil)
      ((= item (car collection)) t)
      (t (member? item (CDR collection))))))
