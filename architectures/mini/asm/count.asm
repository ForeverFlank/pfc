#include "ruledef.asm"

begin:
    xor a
count:
    mov [0xfc], a
    inc
    jnc count

    halt
