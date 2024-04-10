### logi_c

A simple **Truth Table** generator writen in C.

To build, you can follow this steps:

```bash
    mkdir build
    make
    # or
    make run
```

Operators

AND            -> ^
OR             -> v
XOR            -> +
CONDITIONAL    -> ->
BI-CONDITIONAL -> <->

All expressions must be between parenteses

`~(a ^ b) v c`
is valid

`~((a ^ b) v c)`
is valid

`~a ^ b v c`
is not valid


