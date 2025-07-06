# System private functions

**actually, I think this is a bad idea &mdash; or at least needs significantly more thought!**

System-private functions are functions private to the system, which no normal user is entitled to access; these functions normally have an [[access control]] value of NIL.

# (sys-access-control arg)

System private. Takes one argument. Returns the access control list of its argument.

# (sys-readable arg user)

System private. Takes two arguments. Returns `TRUE` if the first argument is readable by the reader represented by the second argument; else `NIL`.

