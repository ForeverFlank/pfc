#include "ruledef.asm"

; snake direction   = 0x00
; snake length      = 0x01
; snake head pos    = 0x02
; food pos          = 0x03
; snake pos         = 0x40 - 0x7f
; video memory      = 0x80 - 0xbf

; button input      = 0xfc
; rng               = 0xfd

empty_char = 0
snake_char = 1
food_char = 2

imm a, 4
st  a, 0

xor a
st  a, 1

imm a, 0b00011011
st  a, 0x40

gen_food:
    ld  a, 0xfd
    and 0x77
    st  a, 3

loop:
    ; button input
    imm c, 0
    ld  b, 0xfc
    mov a, b
    and 1
    jnz btn_pressed
    mov a, b
    and 2
    jnz btn_pressed
    mov a, b
    and 4
    jnz btn_pressed
    mov a, b
    and 8
    jnz btn_pressed
    jmp btn_end
btn_pressed:
    st  a, c
btn_end:

    ; delete tail
    ld  a, 0x40
    add 0x80
    imm b, empty_char
    st  b, a

    ; shift snake
    xor a
    imm c, 1
    shift:
        add c
        ld  b, a
        sub c
        st  b, a
        add c
        mov b, a
        sub 0x40
        mov a, b
        jnz shift
    
    ; move head
    ld  b, 0
    ld  c, 2
    mov a, b
    and 1
    jz  move_up_end
    mov a, c
    sub 0b00001000
    jmp move_fin_ud
move_up_end:
    mov a, b
    and 2
    jz  move_down_end
    mov a, c
    add 0b00001000
    jmp move_fin_ud
move_down_end:
    mov a, b
    and 4
    jz  move_right_end
    mov a, c
    add 0b00000001
    jmp move_fin_lr
move_right_end:
    mov a, b
    and 8
    jz  move_left_end
    mov a, c
    sub 0b00000001
    jmp move_fin_lr
move_left_end:

move_fin_ud:
    and 0b00111000
    mov b, a
    mov a, c
    and 0b00000111
    or  b
    st  b, 2
move_fin_lr:
    and 0b00000111
    mov b, a
    mov a, c
    and 0b00111000
    or  b
    st  b, 2
move_end:
    
    ; add head
    ld  a, c        ; c is still 1
    add 0x40
    ld  b, 2
    st  b, a
    add 0x40
    imm b, snake_char
    st  b, a

    jmp loop

halt