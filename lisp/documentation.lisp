;; This function depends on:
;; `member` (from file `member.lisp`)
;; `nth` (from `nth.lisp`)
;; `string?` (from `types.lisp`)

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

