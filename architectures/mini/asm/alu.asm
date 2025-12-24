#include "ruledef.asm"

imm a, 1
imm b, 2
add b
cmp 3
add_error:
jne add_error

imm a, 200
imm b, 200
add b
imm a, 10
imm b, 10
adc b
cmp 21
adc_error:
jne adc_error

imm a, 50
imm b, 20
sub b
cmp 30
sub_error:
jne sub_error

imm a, 0xbc
imm b, 0xf4
sub b
imm a, 0x02
imm b, 0x01
sbb b
cmp 0
sbb_error:
jne sbb_error

imm a, 0b00110011
imm b, 0b01010101
and b
cmp 0b00010001
and_error:
jne and_error

imm a, 0b00110011
imm b, 0b01010101
or  b
cmp 0b01110111
or_error:
jne or_error

imm a, 0b00110011
imm b, 0b01010101
xor b
cmp 0b01100110
xor_error:
jne xor_error

imm a, 0b00110011
not
cmp 0b11001100
not_error:
jne not_error

imm a, 0b11010111
shl
cmp 0b10101110
shl_error:
jne shl_error

imm a, 0b10000001
shl
imm a, 0b00110010
rol
cmp 0b01100101
rol_error:
jne rol_error

imm a, 0b00001111
shr
cmp 0b00000111
shr_error:
jne shr_error

imm a, 0b10100001
shr
imm a, 0b00001100
ror
cmp 0b10000110
ror_error:
jne ror_error

halt