#include "ruledef.asm"

#addr 0x00
    nop4
    nop4
    jmp 0x80

#addr 0x55
    nop4
    nop4
    jmp 0xaa

#addr 0x80
    nop4
    nop4
    jmp 0x55

#addr 0xaa
    nop4
    nop4
    jmp 0x00

