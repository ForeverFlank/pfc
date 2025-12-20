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
    hlt                         => 0x10
    halt                        => 0x10

    mov {rd: reg}, {rs: reg}    => 0x2 @ rd @ rs
    imm {rd: reg}, {val: i8}    => 0x3 @ 0b00 @ rd @ val

    st  {rs: reg}               => 0x4 @ 0b00 @ rs
    st  {rs: reg}, {addr: i8}   => 0x4 @ 0b01 @ rs @ addr
    ld  {rd: reg}               => 0x5 @ 0b00 @ rd
    ld  {rd: reg}, {addr: i8}   => 0x5 @ 0b01 @ rd @ addr

    jmp {dest: i8}              => 0x60 @ dest
    jz  {dest: i8}              => 0x70 @ dest
    je  {dest: i8}              => 0x70 @ dest
    jn  {dest: i8}              => 0x71 @ dest
    jl  {dest: i8}              => 0x71 @ dest
    jc  {dest: i8}              => 0x72 @ dest
    jle {dest: i8}              => 0x73 @ dest
    jnz {dest: i8}              => 0x74 @ dest
    jne {dest: i8}              => 0x74 @ dest
    jnn {dest: i8}              => 0x75 @ dest
    jge {dest: i8}              => 0x75 @ dest
    jnc {dest: i8}              => 0x76 @ dest
    jg  {dest: i8}              => 0x77 @ dest

    add {rs: reg}               => 0x8 @ 0b00 @ rs
    add {imm: i8}               => 0x8 @ 0b0100 @ imm
    adc {rs: reg}               => 0x8 @ 0b10 @ rs
    adc {imm: i8}               => 0x8 @ 0b1100 @ imm
    sub {rs: reg}               => 0x9 @ 0b00 @ rs
    sub {imm: i8}               => 0x9 @ 0b0100 @ imm
    sbb {rs: reg}               => 0x9 @ 0b10 @ rs
    sbb {imm: i8}               => 0x9 @ 0b1100 @ imm
    and {rs: reg}               => 0xa @ 0b00 @ rs
    and {imm: i8}               => 0xa @ 0b0100 @ imm
    or  {rs: reg}               => 0xa @ 0b10 @ rs
    or  {imm: i8}               => 0xa @ 0b1100 @ imm
    xor {rs: reg}               => 0xb @ 0b00 @ rs
    xor {imm: i8}               => 0xb @ 0b0100 @ imm
    not {rs: reg}               => 0xb8
    shl                         => 0xc0
    rol                         => 0xc8
    shr                         => 0xd0
    ror                         => 0xd8
    cmp {rs: reg}               => 0xe @ 0b00 @ rs
    cmp {imm: i8}               => 0xe @ 0b0100 @ imm
    tst {rs: reg}               => 0xe @ 0b10 @ rs
    tst {imm: i8}               => 0xe @ 0b1100 @ imm
}
