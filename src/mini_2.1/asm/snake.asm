#include "ruledef.asm"

; snake direction   = 0x00
; snake length      = 0x01
; food pos          = 0x02
; head pos          = 0x03
; video buffer      = 0x10 - 0x17
; snake pos         = 0x40 - 0x7f

; button input      = 0xfc
; rng               = 0xfd

snake_dir_addr = 0x01
snake_len_addr = 0x02
food_pos_addr  = 0x03
head_pos_addr  = 0x04

imm a, 4
st  a, snake_dir_addr

imm a, 1
st  a, snake_len_addr

imm a, 0b00011011
st  a, 0x40

gen_food:
    ld  a, 0xfd
    and 0b00111111
    st  a, food_pos_addr

loop:
    ; button input
    imm c, 0
    ld  a, 0xfc
    add c
    jz  btn_end
    st  a, c
btn_end:

    ; ; delete tail
    ; ld  a, 0x40             ; load tail pos
    ; add 0x80                ; shift to video buffer
    ; imm b, empty_char       ; write empty char
    ; st  b, a
    
    ; move head
    ld  b, snake_dir_addr   ; load snake direction
    ld  a, snake_len_addr   ; load snake length
    add 0x3f                ; shift to last element in array
    ld  c, a                ; load snake head pos
    mov d, a                ; keep head address in register d

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
    mov a, c
    sub 0b00000001

move_fin_lr:
    and 0b00000111
    mov b, a
    mov a, c
    and 0b00111000
    jmp move_end
move_fin_ud:
    and 0b00111000
    mov b, a
    mov a, c
    and 0b00000111
move_end:
    or  b
    st  a, head_pos_addr    ; keep moved head pos

    ; shift snake
    imm a, 0x40
    imm c, 1
    shift:
        add c
        ld  b, a
        sub c
        st  b, a
        add c
        mov b, a
        sub 0x7f
        mov a, b
        jnz shift
    
    ld  b, head_pos_addr    ; load the moved head pos
    st  b, d                ; store it at the end of pos array

    ; draw snake
    mov a, b
    and 0b00111000          ; y pos
    shr
    shr
    shr
    add 0x10
    mov d, a                ; keep in register d
    mov a, b
    and 0b00000111          ; x pos
    imm c, 1
    shl:
        

    jmp loop

halt