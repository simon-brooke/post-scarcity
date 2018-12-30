(set! cons? (lambda (o) "True if o is a cons cell." (= (type o) "CONS") ) )
(set! exception? (lambda (o) "True if o is an exception." (= (type o) "EXEP")))
(set! free? (lambda (o) "Trus if o is a free cell - this should be impossible!" (= (type o) "FREE")))
(set! function? (lambda (o) "True if o is a compiled function." (= (type o) "EXEP")))
(set! integer? (lambda (o) "True if o is an integer." (= (type o) "INTR")))
(set! lambda? (lambda (o) "True if o is an interpreted (source) function." (= (type o) "LMDA")))
(set! nil? (lambda (o) "True if o is the canonical nil value." (= (type o) "NIL ")))
(set! nlambda? (lambda (o) "True if o is an interpreted (source) special form." (= (type o) "NLMD")))
(set! rational? (lambda (o) "True if o is an rational number." (= (type o) "RTIO")))
(set! read? (lambda (o) "True if o is a read stream." (= (type o) "READ") ) )
(set! real? (lambda (o) "True if o is an real number." (= (type o) "REAL")))
(set! special? (lambda (o) "True if o is a compiled special form." (= (type o) "SPFM") ) )
(set! string? (lambda (o) "True if o is a string." (= (type o) "STRG") ) )
(set! symbol? (lambda (o) "True if o is a symbol." (= (type o) "SYMB") ) )
(set! true? (lambda (o) "True if o is the canonical true value." (= (type o) "TRUE") ) )
(set! write? (lambda (o) "True if o is a write stream." (= (type o) "WRIT") ) )

(set! or (lambda values
                 "True if any of `values` are non-nil."
                 (cond ((car values) t) (t (apply 'or (cdr values))))))

(set! number?
      (lambda (o)
              "I don't yet have an `or` operator
