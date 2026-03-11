const reg = ["a", "b", "c", "d"];

let opcodeTable = [];

for (let x = 0; x < 8; x++)
    opcodeTable.push("nop")

for (let x = 0; x < 8; x++)
    opcodeTable.push("hlt")

for (let d = 0; d < 4; d++)
    for (let s = 0; s < 4; s++)
        opcodeTable.push("mov " + reg[d] + ", " + reg[s])

for (let x = 0; x < 4; x++)
    for (let s = 0; s < 4; s++)
        opcodeTable.push("ldi " + reg[s] + ", imm")

opcodeTable.push("jmp")
