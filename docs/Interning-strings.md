# Interning strings

I'm trying to understand what it means to intern a name in an environment with a messy and possibly shifting graph of namespaces.

My current thinking is that in data terms a name is just a string. In evaluation terms, an unquoted string (in lexical terms one unprotected by enclosing quotation marks) is a name, while a quoted string is 'a string'. So, supposing the name **froboz** is not bound in the current environment,

    (eval froboz)

causes an unbound variable exception to be thrown, while

    (eval "froboz")

returns the value **"froboz"**. This begs the question of whether there's any difference between **"froboz"** and **'froboz**, and the answer is that at this point I don't know.

There will be a concept of a root [namespace](Namespace.html), in which other namespaces may be bound recursively to form a directed graph. Because at least some namespaces are mutable, the graph is not necessarily acyclic. There will be a concept of a current namespace, that is to say the namespace in which the user is currently working. 

There must be some notation to say distinguish a request for the value of a name in the root namespace and the value of a name in the current namespace. For now I'm proposing that:

    (eval froboz)

will return the value that **froboz** is bound to in the current namespace;

    (eval .froboz)

will return the value that **froboz** is bound to in the root namespace;

    (eval foobar.froboz)

will return the value that **froboz** is bound to in a namespace which is the value of the name **foobar** in the current namespace; and that

    (eval .system.users.simon.environment.froboz)

will return the value that **froboz** is bound to in the environment of the user of the system called **simon**.

The exact path separator syntax may change, but the principal that when interning a symbol it is broken down into a path of tokens, and that the value of each token is sought in a namespace bound to the previous token, is likely to remain.

Obviously if **froboz** is interned in one namespace it is not necessarily interned in another, and vice versa. There's a potentially nasty problem here that two lexically identical strings might be bound in different namespaces, so that there is not one canonical interned **froboz**; if this turns out to cause problems in practice there will need to be a separate canonical [hashtable](Hashtable.html) of individual path elements.

Obviously this means there may be arbitrarily many paths which reference the same data item. This is intended.

## Related functions

### (intern! string)

Binds *string*, considered as a path, to **NIL**. If some namespace along the path doesn't exist, throws an exception. Obviously if the current user is not entitled to write to the terminal namespace, also throws an exception.

### (intern! string T)

Binds *string*, considered as a path, to **NIL**. If some namespace along the path doesn't exist, create it as the current user with both read and write [access control](Access-control.html) lists taken from the current binding of **friends** in the current environment. Obviously if the current user is not entitled to write to the last pre-existing namespace, throws an exception.

### (intern! string T write-access-list)

Binds *string*, considered as a path, to **NIL**. If some namespace along the path doesn't exist, create it as the current user with the read [access control](https://www.journeyman.cc/blog/posts-output/2006-02-20-postscarcity-software/) list taken from the current binding of **friends** in the current environment, and the write access control list taken from the value of *write-access-list*. Obviously if the current user is not entitled to write to the last pre-existing namespace, throws an exception.

### (set! string value)

Binds *string*, considered as a path, to *value*. If some namespace along the path doesn't exist, throws an exception. Obviously if the current user is not entitled to write to the terminal namespace, also throws an exception.

### (set! string value T)

Binds *string*, considered as a path, to *value*. If some namespace along the path doesn't exist, create it as the current user with both read and write [access control](Access-control.html) lists taken from the current binding of **friends** in the current environment. Obviously if the current user is not entitled to write to the last pre-existing namespace, throws an exception.

### (set! string value T write-access-list)

Binds *string*, considered as a path, to *value*. If some namespace along the path doesn't exist, create it as the current user with the read [access control](Access-control.html) list taken from the current binding of **friends** in the current environment, and the write access control list taken from the value of *write-access-list*. Obviously if the current user is not entitled to write to the last pre-existing namespace, throws an exception.

### (put! string token value)

Considers *string* as the path to some namespace, and binds *token* in that namespace to *value*. *Token* should not contain any path separator syntax. If the namespace doesn't exist or if the current user is not entitled to write to the namespace, throws an exception.

### (string-to-path string)

Behaviour as follows:
    (string-to-path "foo.bar.ban") => ("foo" "bar" "ban")
    (string-to-path ".foo.bar.ban") => ("" "foo" "bar" "ban")

Obviously if the current user can't read the string, throws an exception.

### (path-to-string list-of-strings)

Behaviour as follows:
    (path-to-string '("foo" "bar" "ban")) => "foo.bar.ban"
    (path-to-string '("" "foo" "bar" "ban")) => ".foo.bar.ban"

Obviously if the current user can't read some element of *list-of-strings*, throws an exception.

### (interned? string)

Returns a string lexically identical to *string* if *string*, considered as a path, is bound (i.e. 
1. all the non-terminal elements of the path are bound to namespaces, 
2. all these namespaces are readable by the current user;
3. the terminal element is bound in the last of these;

Returns nil if *string* is not so bound, but all namespaces along the path are readable by the current user.

I'm not certain what the behaviour should be if some namespace on the path is not readable. The obvious thing is to throw an access control exception, but there are two possible reasons why not:
1. It may turn out that this occurs too often, and this just becomes a nuisance;
2. From a security point of view, is evidence that there is something there to which you don't have access (but that is in a sense an argument against ever throwing an access exception at all).