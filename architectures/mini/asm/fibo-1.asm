#include "ruledef.asm"

xor a
st  a, 0

imm a, 1
st  a, 1

loop:
    ld  a, 0
    ld  b, 1
    add b
    st  a, 1
    st  b, 0
    jnc loop

halt