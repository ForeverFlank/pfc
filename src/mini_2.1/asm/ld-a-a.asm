#include "ruledef.asm"

imm a, 1
imm b, 2
st  a, b
st  b, a

ld  a, a

halt