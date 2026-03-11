function matchBinary(value, pattern) {
    for (let i = 0; i < 8; ++i) {
        const ch = pattern[7 - i];
        const bit = (value >> i) & 1;
        if (ch === 'x') continue;
        if (ch === '0' && bit === 1) return false;
        if (ch === '1' && bit === 0) return false;
    }
    return true;
}

class CPU {
    constructor() {
        this.pmem = new Uint8Array(256);
        this.reset();
    }

    reset() {
        this.dmem = new Uint8Array(256);
        this.state = 0;
        this.pc = 0;
        this.ir = 0;
        this.mar = 0;
        this.regA = 0;
        this.regB = 0;
        this.regC = 0;
        this.regD = 0;
    }

    step() {
        switch (this.state) {
            case 0:
                step0();
                break;
            case 1:
                step1();
                break;
            case 2:
                step2();
                break;
            default:
                break;
        }
        updateUI();
    }

    step0() {
        this.ir = this.pmem[this.pc++];
        this.state = 1;
    }

    step1() {
        let halt = false;
        if (matchBinary(this.ir, "0000xxxx")) {
            return;
        }
        if (matchBinary(this.ir, "0001xxxx")) {
            halt = true;
        }
        if (matchBinary(this.ir, "0010xxxx")) {
            const perm = this.ir & 15;
            if (perm === 0x0) this.regA = this.regB;
            if (perm === 0x1) this.regA = this.regC;
            if (perm === 0x2) this.regA = this.regD;
            if (perm === 0x3) this.regB = this.regA;
            if (perm === 0x4) this.regB = this.regC;
            if (perm === 0x5) this.regB = this.regD;
            if (perm === 0x6) this.regC = this.regA;
            if (perm === 0x7) this.regC = this.regB;
            if (perm === 0x8) this.regC = this.regD;
            if (perm === 0x9) this.regD = this.regA;
            if (perm === 0xa) this.regD = this.regB;
            if (perm === 0xb) this.regD = this.regC;
            if (perm === 0xc) this.regA = this.pmem[this.pc++];
            if (perm === 0xd) this.regB = this.pmem[this.pc++];
            if (perm === 0xe) this.regC = this.pmem[this.pc++];
            if (perm === 0xf) this.regD = this.pmem[this.pc++];
        }
        if (matchBinary(this.ir, "0100xxxx")) {
            const ra = (this.ir & 12) >> 2;
            if (ra === 0) this.mar = this.regA;
            if (ra === 1) this.mar = this.regB;
            if (ra === 2) this.mar = this.regC;
            if (ra === 3) this.mar = this.regD;
        }

        if (this.halt) return;
        this.state = 2;
    }

    step2() {
        this.state = 0;
    }

    updateUI() {
    }
}

const cpu = new CPU();

document.getElementById("btn-run")
        .addEventListener("click", () => {});

document.getElementById("btn-step")
        .addEventListener("click", cpu.step);

document.getElementById("btn-reset")
        .addEventListener("click", cpu.reset);
