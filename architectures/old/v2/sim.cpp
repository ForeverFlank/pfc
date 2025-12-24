#include <bits/stdc++.h>
#include <windows.h>

#include "compiler.h"

using namespace std;

void printHex(uint8_t value) {
    cout << hex << setfill('0') << setw(2) << static_cast<int>(value);
}

random_device rd;
mt19937 gen(rd());
uniform_int_distribution<> dist(0, 255);

struct ButtonInput {
    ButtonInput() {}

    uint8_t getButtons() {
        uint8_t value = 0;
        if (GetAsyncKeyState('D') & 0x8000) value |= 1;
        if (GetAsyncKeyState('W') & 0x8000) value |= 2;
        if (GetAsyncKeyState('A') & 0x8000) value |= 4;
        if (GetAsyncKeyState('S') & 0x8000) value |= 8;
        value |= dist(gen) & 0xf0;
        return value;
    }
};

struct Display {
    vector<vector<char>> grid;
    uint8_t x, y;
    uint8_t mode;

    bool resetPrint = false;
    COORD cursorPos;

    Display() {
        grid = vector<vector<char>>(8, vector<char>(16, ' '));
        x = 0;
        y = 0;
        mode = 0;
    }

    void update(uint8_t value) {
        if (value == 0xff) {
            grid = vector<vector<char>>(8, vector<char>(16, ' '));
            printGrid();
        } else if ((value & 0b10000000) == 0) {
            grid[y][x] = value & 0b01111111;
            if (mode == 1) {
                x++;
                if (x == 16) {
                    x = 0;
                    y++;
                }
                if (y == 8) {
                    for (int i = 1; i < 8; i++) {
                        grid[i - 1] = grid[i];
                    }
                    grid[7] = vector<char>(16, 0);
                    y = 7;
                }
            }
            printGrid();
        } else {
            if ((value & 0b01000000) == 0) {
                if ((value & 0b00010000) == 0) {
                    x = value & 0x0f;
                } else {
                    y = value & 0x07;
                }
            } else {
                mode = value & 0x0f;
            }
        }
    }

    void init() {
        resetPrint = true;

        for (int row = 0; row < 10; row++) {
            cout << endl;
        }

        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        cursorPos.X = csbi.dwCursorPosition.X;
        cursorPos.Y = csbi.dwCursorPosition.Y - 10;
    }

    void printGrid() {
        if (resetPrint) {
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            SetConsoleCursorPosition(hConsole, cursorPos);
        }

        string printString = "\n";
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 16; col++) {
                printString += grid[row][col];
            }
            printString += "\n";
        }
        printString += "\n";
        cout << printString;
    }
};

struct Computer {
    vector<uint8_t> program;
    vector<uint8_t> ram;
    vector<uint8_t> stack;
    uint8_t pc, sp;
    uint8_t r1, r2, r3, r4, r5, r6;
    bool zf, nf, cf;
    bool prev_set_pc;

    uint8_t inputValue;
    function<uint8_t()> inputFunction;
    function<void(uint8_t)> outputFunction;

    enum Op { None, Mov, Imm, St, Ld, Alu, Jmp, Call, Ret, Halt };

    Computer() {
        program = vector<uint8_t>(256, 0);
        ram = vector<uint8_t>(256, 0);
        stack = vector<uint8_t>(16, 0);
        reset();
    }

    void reset() {
        ram = vector<uint8_t>(256, 0);
        stack = vector<uint8_t>(16, 0);
        pc = sp = 0;
        r1 = r2 = r3 = r4 = r5 = r6 = 0;
        zf = nf = cf = false;
        prev_set_pc = false;
    }

    void bindInput(function<uint8_t()> func) { inputFunction = func; }

    void bindOutput(function<void(uint8_t)> func) { outputFunction = func; }

    void memoryDump() {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        COORD cursorPos;
        cursorPos.X = cursorPos.Y = 0;
        SetConsoleCursorPosition(hConsole, cursorPos);

        string printString = "\n";
        for (int i = 0; i < 32; i++) {
            stringstream line;
            line << " ";
            line << hex << setfill('0') << setw(2) << (i * 8);
            line << " | ";
            for (int j = 0; j < 8; j++) {
                line << hex << setfill('0') << setw(2)
                     << static_cast<int>(ram[8 * i + j]) << " ";
            }
            printString += line.str() + "\n";
        }
        cout << printString;
    }

