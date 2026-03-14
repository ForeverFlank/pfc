#include "ruledef.asm"

begin:
    mov a, 0
    mov b, 1
loop:
    mov [0xfc], a

    mov c, a
    add b
    mov b, c

    jnc loop

    halt
