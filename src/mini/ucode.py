def match_pattern(i, pattern):
    pattern = pattern[::-1]
    for bit in range(13):
        if pattern[bit] == 'x':
            continue
        if pattern[bit] == '0' and ((i >> bit) & 1) == 1:
            return False
        if pattern[bit] == '1' and ((i >> bit) & 1) == 0:
            return False
    return True

uinsts_encoding = {}
uinsts_inv = {}

with open("src/mini/uinst.txt", "r") as file:
    text = file.read()
    offset = 0
    
    for line in text.splitlines():
        uinst = line.strip()
        if uinst == "":
            continue
        
        offset = int(offset)
        value = 1 << offset
        
        print(f"{uinst:16} = 1 << {offset}")
        
        if uinst[0] == "!":
            uinst = uinst[1:]
            uinsts_inv[uinst] = value            
        uinsts_encoding[uinst] = value
        offset += 1

uprograms_encoding = [_ for _ in range(8192)]
with open("src/mini/ucode.txt", "r") as file:
    text = file.read()
    uprograms = []
    for line in text.splitlines():
        line = line.split("//", 1)[0]
        line = line.strip()
        if line == "":
            continue
        ls = line.split()
        pattern = ls[0]
        uinsts = ls[1:]
        uprograms.append((pattern, uinsts))
    
    for i in range(8192):
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
                encoding |= uinsts_encoding[uinst]
        
        for value in uinsts_inv.values():
            encoding ^= value
            
        uprograms_encoding[i] = encoding

for offset in range(3):
    with open(f"src/mini/ucode-{offset + 1}.bin", "wb") as bin_file:
        for i in range(8192):
            encoding = uprograms_encoding[i]
            byte = (encoding >> (8 * offset)) & 0xFF
            bin_file.write(byte.to_bytes(1, "little"))