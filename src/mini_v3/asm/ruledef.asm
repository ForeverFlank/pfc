#ruledef reg
{
    a => 0b00
    b => 0b01
    c => 0b10
    d => 0b11
}

#ruledef
{
    mov {rd: reg}, {rs: reg}    => 0x0 @ rd @ rs
    imm {rd: reg}, {val: i8}    => 0x1 @ rd @ 0b00 @ val
    nop                         => 0x20
    halt                        => 0x28

    jmp {dest: i8}              => 0x30 @ dest
    jz  {dest: i8}              => 0x38 @ dest
    je  {dest: i8}              => 0x38 @ dest
    jn  {dest: i8}              => 0x39 @ dest
    jl  {dest: i8}              => 0x39 @ dest
    jc  {dest: i8}              => 0x3a @ dest
    jo  {dest: i8}              => 0x3b @ dest
    jnz {dest: i8}              => 0x3c @ dest
    jne {dest: i8}              => 0x3c @ dest
    jnn {dest: i8}              => 0x3d @ dest
    jge {dest: i8}              => 0x3d @ dest
    jnc {dest: i8}              => 0x3e @ dest
    jno {dest: i8}              => 0x3f @ dest

    st  {rs: reg}, {ra: reg}    => 0x4 @ rs @ ra
    st  {rs: reg}, {addr: i8}   => 0x5 @ rs @ 0b00 @ addr
    ld  {rd: reg}, {ra: reg}    => 0x6 @ rd @ ra
    ld  {rd: reg}, {addr: i8}   => 0x7 @ rd @ 0b00 @ addr

    add {rs: reg}               => 0x8 @ 0b00 @ rs
    sub {rs: reg}               => 0x8 @ 0b01 @ rs
    adc {rs: reg}               => 0x8 @ 0b10 @ rs
    sbb {rs: reg}               => 0x8 @ 0b11 @ rs
    add {imm: i8}               => 0x9 @ 0b0000 @ imm
    sub {imm: i8}               => 0x9 @ 0b0100 @ imm
    adc {imm: i8}               => 0x9 @ 0b1000 @ imm
    sbb {imm: i8}               => 0x9 @ 0b1100 @ imm
    and {rs: reg}               => 0xa @ 0b00 @ rs
    or  {rs: reg}               => 0xa @ 0b01 @ rs
    xor {rs: reg}               => 0xa @ 0b10 @ rs
    shr                         => 0xa @ 0b1100
    and {imm: i8}               => 0xb @ 0b0000 @ imm
    or  {imm: i8}               => 0xb @ 0b0100 @ imm
    xor {imm: i8}               => 0xb @ 0b1000 @ imm
    ror                         => 0xb @ 0b1100

    cmp {rs: reg}               => 0xc @ 0b01 @ rs
    cmp {imm: i8}               => 0xd @ 0b0100 @ imm
    tst {rs: reg}               => 0xe @ 0b00 @ rs
    tst {imm: i8}               => 0xf @ 0b0000 @ imm
}
