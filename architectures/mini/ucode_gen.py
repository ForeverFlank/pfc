#!/usr/bin/env python3

from enum import IntEnum

# microinstruction bits

# fmt: off
class U(IntEnum):
    IncPc           =  0
    JmpEn           =  1
    NextInst        =  2
    Halt            =  3
    WrA             =  4
    WrB             =  5
    WrC             =  6
    WrMar           =  7
    WrDmem          =  8
    RdA             =  9
    RdB             = 10
    RdC             = 11
    RdDmem          = 12
    RdPmem          = 13
    AluEn           = 14
    AluCin          = 15
    AluInvB         = 16
    AluSelAdder     = 17
    AluS0           = 18
    AluS1           = 19
    AluShr          = 20
    # TODO: n_Update_Flags
# fmt: on


# active-low bits

# fmt: off
ACTIVE_LOW = {
    U.JmpEn, U.NextInst, U.Halt,
    U.WrA, U.WrB, U.WrC, U.WrMar, U.WrDmem,
    U.RdA, U.RdB, U.RdC, U.RdDmem, U.RdPmem,
    U.AluEn
}
# fmt: on

# helpers


def slice_bits(val: int, high: int, low: int) -> int:
    assert high >= low and low >= 0
    width = high - low + 1
    mask = (1 << width) - 1
    return (val >> low) & mask


def match(val: int, pattern: str) -> bool:
    for i, ch in enumerate(reversed(pattern)):
        bit = (val >> i) & 1
        if ch == "0" and bit != 0:
            return False
        if ch == "1" and bit != 1:
            return False
    return True


# fmt: off

def wr(reg: int) -> list[U]:
    assert 0 <= reg and reg <= 2
    return {
        0: [U.WrA],
        1: [U.WrB],
        2: [U.WrC]
    }[reg]


def rd(reg: int) -> list[U]:
    assert 0 <= reg and reg <= 3
    return {
        0: [U.RdA],
        1: [U.RdB],
        2: [U.RdC],
        3: [U.RdPmem, U.IncPc],
    }[reg]

# fmt: on


def alu_rd(reg: int) -> list[U]:
    assert 0 <= reg and reg <= 7
    return {
        0: [U.RdA],
        1: [U.RdB],
        2: [U.RdC],
        3: [U.RdPmem, U.IncPc],
        4: [],
        5: [U.RdB],
        6: [U.RdC],
        7: [U.RdPmem, U.IncPc],
    }[reg]


# microcode table
# (cycle, flags, inst) -> list of microinstructions asserted this cycle