    Op decode(uint8_t instruction) {
        if (instruction == 0xff) return Op::Halt;
        switch (instruction & 0b11000000) {
            case 0b00000000:
                return Op::Mov;
            case 0b01000000:
                return Op::Imm;
            case 0b10000000:
                if ((instruction & 0b00110000) != 0) return Op::Alu;
                if ((instruction & 0b00001000) != 0) return Op::Ld;
                return Op::St;
            case 0b11000000:
                if (instruction == 0b11111001) return Op::Call;
                if (instruction == 0b11111000) return Op::Ret;
                return Op::Jmp;
            default:
                return Op::None;
        }
        return Op::None;
    }

    void loadProgram(vector<uint8_t>& programCode) {
        copy(programCode.begin(), programCode.end(), program.begin());
    }

    uint8_t getRegister(uint8_t reg) {
        switch (reg) {
            case 0:
                return 0;
            case 1:
                return r1;
            case 2:
                return r2;
            case 3:
                return r3;
            case 4:
                return r4;
            case 5:
                return r5;
            case 6:
                return r6;
            case 7:
                return inputFunction();
            default:
                break;
        }
        return 0;
    }

    void setRegister(uint8_t reg, uint8_t value) {
        switch (reg) {
            case 1:
                r1 = value;
                break;
            case 2:
                r2 = value;
                break;
            case 3:
                r3 = value;
                break;
            case 4:
                r4 = value;
                break;
            case 5:
                r5 = value;
                break;
            case 6:
                r6 = value;
                break;
            case 7:
                outputFunction(value);
                break;
            default:
                break;
        }
    }

    void mov(uint8_t instruction) {
        uint8_t src, dest, value;
        src = instruction & 0b00000111;
        dest = (instruction & 0b00111000) >> 3;
        value = getRegister(src);
        setRegister(dest, value);
    }

    void imm(uint8_t instruction) {
        uint8_t value = instruction & 0b00001111;
        bool dest = (instruction & 0b00100000) >> 5;
        bool h = (instruction & 0b00010000) >> 4;
        if (!dest) {
            if (!h) {
                r1 = (r1 & 0b11110000) | value;
            } else {
                r1 = (r1 & 0b00001111) | (value << 4);
            }
        } else {
            if (!h) {
                r2 = (r2 & 0b11110000) | value;
            } else {
                r2 = (r2 & 0b00001111) | (value << 4);
            }
        }
    }

    void st(uint8_t instruction) {
        uint8_t src = instruction & 0b00000111;
        uint8_t value = getRegister(src);
        ram[r2] = value;
    }
    
    void ld(uint8_t instruction) {
        uint8_t dest = instruction & 0b00000111;
        setRegister(dest, ram[r2]);
    }

    void alu(uint8_t instruction) {
        uint8_t op = (instruction & 0b00111000) >> 3;
        uint8_t src = instruction & 0b00000111;
        uint8_t value = getRegister(src);
        switch (op) {
            case 2:
                cf = (r1 + value > 0xff);
                r1 = r1 + value;
                break;
            case 3:
                cf = (r1 < value);
                r1 = r1 - value;
                break;
            case 4:
                cf = false;
                r1 = r1 & value;
                break;
            case 5:
                cf = false;
                r1 = r1 | value;
                break;
            case 6:
                cf = false;
                r1 = r1 ^ value;
                break;
            case 7:
                cf = r1 & 1;
                r1 = r1 >> 1;
                break;
            default:
                break;
        }
        zf = (r1 == 0);
        nf = r1 & 0b10000000;
    }

    bool jmp(uint8_t instruction) {
        uint8_t cond = (instruction & 0b00011000) >> 3;
        bool neg = instruction & 0b00100000;
        bool res;
        switch (cond) {
            case 0:
                res = zf;
                break;
            case 1:
                res = nf;
                break;
            case 2:
                res = cf;
                break;
            case 3:
                res = true;
                break;
        }
        return res ^ neg;
    }

    void call(uint8_t instruction) {
        stack[sp] = pc + 2;
        sp++;
    }

    uint8_t ret(uint8_t instruction) {
        sp--;
        uint8_t top = stack[sp];
        return top;
    }

