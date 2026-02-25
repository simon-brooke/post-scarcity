;; This version segfaults, I think due to a bug in `let`?
;; (set! documentation (lambda (object)
;;    (cond ((= (type object) "LMDA") 
;;            (let ((d . (nth 3 (source object))))
;;                (cond ((string? d) d)
;;                    (t (source object)))))
;;        ((member (type object) '("FUNC" "SPFM"))
;;            (:documentation (meta object))))))
;;
;; (set! doc documentation)

;; This version returns nil even when documentation exists, but doesn't segfault.
(set! documentation
    (lambda (object)
        "`(documentation object)`:  Return documentation for the specified `object`, if available, else `nil`."
        (cond ((and (member (type object) '("LMDA" "NLMD"))
                    (string? (nth 3 (source object))))
                (nth 3 (source object)))
            ((member (type object) '("FUNC" "SPFM"))
                (:documentation (meta object))))))
