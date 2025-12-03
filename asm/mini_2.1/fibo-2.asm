#include "ruledef.asm"

xor a
st  a, 0
st  a, 1
st  a, 3

imm a, 1
st  a, 2

loop:
    ; add lower byte
    ld  a, 0
    ld  b, 2
    add b
    st  b, 0
    st  a, 2
    mov c, a

    ; add upper byte
    ld  a, 1
    ld  b, 3
    adc b
    st  b, 1
    st  a, 3

    ; jump if overflow
    jc  end

    ; store result
    st  c, 0xfc
    st  a, 0xfd

    jmp loop

end:
    halt