# README: PSSE substrate arithmetic

This folder/pseudo package is to implement enough of arithmetic for bootstrap:
that is, enough that all more sophisticated arithmetic can be built on top of
it.

Ratio arithmetic will not be implemented in the substrate, but `make-ratio` 
will. The signature for `make-ratio` will be:

`(make-ratio dividend divisor) => ratio`

Both divisor and dividend should be integers. If the divisor is `1` it will 
return the dividend (as an integer). If the divisor is 0 it will return &infin;.

This implies we need a privileged data item representing infinity...

Bignum arithmetic will not be implemented in the substrate, but `make-bignum`
will be. The signature for `make-bignum` will be

`(make-bignum integer) => bignum`

If the integer argument is less than 64 bits, the argument will be returned 
unmodified. If it is more than 64 bits, a bignum of the same value will be 
returned.