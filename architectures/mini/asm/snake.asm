#include "ruledef.asm"

; video buffer      = 0x20 - 0x27
; snake pos         = 0x40 - 0x7f

; button input      = 0xfc
; rng               = 0xfd

snake_dir_addr = 0x10
head_ptr_addr  = 0x11
food_pos_addr  = 0x12
head_pos_addr  = 0x13
tail_pos_addr  = 0x14

vbuf_begin_addr = 0x20
vbuf_end_addr   = 0x28

snake_pos_begin_addr = 0x40
snake_pos_last_addr  = 0x7f

btn_addr = 0xfc
rng_addr = 0xfd
matrix_data_addr = 0xfc
matrix_ctrl_addr = 0xfd

start:
    ; init variables
    imm a, 4
    st  a, snake_dir_addr

    imm a, 0x40
    st  a, head_ptr_addr

    imm a, 0b00011011
    st  a, head_pos_addr

gen_food:
    ld  a, rng_addr
    and 0b00111111
    st  a, food_pos_addr
    jmp gen_food_end

loop:
    ; button input
    ld  a, btn_addr
    cmp 0
    je  btn_end
    st  a, snake_dir_addr
btn_end:
    ; move head
    ld  a, head_ptr_addr    ; load snake head ptr
    mov b, a
    ld  c                   ; load snake head pos
    ld  b, snake_dir_addr   ; load snake direction
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

    ; check if food eaten
    ld  b, food_pos_addr
    cmp b
    jne food_not_eaten
    ld  a, head_ptr_addr    ; if eaten, increment head ptr
    add 1
    st  a, head_ptr_addr
    jmp gen_food

food_not_eaten:
    ; shift snake
    imm a, snake_pos_begin_addr
    imm c, 1
shift:
    add c
    mov b, a
    ld  d                   ; load mem[x + 1]
    sub c
    mov b, a
    st  d                   ; store at mem[x]
    add c
    cmp snake_pos_last_addr
    jnz shift
gen_food_end:
    ; store head pos
    ld  a, head_pos_addr    ; load the moved head pos to register b
    ld  b, head_ptr_addr    ; load head ptr
    st  a                   ; store head pos at head ptr

    ; draw snake
    mov a, b
    add 1
    mov c, a
    imm a, snake_pos_begin_addr
draw_snake:
    mov b, a
    ld  d
    st  d, 0xfe
    add 1
    cmp c
    jl  draw_snake

    ; draw food
    ld  a, food_pos_addr
    st  a, 0xfe

    jmp loop