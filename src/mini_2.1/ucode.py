def match_pattern(i, pattern):
    pattern = pattern[::-1]
    for bit in range(8):
        if pattern[bit] == 'x':
            continue
        if pattern[bit] == '0' and ((i >> bit) & 1) == 1:
            return False
        if pattern[bit] == '1' and ((i >> bit) & 1) == 0:
            return False
    return True

uinsts_list = [
    "",
    "wr_a",
    "wr_b",
    "wr_c",
    "wr_d",
    "wr_mem",
    "rd_a",
    "rd_b",
    "rd_c",
    "rd_d",
    "rd_mem",
    "rd_opnd",
    "addr_a",
    "addr_b",
    "addr_c",
    "addr_d",
    "addr_opnd",
    "alu",
    "st_c",
    "st_o",
    "jmp",
    "jmp_cond",
    "halt"
]

uinsts_encoding = {}
for i in range(len(uinsts_list)):
    uinsts_encoding[uinsts_list[i]] = 1 << i

two_bytes_insts = [
    "0011xxxx",
    "01x1xxxx", 
    "1010xxxx",
    "10110xxx",
    "101110xx",
    "11xxxxxx"
]

uprograms_encoding = [_ for _ in range(256)]
with open("src/mini_2.1/ucode.txt", "r") as file:
    text = file.read()
    
    uprograms = []
    for line in text.splitlines():
        ls = line.split()
        pattern = ls[0]
        uinsts = ls[1:]
        uprograms.append((pattern, uinsts))
    
    
    for i in range(256):
        encoding = 0
        
        for (pattern, uinsts) in uprograms:
            if not match_pattern(i, pattern):
                continue
            
            for uinst in uinsts:
                if uinst[-2:] in ["10", "32"]:
                    if uinst[-2:] == "10":
                        idx = i & 0x03
                    else:
                        idx = (i & 0x0c) >> 2
                    suffix = ['a', 'b', 'c', 'd'][idx]
                    uinst = uinst[:-2] + suffix
                encoding += uinsts_encoding[uinst]
        
        if any(map(lambda p: match_pattern(i, p), two_bytes_insts)):
            encoding |= 1
        
        uprograms_encoding[i] = encoding
    
    # for encoding in uprograms_encoding:
    #     print(f"{encoding:032b}")
    
# with open("src/mini_2.1/ucode.bin", "wb") as bin_file:
#     for i in range(4):
#         for encoding in uprograms_encoding:
#             byte = (encoding >> (8 * i)) & 0xFF
#             bin_file.write(byte.to_bytes(1, 'little'))

for offset in range(3):
    with open(f"src/mini_2.1/ucode-{offset + 1}.hex", "w") as bin_file:
        for i in range(256):
            encoding = uprograms_encoding[i]
            byte = (encoding >> (8 * offset)) & 0xFF
            bin_file.write(f"{byte:02x} ")
            if i % 16 == 15:
                bin_file.write("\n")