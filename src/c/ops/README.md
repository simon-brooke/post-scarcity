# README: PSSE substrate operations

This folder/pseudo-package is for things which implement basic Lisp functions.
These will be the functions which make up the `:bootstrap` and `:substrate`
packages in Lisp.

For each basic function the intention is that there should be one `.c` file 
(and normally one `.h` file as well). This file will provide one version of the
function with Lisp calling conventions, called `lisp_xxxx`, and one with C 
calling conventions, called `xxxx`. It does not matter whether the lisp version
calls the C version or vice versa, but one should call the other so there are 
not two different versions of the logic.

Substrate I/O functions will not be provided in this pseudo-package but in `io`. 
Substrate arithmetic functions will not be provided in this pseudo-package but 
in `arith`.