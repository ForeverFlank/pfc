#include "ruledef.asm"

; video buffer      = 0x20 - 0x27
; snake pos         = 0x40 - 0x7f

; button input      = 0xfc
; rng               = 0xfd

snake_dir_addr = 0x10
head_ptr_addr  = 0x11
food_pos_addr  = 0x12
head_pos_addr  = 0x13

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
    mov [snake_dir_addr], 4
    mov [head_ptr_addr], 0x40
    mov [head_pos_addr], 0b00011011

gen_food:
    mov a, [rng_addr]
    and 0b00111111
    mov [food_pos_addr], a
    jmp gen_food_end

loop:
    ; button input
    mov a, [btn_addr]
    cmp 0
    je  btn_end
    mov [snake_dir_addr], a
    
btn_end:
    ; move snake head
    mov a, [head_ptr_addr]  ; load snake head ptr
    mov c, [a]              ; load snake head pos
    mov b, [snake_dir_addr] ; load snake direction
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
    mov [head_pos_addr], a  ; keep moved head pos

draw_snake:
    mov a, [head_ptr_addr]  ; load head ptr
draw_snake_loop:
    mov b, [a]
    mov c, a
    mov a, b

    mov [0xfe], a
    or  0x40
    mov [0xfe], a
    and 0x3f
    mov [0xfe], a

    mov a, c
    dec
    cmp snake_pos_begin_addr
    jge draw_snake_loop

    ; draw food
    mov a, [food_pos_addr]
    mov [0xfe], a
    or  0x40
    mov [0xfe], a
    and 0x3f
    mov [0xfe], a

check_food_eaten:
    mov a, [head_pos_addr]
    cmp [food_pos_addr]
    jne shift_snake
    mov a, [head_ptr_addr]  ; if eaten, increment head ptr
    inc
    mov [head_ptr_addr], a
    jmp gen_food
shift_snake:
    mov a, snake_pos_begin_addr
shift_loop:
    inc
    mov b, [a]              ; load mem[x + 1]
    dec
    mov [a], b              ; store at mem[x]
    inc
    cmp snake_pos_last_addr
    jnz shift_loop
gen_food_end:

store_head_pos:
    mov a, [head_pos_addr]  ; load the moved head pos to register b
    mov b, [head_ptr_addr]  ; load head ptr
    mov [b], a              ; store head pos at head ptr

    jmp loop