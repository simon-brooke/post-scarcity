;; This function depends on:
;; `member` (from file `member.lisp`)
;; `nth` (from `nth.lisp`)
;; `string?` (from `types.lisp`)

(set! nil? (lambda 
          (o) 
          "`(nil? object)`: Return `t` if object is `nil`, else `t`."
          (= o nil)))

(set! member? (lambda
            (item collection)
            "`(member item collection)`: Return `t` if this `item` is a member of this `collection`, else `nil`."
            (print (list "In member? item is " item "; collection is " collection))
            (println)
            (cond
              ((= 0 (count collection)) nil)
              ((= item (car collection)) t)
              (t (member? item (cdr collection))))))

;; (member? (type member?) '("LMDA" "NLMD"))

(set! nth (lambda (n l) 
    "Return the `n`th member of this list `l`, or `nil` if none."
    (cond ((= nil l) nil)
        ((= n 1) (car l))
        (t (nth (- n 1) (cdr l))))))

(set! string? (lambda (o) "True if `o` is a string." (= (type o) "STRG") ) )

(set! documentation (lambda (object)
    "`(documentation object)`:  Return documentation for the specified `object`, if available, else `nil`."
   (cond ((member? (type object) '("FUNC" "SPFM"))
            (:documentation (meta object)))
        ((member? (type object) '("LMDA" "NLMD")) 
           (let ((d . (nth 3 (source object))))
               (cond ((string? d) d)
                   (t (source object)))))
        (t object))))

(set! doc documentation)

(documentation apply)

;; (documentation member?)

