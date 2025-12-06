#include "ruledef.asm"

; video buffer = 0x00 - 0x07
; y position   = 0x08
; direction    = 0x09

; 0  1
;  \/
;  /\
; 2  3

; start at (0, 2)
imm a, 128
imm b, 2
st  a, b
st  b, 8

loop:

    imm a, 1
    st  a, 0xfd
    xor a

display:
    ld  b, a
    st  b, 0xfc
    add 1
    cmp 8
    jl  display

    imm a, 2
    st  a, 0xfd

    ld  a, 8
    mov b, a
    cmp 0
    jne bounce_top_end
    ld  a, 9
    or  2
    st  a, 9
    jmp bounce_bottom_end
bounce_top_end:
    cmp 7
    jne bounce_bottom_end
    ld  a, 9
    and 1
    st  a, 9
bounce_bottom_end:
    ld  a, b
    cmp 128
    jne bounce_left_end
    ld  a, 9
    or  1
    st  a, 9
    jmp bounce_right_end
bounce_left_end:
    cmp 1
    jne bounce_right_end
    ld  a, 9
    and 2
    st  a, 9
bounce_right_end:

    ld  c, 8
    ld  a, 9
    mov b, a
    tst 1
    ld  a, c
    imm d, 0
    st  d, c
    jnz right
left:
    add a
    jmp lr_end
right:
    shr
lr_end:
    mov d, a
    mov a, b
    tst 2
    mov a, c
    jnz down
up:
    sub 1
    jmp ud_end
down:
    add 1
ud_end:
    st  d, a
    st  a, 8

    jmp loop