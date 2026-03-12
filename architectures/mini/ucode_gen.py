"""
🗣🗣🗣 NOTE FROM HUMAN 🗣🗣🗣:
I originally wrote this in Haskell. However, something went wrong and
I didn't dare to debug it, so I ask Claude to convert it to Python.
"""

#!/usr/bin/env python3

"""
Microcode ROM generator.
Produces 3 binary files: ucode-1-hs.bin, ucode-2-hs.bin, ucode-3-hs.bin
Each ROM address is 13 bits: [cycle(3)] [flags(3)] [inst(8)]
Each entry is a 32-bit word (split across the 3 ROM chips by byte offset).
"""

import struct
from enum import IntEnum

# ---------------------------------------------------------------------------
# Micro-instruction bits
# ---------------------------------------------------------------------------

class U(IntEnum):
    IncPc          =  0
    JmpEn          =  1
    NextInst       =  2
    Halt           =  3
    WrA            =  4
    WrB            =  5
    WrC            =  6
    WrMar          =  7
    WrDmem         =  8
    RdA            =  9
    RdB            = 10
    RdC            = 11
    RdTmp          = 12
    RdDmem         = 13
    RdPmem         = 14
    AluEn          = 15
    AluDisableA    = 16
    AluCin         = 17
    AluInvB        = 18
    AluSelAdder    = 19
    AluS0          = 20
    AluS1          = 21
    AluShr         = 22
    DontUpdateFlags = 23

# Bits that are active-low (default 1, asserted by clearing to 0)
ACTIVE_LOW = {
    U.JmpEn, U.NextInst, U.Halt,
    U.WrA, U.WrB, U.WrC, U.WrMar, U.WrDmem,
    U.RdA, U.RdB, U.RdC, U.RdTmp, U.RdDmem, U.RdPmem,
    U.AluEn, U.AluDisableA, U.DontUpdateFlags,
}

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def slice_bits(val, high, low):
    """Extract bits [high:low] inclusive."""
    width = high - low + 1
    mask = (1 << width) - 1
    return (val >> low) & mask

def match(val, pattern):
    """
    Match val against a bit pattern string (MSB first).
    '0'/'1' match exactly; any other char is a wildcard.
    """
    for i, ch in enumerate(reversed(pattern)):
        bit = (val >> i) & 1
        if ch == '0' and bit != 0:
            return False
        if ch == '1' and bit != 1:
            return False
    return True

def wr(reg):
    """Write-enable signals for register r."""
    return {0: [U.WrA], 1: [U.WrB], 2: [U.WrC]}.get(reg, [])

def rd(reg):
    """Read signals for register r (r=3 means immediate from program memory)."""
    return {
        0: [U.RdA],
        1: [U.RdB],
        2: [U.RdC],
        3: [U.RdPmem, U.IncPc],
    }.get(reg, [])

def alu_rd(reg):
    """Read signals for ALU source (includes memory-reference variants 5-7)."""
    return {
        0: [U.RdA],
        1: [U.RdB],
        2: [U.RdC],
        3: [U.RdPmem, U.IncPc],
        4: [],
        5: [U.RdB],
        6: [U.RdC],
        7: [U.RdPmem, U.IncPc],
    }.get(reg, [])

# ---------------------------------------------------------------------------
# Microcode table  (cycle, flags, inst) -> list of UInsts asserted this cycle
# ---------------------------------------------------------------------------