def ucode(cycle: int, flags: int, inst: int) -> list[U]:
    assert 0 <= cycle < 4 and 0 <= flags < 8 and 0 <= inst < 256

    # decoded fields
    src = slice_bits(inst, 1, 0)
    dst = slice_bits(inst, 4, 3)

    # flag bits
    z = bool(flags & (1 << 0))
    n = bool(flags & (1 << 1))
    c = bool(flags & (1 << 2))

    # conditional jump condition codes
    # fmt: off
    cond = slice_bits(inst, 2, 0)
    cond_ok = [
        z,              # 0: jz / je
        n,              # 1: jn / jl
        c,              # 2: jc
        z or n,         # 3: jle
        not z,          # 4: jnz / jne
        not n,          # 5: jnn / jge
        not c,          # 6: jnc
        not (z or n),   # 7: jg
    ][cond]
    # fmt: on

    # ALU fields
    alu_src = slice_bits(inst, 2, 0)
    alu_mem = alu_src >= 5  # memory-reference mode

    def alu_op(op: int, rd_src: list[U], cf: bool) -> list[U]:
        if not (0 <= op and op <= 15):
            raise Exception(f"Expected 0 <= op <= 15, got {op}")

        cin = [U.AluCin] if cf else []

        # fmt: off
        table = {
             0: rd_src + [U.WrA, U.AluSelAdder],                        # add
             1: rd_src + [U.WrA, U.AluSelAdder] + cin,                  # adc
             2: rd_src + [U.WrA, U.AluSelAdder, U.AluCin, U.AluInvB],   # sub
             3: rd_src + [U.WrA, U.AluSelAdder, U.AluInvB] + cin,       # sbb
             4: [U.WrA, U.AluSelAdder, U.AluCin],                       # inc
             5: [U.WrA, U.AluSelAdder, U.AluInvB],                      # dec
             6: rd_src + [U.WrA],                                       # and
             7: rd_src + [U.WrA, U.AluS0],                              # or
             8: rd_src + [U.WrA, U.AluS1],                              # xor
             9: [U.WrA, U.AluS1, U.AluInvB],                            # not
            10: [U.WrA, U.AluSelAdder],                                 # shl
            11: [U.WrA, U.AluSelAdder, U.AluShr] + cin,                 # rol
            12: [U.WrA, U.AluS1, U.AluS0, U.AluShr],                    # shr
            13: [U.WrA, U.AluS1, U.AluS0, U.AluShr] + cin,              # ror
            14: rd_src + [U.AluSelAdder, U.AluCin, U.AluInvB],          # cmp
            15: rd_src + []                                             # tst
        }
        # fmt: on

        return table[op] + [U.AluEn, U.NextInst]

    # cycle 0 -- fetch
    if cycle == 0:
        return [U.IncPc]

    # mov d, s      (dst != 3)
    elif match(inst, "000__0__") and dst != 3:
        if cycle == 1:
            return wr(dst) + rd(src) + [U.NextInst]

    # mov d, [s]    (dst != 3)
    elif match(inst, "000__1__") and dst != 3:
        if cycle == 1:
            return [U.WrMar] + rd(src)
        if cycle == 2:
            return wr(dst) + [U.RdDmem, U.NextInst]

    # mov [d], s
    elif match(inst, "001__0__"):
        if cycle == 1:
            return [U.WrMar] + rd(dst)
        if cycle == 2:
            return [U.WrDmem] + rd(src) + [U.NextInst]

    # jmp dest
    elif match(inst, "01000___"):
        if cycle == 1:
            return [U.JmpEn, U.IncPc, U.NextInst]

    # jcc dest
    elif match(inst, "01001___"):
        if cycle == 1:
            return ([U.JmpEn] if cond_ok else []) + [U.IncPc, U.NextInst]

    # halt & nops
    # (nop2=01, nop3=10, nop4=11 in bits [3:2])
    elif match(inst, "011_____"):
        kind = slice_bits(inst, 4, 3)
        if kind == 0 and cycle == 1:
            return [U.Halt, U.NextInst]
        if kind == 1 and cycle == 1:
            return [U.NextInst]
        if kind == 2 and cycle == 2:
            return [U.NextInst]
        if kind == 3 and cycle == 3:
            return [U.NextInst]

    # alu
    elif match(inst, "1_______"):
        op = slice_bits(inst, 6, 3)
        if not alu_mem:
            if cycle == 1:
                return alu_op(op, alu_rd(alu_src), c)
        else:
            if cycle == 1:
                return [U.WrMar] + alu_rd(alu_src)
            if cycle == 2:
                return alu_op(op, [U.RdDmem], c)

    return []


# encode a list of microinstructions into a 32-bit word


def encode(uinsts: list[U]) -> int:
    word = 0
    for u in ACTIVE_LOW:
        word |= 1 << int(u)

    for u in uinsts:
        if u in ACTIVE_LOW:
            word &= ~(1 << int(u))
        else:
            word |= 1 << int(u)

    return word & 0xFFFFFFFF


# build the full ROM


def decode_address(addr: int) -> tuple[int, int, int]:
    cycle = (addr >> 11) & 0x7
    flags = (addr >> 8) & 0x7
    inst = addr & 0xFF
    return cycle, flags, inst


def build_rom() -> list[int]:
    words = []
    for addr in range(8192):
        cycle, flags, inst = decode_address(addr)
        uinsts = ucode(cycle, flags, inst)
        words.append(encode(uinsts))
    return words


# swizzle the bits for the C array .h files

# fmt: off
BIT_SWIZZLE = {
    0: 7,
    1: 6,
    2: 5,
    3: 4,
    4: 3,
    5: 2,
    6: 1,
    7: 0
}
# fmt: on


def swizzle_byte(b):
    result = 0
    for old, new in BIT_SWIZZLE.items():
        if b & (1 << old):
            result |= 1 << new
    return result


# write one byte-lane ROM file


def write_rom(words, byte_offset):
    lane = byte_offset + 1
    raw = [(w >> (8 * byte_offset)) & 0xFF for w in words]

    # write a .bin file
    with open(f"ucode-{lane}.bin", "wb") as f:
        f.write(bytes(raw))
    print(f"Wrote ucode-{lane}.bin")

    # write a .h file
    swizzled = [swizzle_byte(b) for b in raw]
    with open(f"ucode-{lane}.h", "w") as f:
        f.write(f"#pragma once\n")
        f.write(f"const PROGMEM uint8_t eepromData[] = {{\n")
        for i in range(0, len(swizzled), 16):
            row = swizzled[i : i + 16]
            f.write("    " + ", ".join(f"0x{b:02x}" for b in row) + ",\n")
        f.write("};\n")
    print(f"Wrote ucode-{lane}.h")


if __name__ == "__main__":
    rom = build_rom()
    for lane in range(3):
        write_rom(rom, lane)
