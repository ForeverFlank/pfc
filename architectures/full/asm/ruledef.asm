#ruledef reg
{
    r0  => 0x0
    r1  => 0x1
    r2  => 0x2
    r3  => 0x3
    r4  => 0x4
    r5  => 0x5
    r6  => 0x6
    r7  => 0x7
    r8  => 0x8
    r9  => 0x9
    r10 => 0xa
    r11 => 0xb
    r12 => 0xc
    r13 => 0xd
    r14 => 0xe
    r15 => 0xf
}

#ruledef
{
    add {rd: reg}, {rs1: reg}, {rs2: reg}           => 0x00 @ rs1 @ rd @ 0x0 @ rs2 @ 0x00
    adc {rd: reg}, {rs1: reg}, {rs2: reg}           => 0x00 @ rs1 @ rd @ 0x0 @ rs2 @ 0x01
    sub {rd: reg}, {rs1: reg}, {rs2: reg}           => 0x00 @ rs1 @ rd @ 0x0 @ rs2 @ 0x02
    sbb {rd: reg}, {rs1: reg}, {rs2: reg}           => 0x00 @ rs1 @ rd @ 0x0 @ rs2 @ 0x03
    adds {rd: reg}, {rs1: reg}, {rs2: reg}          => 0x00 @ rs1 @ rd @ 0x0 @ rs2 @ 0x10
    adcs {rd: reg}, {rs1: reg}, {rs2: reg}          => 0x00 @ rs1 @ rd @ 0x0 @ rs2 @ 0x11
    subs {rd: reg}, {rs1: reg}, {rs2: reg}          => 0x00 @ rs1 @ rd @ 0x0 @ rs2 @ 0x12
    sbbs {rd: reg}, {rs1: reg}, {rs2: reg}          => 0x00 @ rs1 @ rd @ 0x0 @ rs2 @ 0x13

    add {rd: reg}, {rs1: reg}, {val: i8}            => 0x01 @ rs1 @ rd @ val @ 0x00
    adc {rd: reg}, {rs1: reg}, {val: i8}            => 0x01 @ rs1 @ rd @ val @ 0x01
    sub {rd: reg}, {rs1: reg}, {val: i8}            => 0x01 @ rs1 @ rd @ val @ 0x02
    sbb {rd: reg}, {rs1: reg}, {val: i8}            => 0x01 @ rs1 @ rd @ val @ 0x03
    adds {rd: reg}, {rs1: reg}, {val: i8}           => 0x01 @ rs1 @ rd @ val @ 0x10
    adcs {rd: reg}, {rs1: reg}, {val: i8}           => 0x01 @ rs1 @ rd @ val @ 0x11
    subs {rd: reg}, {rs1: reg}, {val: i8}           => 0x01 @ rs1 @ rd @ val @ 0x12
    sbbs {rd: reg}, {rs1: reg}, {val: i8}           => 0x01 @ rs1 @ rd @ val @ 0x13

    halt                                            => 0x0000000F

    mov {rd: reg}, {rs1: reg}                       => 0x00 @ rs1 @ rd @ 0x0000
    ldi {rd: reg}, {val: i8}                        => 0x01 @ 0x0 @ rd @ val @ 0x00
}