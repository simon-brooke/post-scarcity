(set! nil? (lambda 
          (o) 
          "`(nil? object)`: Return `t` if object is `nil`, else `t`."
          (= o nil)))

(set! member? (lambda
            (item collection)
            "`(member item collection)`: Return `t` if this `item` is a member of this `collection`, else `nil`."
            (cond
              ((nil? collection) nil)
              ((= item (car collection)) t)
              (t (member? item (cdr collection))))))

;; (member? (type member?) '("LMDA" "NLMD"))
