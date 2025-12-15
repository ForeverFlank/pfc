#include "ruledef.asm"

imm a, 1
imm b, 2
cmp b
jne case_1

case_1:
    imm c, 1
    halt
case_2:
    imm c, 2
    halt