(set! nil? (lambda (o) (= o nil)))

(set! member (lambda
            (item collection)
            "Return `t` if this `item` is a member of this `collection`, else `nil`."
            (cond
              ((nil? collection) nil)
              ((= item (car collection)) t)
              (t (member item (cdr collection))))))