# Access control
_
ote that a number of details not yet finalised are used in examples in this note. There must be some mechanism for creating fully qualified and partially qualified hierarchical names, but I haven't finalised it yet. In this note I've assumed that the portions of an hierarchical name are separated by periods ('.'); that fully qualified names start with a quote mark; and that where a name doesn't start with a quote mark, the first portion of it is evaluated in the current environment and its value assumed to be a fully qualified equivalent. All of these details may change._

In a multi-user environment, access control is necessary in order for a user to be able to protect an item of data from being seen by someone who isn't authorised to see it. But actually, in a world of immutable data, it's less necessary than you might think. As explained in my note on [[Memory, threads and communication]], if there's strict immutability, and all user processes spawn from a common root process, then no user can see into any other user's data space anyway.

But that makes collaboration and communication impossible, so I've proposed namespaces be mutable. So the value of a name in a [[namespace]] will be a data item and inevitably that data item will be in some user's data space. So we do need an access control list on each data item.

## Initial thoughts

My initial plan was that the access control list would have the following structure:

1. **NIL** would mean only the owner could read it;
2. **T** would mean that anyone could read it;
3. A list of user objects and group objects, any of whom could read it.

This does not work. We don't know who the owner of a cell is; in the current design we don't store that information, and I cannot see merit in storing that information. So **NIL** cannot mean 'only the owner could read it'. It essentially means 'no-one can read it' (or possibly 'only system processes can read it, which is potentially useful).

Worse though, if the list points to immutable user and group objects, then if a new user is added to the system after the object was created, they can never be added to the access control list.

## Write access

As most data is immutable, there's no need for write access lists. If it exists, you can't write it, period. You can make a modified copy, but you can't modify the original. So most data objects don't need a write access list.

A sort-of minor exception to this is write streams. If you have normal access to a write stream, gatekept by the normal access lists, you can write to the stream; what you can't do is change where the stream points to. As you can't read from a write stream, there's still only one access list needed.

However, if (some) [[namespaces]] are mutable - and I believe some must be - then a namespace does need a write access list, in addition to its (normal) read access list. The structure of a write access list will be the same as of a read access list.

### Modifying write access lists on mutable namespaces

If mutable namespaces have write access lists, then someone has to be able to manage the content of those write access lists - including modify them. Of course, data being immutable, 'modify' here really means replace. Who has authority to modify the access control list of a mutable namespace? The simplest thing would be for mutable namespaces to have an extra key, '**write-access**', which pointed to the write access list. Then any user with write access to the namespace could modify the write access list. That may be undesirable and needs further thought, but any other solution is going to be complex.

## Execute access

I don't see the need for separate read and execute access lists. This is possibly slightly affected by whether the system can run interpreted code, compiled code, or both. If it can run interpreted code, then having read access to the source is equivalent to having execute access, unless there is a separate execute access list (which I don't want). Thus, a user able to edit a system function would also be able to execute it - but as themselves, not as system, so it would not be able to call any further system functions it depended on, unless the user also had read access to them. Note that, of course, in order to put the new version of the function into the system namespace, to make it the version which will be called by other processes, the user would need write access to the system namespace.

However, it's really hard to make the semantics of interpreted code identical to compiled code, and compilation is no longer such a big deal on modern fast processors. So I don't see the necessity of being able to run interpretted code; it's easier if source and executable are different objects, and, if they're different objects, they can have different access lists. So having access to the source doesn't necessarily means having access to the executable, and vice versa.

If only compiled code can be executed, then it seems to me that having access to the compiled code means one can execute it, and still there's only one access list needed.

## Read access

Thus the default access list is the read access list; every cell has an access list. What do its possible values mean?

1. **T** everyone can read this;
2. An executable function of two arguments: the current user can read the cell if the function, when passed as arguments the current user and the cell to which access is requested, returns **T** or a list of names as below, and the user is present on that list;
3. A list of names: true if the value of one of those names is the user object of the current user, or is a group which contains the user object of the current user.

If there's anything on the list which isn't a name it's ignored. Any value of the access list which isn't **T**, an executable function, of a list of names is problematic; we either have to treat it as **T** (everyone) or as **NIL** (either no-one or system-only). We should probably flag an error if an attempt is made to create a cell with an invalid access list. Access control list cells also clearly have their own access control lists; there is a potential for very deep recursion and consequently poor performance here, so it will be desirable to keep such access control lists short or just **T**. Obviously, if you can't read an access control list you can't read the cell that it guards.

## If data is immutable, how is an access control list set?

My idea of this is that there will be a priviliged name which is bound in the environment of each user; each user will have their own binding for this name, and, furthermore, they can change the binding of the name in their environment. For now I propose that this name shall be **friends**. The value of **friends** should be an access list as defined above. The access control list of any cell is the value that **friends** had in the environment in the environment in which it was created, at the time it was created.

## Managing access control

The `with` function can be used to make this easier:

```
(with ((*friends* . list-or-t-or-executable)) s-exprs...)
```

Creates a new environment in which **friends** is bound to the value of **list-or-t-or-executable**, and within that environment evaluates the specified **s-exprs**. Any cells created during that evaluation will obviously have **list-or-t-or-executable** as their access control. Returns the value of executing the last **s-expr**.

### (get-access-control s-expr)

Returns the access control list of the object which is the value of the **s-expr**.

### Typical use cases

Suppose I want to compile a function **foo** which will be executable by all my current friends and additionally the group **foo-users**:

```
    (with-open-access-control
      (cons ::system:groups:foo-users *friends*)
      (rebind! ::system:users:simon:functions:foo (compile foo))
```

_Here **rebind!** creates a new binding for the name **foo** in the namespace **::system:users:simon:functions**, whether or not that name was previously bound there. Analogous to the Clojure function **swap!**_

Suppose I want to compile a function **bar** which will be executable by exactly the same people as **foo**:

    (with-access-control
      (get-access-control 'system.users.simon.exec.foo)
      (rebind! 'system.users.simon.exec.bar (compile bar))

Suppose I want to do some work which is secret, visible only to me and not to my normal friends:

```
    (with ((*friends* . (list current-user)))
      (repl))
```

(or, obviously, 
```
    (with ((*friends* current-user))
      (repl))
```
which is in practice exactly the same)

_Here **repl** starts a new read-eval-print loop in the modified environment - I suspect this is a common use case._

Suppose I want to permanently add Anne and Bill to my normal friends:

```
    (rebind! *environment*:*friends* (cons ::system:users:anne (cons ::system:users:bill *friends*)))
```

_Here I'm presuming that `*environment*` is bound to the value of `::system:users:simon:environment`, and that unqualified names are searched for first in my own environment._

Suppose I want everyone to be able to play a game, but only outside working hours; and for my friends to be able to play it additionally at lunchtime:

```
    (with ((*friends* 
      (compile
        (lambda (user cell)
          (let ((time . (now)))
            (cond 
              ((< time 09:00) T)
              ((> time 17:00) T)
              ((and (> time 12:30)(< time 13:30)) *friends*)
              (T NIL)))))))
      (rebind! ::system:users:simon:functions:excellent-game (compile excellent-game)))
```

## Summary

Security is hard to do well, and all this requires careful further thought, in part because the proposed post-scarcity environment is so unlike any existing software environment.
