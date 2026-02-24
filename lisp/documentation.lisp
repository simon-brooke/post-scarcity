(set! documentation (lambda (object)
    (cond ((= (type object) "LMDA") 
            (let (d (nth 3 (source object)))
                (cond ((string? d) d)
                    (t (source object)))))
        ((member (type object) '("FUNC" "SPFM"))
            (:documentation (meta object))))))

(set! doc documentation)
