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

; generate shift lookup and clear vbuf
imm a, 8
imm b, 0x01
imm c, 0
init_arr:
    sub 1
    jn  init_arr_end
    st  b, a
    mov d, a
    add vbuf_begin_addr
    st  c, a
    mov a, b
    add a                   ; effectively shr
    mov b, a
    mov a, d
    jmp init_arr
init_arr_end:

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

    ; delete tail
    ld  b, snake_pos_begin_addr             ; load tail pos
    mov a, b
    and 0b00111000          ; y pos
    shr                     ; >> 3
    shr
    shr
    add vbuf_begin_addr     ; offset to the destinated row address
    mov d, a                ; keep in register d
    mov a, b
    and 0b00000111          ; x pos
    ld  a, a                ; load value single pixel
    xor 0xff                ; invert for bitmask
    ld  b, d                ; load existing row
    and b                   ; clear pixel
    st  a, d                ; store row with added pixel

    ; move head
    ld  a, head_ptr_addr    ; load snake head ptr
    ld  c, a                ; load snake head pos
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
gen_food_end:

food_not_eaten:
    ; shift snake
    imm a, snake_pos_begin_addr
    imm c, 1
shift:
    add c
    ld  b, a                ; load mem[x + 1]
    sub c
    st  b, a                ; store at mem[x]
    add c
    mov b, a
    sub snake_pos_last_addr
    mov a, b
    jnz shift
store_head:
    ; store head pos
    ld  a, head_pos_addr    ; load the moved head pos to register b
    ld  b, head_ptr_addr    ; load head ptr
    st  a, b                ; store head pos at head ptr

    ; draw snake
    mov b, a
    and 0b00111000          ; y pos
    shr                     ; >> 3
    shr
    shr
    add vbuf_begin_addr     ; offset to the destinated row address
    mov d, a                ; keep in register d
    mov a, b
    and 0b00000111          ; x pos
    ld  b, a                ; load value single pixel
    ld  a, d                ; load existing row
    or  b                   ; add pixel
    st  a, d                ; store row with added pixel

    ; draw food
    ld  b, food_pos_addr
    mov a, b
    and 0b00111000
    shr
    shr
    shr
    add vbuf_begin_addr
    mov d, a
    mov a, b
    and 0b00000111
    ld  b, a
    ld  a, d
    or  b
    st  a, d 

    ; upload vbuf to matrix display
    imm a, 1
    st  a, matrix_ctrl_addr
    imm a, vbuf_begin_addr
    upload_vbuf:
        ld  b, a
        st  b, matrix_data_addr
        add 1
        cmp vbuf_end_addr
        jne upload_vbuf
    imm a, 2
    st  a, matrix_ctrl_addr

    jmp loop

halt