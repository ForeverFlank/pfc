#subruledef reg_opnd
{
    a           => 0b00
    b           => 0b01
    c           => 0b10
}

#subruledef alu_opnd
{
    a           => 0b000
    b           => 0b001
    c           => 0b010
    {imm: i8}   => 0b011 @ imm
    0           => 0b100
    [b]         => 0b101
    [c]         => 0b110
    [{imm: i8}] => 0b111 @ imm
}

#ruledef
{
    mov {d: reg_opnd}, {s: reg_opnd}    => 0b00 @ 0b0 @ d    @ 0b0 @ s
    mov {d: reg_opnd}, {s: i8}          => 0b00 @ 0b0 @ d    @ 0b0 @ 0b11   @ s

    mov {d: reg_opnd}, [{s: reg_opnd}]  => 0b00 @ 0b0 @ d    @ 0b1 @ s
    mov {d: reg_opnd}, [{s: i8}]        => 0b00 @ 0b0 @ d    @ 0b1 @ 0b11   @ s

    mov [{d: reg_opnd}], {s: reg_opnd}  => 0b00 @ 0b1 @ d    @ 0b0 @ s
    mov [{d: reg_opnd}], {s: i8}        => 0b00 @ 0b1 @ d    @ 0b0 @ 0b11   @ s
    mov [{d: i8}], {s: reg_opnd}        => 0b00 @ 0b1 @ 0b11 @ 0b0 @ s      @ d
    mov [{d: i8}], {s: i8}              => 0b00 @ 0b1 @ 0b11 @ 0b0 @ 0b11   @ d @ s

    jmp {dest: i8}                      => 0x40 @ dest
    jz  {dest: i8}                      => 0x48 @ dest
    je  {dest: i8}                      => 0x48 @ dest
    jn  {dest: i8}                      => 0x49 @ dest
    jl  {dest: i8}                      => 0x49 @ dest
    jc  {dest: i8}                      => 0x4a @ dest
    jle {dest: i8}                      => 0x4b @ dest
    jnz {dest: i8}                      => 0x4c @ dest
    jne {dest: i8}                      => 0x4c @ dest
    jnn {dest: i8}                      => 0x4d @ dest
    jge {dest: i8}                      => 0x4d @ dest
    jnc {dest: i8}                      => 0x4e @ dest
    jg  {dest: i8}                      => 0x4f @ dest

    halt                                => 0x60
    nop2                                => 0x68
    nop3                                => 0x70
    nop4                                => 0x78

    add {s: alu_opnd}                   => 0x10`5 @ s
    adc {s: alu_opnd}                   => 0x11`5 @ s
    sub {s: alu_opnd}                   => 0x12`5 @ s
    sbb {s: alu_opnd}                   => 0x13`5 @ s
    inc                                 => 0x14`5 @ 0b000
    dec                                 => 0x15`5 @ 0b000
    and {s: alu_opnd}                   => 0x16`5 @ s
    or  {s: alu_opnd}                   => 0x17`5 @ s
    xor {s: alu_opnd}                   => 0x18`5 @ s
    not                                 => 0x19`5 @ 0b000
    shl                                 => 0x1a`5 @ 0b000
    rol                                 => 0x1b`5 @ 0b000
    shr                                 => 0x1c`5 @ 0b000
    ror                                 => 0x1d`5 @ 0b000
    cmp {s: alu_opnd}                   => 0x1e`5 @ s
    tst {s: alu_opnd}                   => 0x1f`5 @ s
}
