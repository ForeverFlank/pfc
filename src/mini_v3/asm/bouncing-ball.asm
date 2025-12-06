#include "ruledef.asm"

; video buffer = 0x00 - 0x07
; x position   = 0x08
; y position   = 0x09
; direction    = 0x0a

; 0  1
;  \/
;  /\
; 2  3

imm a, 0
st  a, 8

imm a, 2
st  a, 9

imm b, 1
st  b, 10

imm a, 0x17
gen_shl:
    st  b, a
    mov c, a
    mov a, b
    add a
    mov b, a
    mov a, c
    sub 1
    cmp 0x10
    jge gen_shl

loop:
    xor a
    mov b, a

clear:
    st  b, a
    add 1
    cmp 8
    jl  clear

    ld  a, 8
    add 0x10
    ld  a, a
    ld  c, 9
    st  a, c

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

    ld  a, 10
    ld  b, 8
    imm d, 1
case_0:
    cmp 0
    jne case_1
    mov a, b
    sub d
    mov b, a
    mov a, c
    sub d
    jmp move_end
case_1:
    cmp 1
    jne case_2
    mov a, b
    add d
    mov b, a
    mov a, c
    sub d
    jmp move_end
case_2:
    cmp 2
    jne case_3
    mov a, b
    sub d
    mov b, a
    mov a, c
    add d
    jmp move_end
case_3:
    mov a, b
    add d
    mov b, a
    mov a, c
    add d
move_end:
    mov c, a

    st  b, 8
    st  c, 9
    imm d, 10

    mov a, b
    cmp 0
    jne bounce_left_end
    ld  a, d
    or  1
    st  a, d
bounce_left_end:
    mov a, b
    cmp 7
    jne bounce_right_end
    ld  a, d
    and 2
    st  a, d
bounce_right_end:
    mov a, c
    cmp 0
    jne bounce_top_end
    ld  a, d
    or  2
    st  a, d
bounce_top_end:
    mov a, c
    cmp 7
    jne bounce_bottom_end
    ld  a, d
    and 1
    st  a, d
bounce_bottom_end:

    jmp loop

halt