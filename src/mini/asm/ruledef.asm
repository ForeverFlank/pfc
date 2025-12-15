#ruledef reg
{
    a => 0b00
    b => 0b01
    c => 0b10
    d => 0b11
}

#ruledef
{
    nop                         => 0x00
    mov {rd: reg}, {rs: reg}    => 0x0 @ rd @ rs
    imm {rd: reg}, {val: i8}    => 0x1 @ 0b00 @ rd @ val

    st  {rs: reg}               => 0x2 @ 0b00 @ rs
    st  {rs: reg}, {addr: i8}   => 0x2 @ 0b01 @ rs @ addr
    ld  {rd: reg}               => 0x3 @ 0b00 @ rd
    ld  {rd: reg}, {addr: i8}   => 0x3 @ 0b01 @ rd @ addr

    cmp {rs: reg}               => 0x4 @ 0b00 @ rs
    cmp {imm: i8}               => 0x4 @ 0b0100 @ imm
    tst {rs: reg}               => 0x5 @ 0b00 @ rs
    tst {imm: i8}               => 0x5 @ 0b0100 @ imm

    jmp {dest: i8}              => 0x60 @ dest
    jz  {dest: i8}              => 0x68 @ dest
    je  {dest: i8}              => 0x68 @ dest
    jn  {dest: i8}              => 0x69 @ dest
    jl  {dest: i8}              => 0x69 @ dest
    jc  {dest: i8}              => 0x6a @ dest
    jle {dest: i8}              => 0x6b @ dest
    jnz {dest: i8}              => 0x6c @ dest
    jne {dest: i8}              => 0x6c @ dest
    jnn {dest: i8}              => 0x6d @ dest
    jge {dest: i8}              => 0x6d @ dest
    jnc {dest: i8}              => 0x6e @ dest
    jg  {dest: i8}              => 0x6f @ dest

    halt                        => 0x70

    add {rs: reg}               => 0x8 @ 0b00 @ rs
    add {imm: i8}               => 0x8 @ 0b0100 @ imm
    sub {rs: reg}               => 0x9 @ 0b00 @ rs
    sub {imm: i8}               => 0x9 @ 0b0100 @ imm
    adc {rs: reg}               => 0xa @ 0b00 @ rs
    adc {imm: i8}               => 0xa @ 0b0100 @ imm
    sbb {rs: reg}               => 0xb @ 0b00 @ rs
    sbb {imm: i8}               => 0xb @ 0b0100 @ imm
    and {rs: reg}               => 0xc @ 0b00 @ rs
    and {imm: i8}               => 0xc @ 0b0100 @ imm
    or  {rs: reg}               => 0xd @ 0b00 @ rs
    or  {imm: i8}               => 0xd @ 0b0100 @ imm
    xor {rs: reg}               => 0xe @ 0b00 @ rs
    xor {imm: i8}               => 0xe @ 0b0100 @ imm
    shr                         => 0xf @ 0b0000
}
