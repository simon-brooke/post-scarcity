# Hybrid assoc lists

In it's current very prototype stage, PSSE has to forms of name/value store. One is the assoc list, the other is the hashmap. 

An assoc (association) list is a list of the form:

    ((name<sub>1</sub> . value<sub>1</sub>)(name<sub>2</sub> . value<sub>2</sub>)(name<sub>3</sub> . value<sub>3</sub>)...)

Hashmaps have many very clear advantages, but assoc lists have one which is very important in the evaluation environment, and that is precisely its sequentiality. Thus, if the same name is bound twice on an assoc list, the value nearest the head is the one which will be recovered:

    (assoc :bar '((:foo . 1) (:bar . "Hello there!")(:ban . 3)(:bar . 2)))
    => "Hello there!"

Why does this matter? Well, for precisely the same reason it matters when a UNIX system searches for an executable.

Suppose Clare is a user who trusts both Alice and Bob, but she trusts Alice more than Bob. Suppose both Alice and Bob have written implementations of a function called `froboz`. Suppose Clare invokes

    (froboz 3)

Which implementation of `froboz` should be evaluated? An assoc list makes that simple. If Clare binds Alice's implementation into her environment later than Bob's, Alice's will be the one found.

But an assoc list is also fearsomely inefficient, especially if we are in a system with many thousands of names, each of which may be bound multiple times in typical runtime environment.

How to resolve this? How to get some of the benefits of sequential access of assoc lists, with some of the efficiency benefits of hashmaps? What I'm going to propose is a **hybrid assoc list**, that is to say, a list whose members are either

1. (key . value) pairs, or else
2. hashmaps.

So suppose we have a list, `l`, thus:

    ((:foo . 1) (:bar . 2) {:foo "not this" :ban 3} (:ban . "not this either") (:froboz . 4))

Then:

    (assoc :foo l) => 1
    (assoc :bar l) => 2
    (assoc :ban l) => 3
    (assoc :froboz l) => 4

This will make the implementation of namespaces and search paths vastly easier.