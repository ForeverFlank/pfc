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

uinsts_encoding = {}

with open("src/mini_v3/uinst.txt", "r") as file:
    text = file.read()
    for line in text.splitlines():
        (uinst, offset, value) = line.split()
        offset = int(offset)
        value = int(value)
        uinsts_encoding[uinst] = value << offset

uprograms_encoding = [_ for _ in range(256)]
with open("src/mini_v3/ucode.txt", "r") as file:
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

        uprograms_encoding[i] = encoding

for offset in range(2):
    with open(f"src/mini_v3/ucode-{offset + 1}.bin", "wb") as bin_file:
        for i in range(256):
            encoding = uprograms_encoding[i]
            byte = (encoding >> (8 * offset)) & 0xFF
            bin_file.write(byte.to_bytes(1, "little"))