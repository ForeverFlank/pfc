#include "ruledef.asm"

imm a, 2
st  a, 1
imm b, 3
st  b, a

ld  c, a
ld  d, 1

halt