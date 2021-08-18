# How do we notate paths?

In order to make the namespaces thing work, we need a convenient way to notate paths from the current namespace to a target, or from the root to a target. This is similar to relative and absolute paths in UNIX, except that in PSSE there is no concept of a single or privileged ancestor namespace to the current namespace, so you have no equivalent of `../`.

In this discussion, a **namespace** is just a named, mutable hashmap (but not necessarily mutable by all users; indeed namespaces will almost always be mutable only by selected users. I cannot presently see a justified use for a publicly writable namespace). '**Named**', of a hashmap, merely means there is some path from the privileged  root namespace which is the value of `oblist` which leads to that hashmap. A **path** is in principle just a sequence of keys, such that the value of each successive key is bound to a namespace in the namespace bound by its predecessor. The evaluable implementation of paths will be discussed later.

I think also that there must be a privileged **session** namespace, containing information about the current session, which the user can read but not write. 

## Security considerations

What's important is that a user cannot rebind privileged names in their own environment. Thus to ensure privileged names, such names must be paths either from the `oblist`or from the current session. So we need magic, privileged notations for these things built into the reader, which cannot be overridden.

This kind of takes away from my general feeling that users should be able to override *anything*. but hey, that's engineering for you.

Users should be able to override reader macros generally; a programmable reader is in the medium term something which should be implemented. But the privileged syntax for paths should not be overridable.

## Current state of the reader

At present, you can rebind the value of the symbol `oblist` in the runtime environment. In principle, you should be able to rebind any symbol. Paths and symbols are not the same.

At present, the colon character is a separator character. So if you type

> (list :foo:bar:ban)

the reader will return

> (list :foo :bar :ban)

That's significant, and helpful.

## Constructing path notation

The Clojure threading macro, `->`, is a useful example of how we can implement this. Essentially it takes a expression as its first argument, passes the value of that expression to the function which is its second argument, the value of that as argument to the function which is its next, and so on. Given that, in Clojure, an expression which has a keyword in the function position and a hashmap in the argument position will return the value of that keyword in that hashmap, this means that, given the hashmap

>  (def x {:foo {:bar {:ban "Howzat!!"}}}) 

the expression

> (-> x :foo :bar :ban)

will return 

> "Howzat!!"

So, in general, if we implement the 'keyword in the function position' `eval` hack and the threading macro, then something like

> (-> oblist :users :simon :functions 'foo)

should return the value of the symbol `foo` in the `:functions` of the user called `:simon`.

That's stage one of our goal.

Stage two of our goal is that a stream of non-separator characters separated by colons should be interpreted as a list of keywords. Thus typing

> :foo:bar:ban

should result in not just `:foo`being read, but the list `(:foo :bar :ban)`(? not sure about this)

Stage 3 is to allow a symbol to be appended to a sequence of keywords written by using `/`as a separator, so

> :foo:bar/ban

would be read as `(:foo :bar 'ban)`

Finally, we need privileged notation for root (oblist) and for session. There are very few non-alpha-numeric characters which are available on a standard keyboard and which are not already used as significant lexical characters in Lisp readers. PSSE is not limited, of course, to the characters which are part of the ASCII character set, but it is helpful to use symbols which are reasonably convenient to type, possibly with special keyboard bindings.

So I'm going to propose that the reader should interpret

> /:users:simon:functions/assoc

as

> (-> oblist :users :simon :functions 'assoc)

where `oblist` is the actual privileged global object list, not just the current binding of `oblist` in the environment. Thus, this expression would return my personal version of the function `assoc`, whatever the symbol `assoc` was bound to in the runtime environment.

The use of the leading slash here follows UNIX convention.

I'm going to suggest that the session is referenced by the character &sect;, otherwise known as the 'silcrow'. This is not available on most keyboard mappings, so a custom mapping might be needed, or we might have to fall back on `$`.

Thus the reader should interpret 

> &sect;:user

as

>  (-> session :user)

where `session`is again a system privileged value, not the binding of `session` in the current environment.