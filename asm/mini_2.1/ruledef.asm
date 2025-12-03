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
    halt                        => 0x10
    mov {rd: reg}, {rs: reg}    => 0x2 @ rd @ rs
    imm {rd: reg}, {val: i8}    => 0x3 @ rd @ 0b00 @ val
    st  {rs: reg}, {ra: reg}    => 0x4 @ rs @ ra
    st  {rs: reg}, {addr: i8}   => 0x5 @ rs @ 0b00 @ addr
    ld  {rd: reg}, {ra: reg}    => 0x6 @ rd @ ra
    ld  {rd: reg}, {addr: i8}   => 0x7 @ rd @ 0b00 @ addr
    add {rs: reg}               => 0x8 @ 0b00 @ rs
    sub {rs: reg}               => 0x8 @ 0b01 @ rs
    adc {rs: reg}               => 0x8 @ 0b10 @ rs
    sbb {rs: reg}               => 0x8 @ 0b11 @ rs
    and {rs: reg}               => 0x9 @ 0b00 @ rs
    or  {rs: reg}               => 0x9 @ 0b01 @ rs
    xor {rs: reg}               => 0x9 @ 0b10 @ rs
    shr                         => 0x9 @ 0b1100
    add {imm: i8}               => 0xa0 @ imm
    sub {imm: i8}               => 0xa4 @ imm
    adc {imm: i8}               => 0xa8 @ imm
    sbb {imm: i8}               => 0xac @ imm
    and {imm: i8}               => 0xb0 @ imm
    or  {imm: i8}               => 0xb4 @ imm
    xor {imm: i8}               => 0xb8 @ imm
    ror                         => 0xbc
    jmp                         => 0xc0
    jz                          => 0xe0
    jn                          => 0xe4
    jc                          => 0xe8
    jo                          => 0xec
    jnz                         => 0xf0
    jnn                         => 0xf4
    jnc                         => 0xf8
    jno                         => 0xfc
}