def ucode(cycle, flags, inst):
    if not (0 <= cycle < 4 and 0 <= flags < 8 and 0 <= inst < 256):
        raise ValueError("Invalid entry")

    # Decoded fields (named for clarity)
    src = slice_bits(inst, 1, 0)
    dst = slice_bits(inst, 4, 3)

    # Flag bits
    z = bool(flags & (1 << 0))
    n = bool(flags & (1 << 1))
    c = bool(flags & (1 << 2))

    # Conditional jump condition codes
    cond = slice_bits(inst, 2, 0)
    cond_ok = [
        z,                  # 0: jz / je
        n,                  # 1: jn / jl
        c,                  # 2: jc
        z or n,             # 3: jle
        not z,              # 4: jnz / jne
        not n,              # 5: jnn / jge
        not c,              # 6: jnc
        not z and not n,    # 7: jg
    ][cond]

    # ALU fields
    alu_src    = slice_bits(inst, 2, 0)
    alu_mem    = alu_src >= 5           # memory-reference mode

    def alu_op():
        op = slice_bits(inst, 6, 3)
        if op == 0:
            return [U.AluEn, U.AluSelAdder]
        if op == 1:
            return [U.AluEn, U.AluSelAdder] + ([U.AluCin] if c else [])
        return []

    alu_end = [U.WrA, U.RdTmp, U.NextInst]

    # ------------------------------------------------------------------
    # Fetch (cycle 0 is always the same)
    # ------------------------------------------------------------------
    if cycle == 0:
        return [U.IncPc]

    # ------------------------------------------------------------------
    # mov d, s        000__0__  (dst != 3)
    # ------------------------------------------------------------------
    if match(inst, "000__0__") and dst != 3:
        if cycle == 1:
            return wr(src) + rd(dst) + [U.NextInst]

    # ------------------------------------------------------------------
    # mov d, [s]      000__1__  (dst != 3)
    # ------------------------------------------------------------------
    elif match(inst, "000__1__") and dst != 3:
        if cycle == 1: return [U.WrMar] + rd(src)
        if cycle == 2: return wr(src) + [U.RdDmem, U.NextInst]

    # ------------------------------------------------------------------
    # mov [d], s      001__0__
    # ------------------------------------------------------------------
    elif match(inst, "001__0__"):
        if cycle == 1: return [U.WrMar] + rd(dst)
        if cycle == 2: return [U.WrDmem] + rd(src) + [U.NextInst]

    # ------------------------------------------------------------------
    # xchg d, s       010__0__  (dst != 3, src != 3)
    # ------------------------------------------------------------------
    elif match(inst, "010__0__") and dst != 3 and src != 3:
        if cycle == 1: return [U.AluEn, U.AluDisableA, U.AluSelAdder, U.DontUpdateFlags] + rd(dst)
        if cycle == 2: return wr(dst) + rd(src)
        if cycle == 3: return wr(src) + [U.RdTmp, U.NextInst]

    # ------------------------------------------------------------------
    # jcc dest        01100___
    # ------------------------------------------------------------------
    elif match(inst, "01100___"):
        if cycle == 1:
            return ([U.JmpEn] if cond_ok else []) + [U.IncPc, U.NextInst]

    # ------------------------------------------------------------------
    # jmp dest        01101___
    # ------------------------------------------------------------------
    elif match(inst, "01101___"):
        if cycle == 1:
            return [U.JmpEn, U.IncPc, U.NextInst]

    # ------------------------------------------------------------------
    # halt            011100__
    # ------------------------------------------------------------------
    elif match(inst, "011100__"):
        if cycle == 1:
            return [U.Halt, U.NextInst]

    # ------------------------------------------------------------------
    # nop variants    0111____
    # (nop2=01, nop3=10, nop4=11 in bits [3:2])
    # ------------------------------------------------------------------
    elif match(inst, "0111____"):
        nop_kind = slice_bits(inst, 3, 2)
        if nop_kind == 1 and cycle == 1: return [U.NextInst]
        if nop_kind == 2 and cycle == 2: return [U.NextInst]
        if nop_kind == 3 and cycle == 3: return [U.NextInst]

    # ------------------------------------------------------------------
    # add s           10000___
    # ------------------------------------------------------------------
    elif match(inst, "10000___"):
        if not alu_mem:
            if cycle == 1: return alu_op() + alu_rd(alu_src)
            if cycle == 2: return alu_end
        else:
            if cycle == 1: return [U.WrMar] + alu_rd(alu_src)
            if cycle == 2: return alu_op() + [U.RdDmem]
            if cycle == 3: return alu_end

    return []

# ---------------------------------------------------------------------------
# Encode a list of UInsts into a 32-bit word
# ---------------------------------------------------------------------------

def encode(uinsts):
    # Start with all active-low bits SET (deasserted)
    word = 0
    for u in ACTIVE_LOW:
        word |= (1 << int(u))

    for u in uinsts:
        if u in ACTIVE_LOW:
            word &= ~(1 << int(u))   # assert by clearing
        else:
            word |=  (1 << int(u))   # assert by setting

    return word & 0xFFFFFFFF

# ---------------------------------------------------------------------------
# Build the full ROM (8192 entries)
# ---------------------------------------------------------------------------

def decode_address(addr):
    cycle = (addr >> 11) & 0x7
    flags = (addr >>  8) & 0x7
    inst  =  addr        & 0xFF
    return cycle, flags, inst

def build_rom():
    words = []
    for addr in range(8192):
        cycle, flags, inst = decode_address(addr)
        uinsts = ucode(cycle, flags, inst)
        words.append(encode(uinsts))
    return words

# ---------------------------------------------------------------------------
# Write one byte-lane ROM file
# ---------------------------------------------------------------------------

def write_rom(words, byte_offset):
    filename = f"ucode-{byte_offset + 1}.bin"
    data = bytes((w >> (8 * byte_offset)) & 0xFF for w in words)
    with open(filename, "wb") as f:
        f.write(data)
    print(f"Wrote {filename}  ({len(data)} bytes)")

# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

if __name__ == "__main__":
    rom = build_rom()
    for lane in range(3):
        write_rom(rom, lane)