    bool execute(int delayMicroseconds, int verbose) {
        auto start = chrono::high_resolution_clock::now();

        uint8_t instruction = program[pc];

        bool set_pc = false;
        uint8_t next_pc = pc + 1;

        if (prev_set_pc) {
            next_pc = instruction;
        } else {
            Op op = decode(instruction);
            switch (op) {
                case Op::Mov:
                    mov(instruction);
                    break;
                case Op::Imm:
                    imm(instruction);
                    break;
                case Op::St:
                    st(instruction);
                    break;
                case Op::Ld:
                    ld(instruction);
                    break;
                case Op::Alu:
                    alu(instruction);
                    break;
                case Op::Jmp:
                    set_pc = jmp(instruction);
                    if (!set_pc) {
                        next_pc = pc + 2;
                    }
                    break;
                case Op::Call:
                    call(instruction);
                    set_pc = true;
                    break;
                case Op::Ret:
                    next_pc = ret(instruction);
                    break;
                case Op::Halt:
                    return false;
                case Op::None:
                default:
                    break;
            }
        }

        if (verbose & 4) {
            cout << " ";
            printHex(pc);
            cout << " ";
            printHex(instruction);
            cout << " | ";
            uint8_t regs[] = {r1, r2, r3, r4, r5, r6};
            for (uint8_t reg : regs) {
                printHex(reg);
                cout << " ";
            }
            cout << "| ";
            cout << (zf ? 1 : 0);
            cout << (nf ? 1 : 0);
            cout << (cf ? 1 : 0);
            cout << endl;
        }

        if (verbose & 8) {
            memoryDump();
        }

        pc = next_pc;
        prev_set_pc = set_pc;

        while (chrono::high_resolution_clock::now() - start <
               chrono::microseconds(delayMicroseconds));

        return true;
    }

    void loop(int delayMicroseconds, int verbose) {
        while (execute(delayMicroseconds, verbose)) {
        };
    }
};

void printInputFile(string& inputFilename, string& programString) {
    cout << "Input file (" << inputFilename << "):" << endl;
    stringstream ss(programString);
    string line;
    int lineNumber = 1;
    while (getline(ss, line)) {
        cout << setw(3) << setfill(' ') << lineNumber << " | " << line << endl;
        lineNumber++;
    }
    cout << endl;
}

void printOutputFile(string& outputFilename, vector<uint8_t>& program) {
    cout << "Output hex machine code (" << outputFilename << "):" << endl;
    for (int i = 0; i < 32; i++) {
        cout << " ";
        printHex(i * 8);
        cout << " | ";
        for (int j = 0; j < 8; j++) {
            printHex(program[8 * i + j]);
            cout << " ";
        }
        cout << endl;
    }
    cout << endl;
}

int main(int argc, char* argv[]) {
    system("cls");
    // cout.sync_with_stdio(false);

    int verbose = 0;
    string inputFilename;
    string outputFilename;
    int clockSpeed = 1000;

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];

        if (arg == "--verbose" && i + 1 < argc) {
            verbose = stoi(argv[++i]);
        } else if (arg == "-i" && i + 1 < argc) {
            inputFilename = argv[++i];
        } else if (arg == "-o" && i + 1 < argc) {
            outputFilename = argv[++i];
        } else if (arg == "-c" && i + 1 < argc) {
            clockSpeed = stoi(argv[++i]);
        }
    }

    if (inputFilename.empty()) {
        cout << "Error: Input filename is empty";
        return 1;
    }

    ifstream file(inputFilename);
    stringstream buffer;
    buffer << file.rdbuf();
    string programString = buffer.str();
    if (verbose & 1) printInputFile(inputFilename, programString);

    vector<uint8_t> program = compileAssembly(programString);

    if (!outputFilename.empty()) {
        ofstream outFile(outputFilename, ios::binary);
        outFile.write(reinterpret_cast<const char*>(program.data()),
                      program.size());
        outFile.close();
    }
    if (verbose & 2) printOutputFile(outputFilename, program);

    if (verbose & 4) {
        cout << "CPU Emulation:" << endl;
        cout << " pc in   r1 r2 r3 r4 r5 r6   znc " << endl;
    }

    Computer computer;
    ButtonInput buttonInput;
    Display display;

    if (verbose & 8) {
        for (int i = 0; i < 34; ++i) cout << endl;
    }
    if ((verbose & 4) == 0) display.init();

    computer.loadProgram(program);
    computer.bindInput(
        [&buttonInput]() -> uint8_t { return buttonInput.getButtons(); });
    computer.bindOutput([&display](uint8_t x) { display.update(x); });
    computer.loop(1000000 / clockSpeed, verbose);
    return 0;
}