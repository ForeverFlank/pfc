#include "ruledef.asm"

max = 100

imm c, 0
st  c, 0xfc
st  c, 0xfd

imm a, max
imm b, 1
init:
    sub 1
    st  b, a
    jnz init
st  c, c
st  c, b

imm a, 2
sieve_init:
    mov b, a
    st  a, 0xfc
    st  c, 0xfd
sieve:
    add b
    st  c, a
    cmp max
    jl  sieve
mov a, b

find_prime:
    add 1
    cmp max
    jge end
    ld  b, a
    mov d, a
    mov a, b
    tst 1
    mov a, d
    jz find_prime
jmp sieve_init

end:
    halt