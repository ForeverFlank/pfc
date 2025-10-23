#include <bits/stdc++.h>
#include <windows.h>

using namespace std;

bool interrupt = false;
void sigint(int)
{
    system("cls");
    interrupt = true;
}

string hex(uint8_t val)
{
    const char *digits = "0123456789abcdef";
    return string() + digits[val / 16] + digits[val % 16];
}

bool match(uint8_t val, string pattern)
{
    for (size_t i = 0; i < 8; i++)
    {
        uint8_t b = (val >> i) & 1;
        char c = pattern[7 - i];
        if (c == 'x') continue;
        if (c == '0' && b != 0) return false;
        if (c == '1' && b != 1) return false;
    }
    return true;
}

uint8_t get_wasd_inputs()
{
    uint8_t value = 0;
    if (GetAsyncKeyState('W') & 0x8000) value |= 1;
    if (GetAsyncKeyState('S') & 0x8000) value |= 2;
    if (GetAsyncKeyState('D') & 0x8000) value |= 4;
    if (GetAsyncKeyState('A') & 0x8000) value |= 8;
    return value;
}

void clear_screen()
{
    static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD topLeft = {0, 0};
    SetConsoleCursorPosition(hOut, topLeft);
}

void compile(ifstream &src, uint8_t *program)
{
    auto reg_to_num = [] (string r) -> int
        {
            if (r == "ra") return 0;
            if (r == "rb") return 1;
            if (r == "rc") return 2;
            if (r == "rd") return 3;
            throw runtime_error("Unexpected register: " + r);
        };

    unordered_map<string, uint8_t> labels;
    vector<string> source_lines;
    string line;

    while (getline(src, line))
    {
        source_lines.push_back(line);
    }

    size_t line_num = 0;
    vector<string> aligned_lines;

    for (size_t i = 0; i < source_lines.size(); ++i)
    {
        string raw_line = source_lines[i];
        string processed_line = raw_line.substr(0, raw_line.find(';'));
        stringstream ss(processed_line);
        string opcode;
        if (!(ss >> opcode)) continue;

        if (opcode == "label")
        {
            string label_name;
            ss >> label_name;

            while (line_num % 4 != 0)
            {
                aligned_lines.push_back("nop");
                ++line_num;
            }

            labels[label_name] = static_cast<uint8_t>(line_num);
            continue;
        }

        aligned_lines.push_back(raw_line);
        ++line_num;
    }

    line_num = 0;
    for (string &raw_line : aligned_lines)
    {
        string processed_line = raw_line.substr(0, raw_line.find(';'));
        stringstream ss(processed_line);
        string opcode;
        if (!(ss >> opcode)) continue;

        uint8_t curr = 0;

        if (opcode == "nop") curr = 0;
        else if (opcode == "mov")
        {
            string rd, rs;
            ss >> rd >> rs;
            int d = reg_to_num(rd), s = reg_to_num(rs);
            curr = 0b00000000 | (d << 2) | s;
        }
        else if (opcode == "in")
        {
            string rd; int s;
            ss >> rd >> s;
            int d = reg_to_num(rd);
            curr = 0b00010000 | (d << 2) | s;
        }
        else if (opcode == "out")
        {
            string rs; int d;
            ss >> d >> rs;
            int s = reg_to_num(rs);
            curr = 0b00100000 | (d << 2) | s;
        }
        else if (opcode == "iml")
        {
            string reg; int val;
            ss >> reg >> val;
            int r = reg_to_num(reg);
            curr = 0b01000000 | (r << 4) | val;
        }
        else if (opcode == "imh")
        {
            string reg; int val;
            ss >> reg >> val;
            int r = reg_to_num(reg);
            curr = 0b01100000 | (r << 4) | val;
        }
        else if (opcode == "and")
        {
            string rs; ss >> rs;
            curr = 0b10000000 | reg_to_num(rs);
        }
        else if (opcode == "or")
        {
            string rs; ss >> rs;
            curr = 0b10000100 | reg_to_num(rs);
        }
        else if (opcode == "xor")
        {
            string rs; ss >> rs;
            curr = 0b10001000 | reg_to_num(rs);
        }
        else if (opcode == "shr")
        {
            curr = 0b10001100;
        }
        else if (opcode == "add")
        {
            string rs; ss >> rs;
            curr = 0b10010000 | reg_to_num(rs);
        }
        else if (opcode == "sub")
        {
            string rs; ss >> rs;
            curr = 0b10010100 | reg_to_num(rs);
        }
        else if (opcode == "adc")
        {
            string rs; ss >> rs;
            curr = 0b10011000 | reg_to_num(rs);
        }
        else if (opcode == "sbc")
        {
            string rs; ss >> rs;
            curr = 0b10011100 | reg_to_num(rs);
        }
        else if (opcode == "st")
        {
            string rs; ss >> rs;
            curr = 0b10100000 | reg_to_num(rs);
        }
        else if (opcode == "ld")
        {
            string rd; ss >> rd;
            curr = 0b10110000 | (reg_to_num(rd) << 2);
        }
        else if (opcode == "hlt")
        {
            curr = 0b10111111;
        }
        else if (opcode == "jz")
        {
            string target;
            ss >> target;
            auto it = labels.find(target);
            if (it == labels.end())
            {
                throw runtime_error("Unknown label: " + target);
            }
            uint8_t addr = it->second >> 2;
            curr = 0b11000000 | (addr & 0b00111111);
        }
        else
        {
            throw runtime_error("Unexpected opcode: " + opcode);
        }

        program[line_num++] = curr;
    }
}

void run(uint8_t *program)
{
    uint8_t reg[4] = {0}, io[4] = {0}, pc = 0;
    uint8_t ram[256] = {0};
    uint8_t *ra = reg + 0;
    uint8_t *rb = reg + 1;
    uint8_t *rc = reg + 2;
    uint8_t *rd = reg + 3;

    system("cls");
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    cursorInfo.bVisible = 0;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);

    size_t framenumber = 0;
    while (true && !interrupt)
    {
        clear_screen();

        std::ostringstream oss;
        for (int row = 8; row < 16; row++)
        {
            for (int col = 0; col < 16; col++)
            {
                char c = ram[row * 16 + col];
                oss << (c == 0 ? ' ' : c);
            }
            oss << '\n';
        }

        oss << "pc: " << setw(2) << hex << (int)pc << "\t";
        oss << "ra: " << setw(2) << hex << (int)*ra << "\t";
        oss << "rb: " << setw(2) << hex << (int)*rb << "\t";
        oss << "rc: " << setw(2) << hex << (int)*rc << "\t";
        oss << "rd: " << setw(2) << hex << (int)*rd << "\t";
        oss << '\n';

        cout << oss.str();

        io[0] = get_wasd_inputs();
        io[1] = rand() % 256;

        uint8_t inst = program[pc++];

        if (match(inst, "0000xxxx"))
        {
            uint8_t d = (inst >> 2) & 3;
            uint8_t s = (inst >> 0) & 3;
            reg[d] = reg[s];
        }
        else if (match(inst, "0001xxxx"))
        {
            uint8_t d = (inst >> 2) & 3;
            uint8_t s = (inst >> 0) & 3;
            reg[d] = io[s];
        }
        else if (match(inst, "0010xxxx"))
        {
            uint8_t d = (inst >> 2) & 3;
            uint8_t s = (inst >> 0) & 3;
            io[d] = reg[s];
        }
        else if (match(inst, "01xxxxxx"))
        {
            uint8_t r = (inst >> 4) & 1;
            uint8_t h = (inst >> 5) & 1;
            if (r == 0)
            {
                *ra = h == 0
                    ? (*ra & 0xF0) | (inst & 0x0F)
                    : (*ra & 0x0F) | ((inst & 0x0F) << 4);
            }
            else
            {
                *rb = h == 0
                    ? (*rb & 0xF0) | (inst & 0x0F)
                    : (*rb & 0x0F) | ((inst & 0x0F) << 4);
            }
        }
        else if (match(inst, "100xxxxx"))
        {
            uint8_t s = inst & 3;
            uint8_t op = (inst >> 2) & 7;
            if (op == 0) *ra = *ra & reg[s];
            if (op == 1) *ra = *ra | reg[s];
            if (op == 2) *ra = *ra ^ reg[s];
            if (op == 3) *ra = *ra >> 1;
            if (op == 4) *ra = *ra + reg[s];
            if (op == 5) *ra = *ra - reg[s];
        }
        else if (match(inst, "10111111"))
        {
            break;
        }
        else if (match(inst, "1010xxxx"))
        {
            uint8_t s = inst & 3;
            ram[*rb] = reg[s];
        }
        else if (match(inst, "1011xxxx"))
        {
            uint8_t d = (inst >> 2) & 3;
            reg[d] = ram[*rb];
        }
        else if (match(inst, "11xxxxxx"))
        {
            uint8_t dest = (inst & 0b00111111) << 2;
            if (*ra == 0) pc = dest;
        }

        if (framenumber % 256 == 0)
        {
            this_thread::sleep_for(chrono::milliseconds(1));
        }
        framenumber++;

        // getchar();
    }

    cout << "Press any key to continue...";
    getchar();

    for (size_t i = 0; i < 256; i++)
    {
        cout << hex(ram[i]) << " ";
        if (i % 16 == 15) cout << endl;
    }
}

int main(int argc, char **argv)
{
    signal(SIGINT, sigint);

    srand(time(NULL));

    if (argc < 2)
    {
        cerr << "Usage: " << argv[0] << " <source> [-p]" << endl;
        return 1;
    }

    bool dump_program = false;
    string filename;

    for (int i = 1; i < argc; ++i)
    {
        string arg = argv[i];
        if (arg == "-p")
        {
            dump_program = true;
        }
        else
        {
            filename = arg;
        }
    }

    if (filename.empty())
    {
        cerr << "No source file provided." << endl;
        return 1;
    }

    ifstream src(filename);
    if (!src)
    {
        cerr << "Error opening file: " << filename << endl;
        return 1;
    }

    uint8_t program[256] = {0};
    compile(src, program);

    if (dump_program)
    {
        cout << "Compiled program dump:\n";
        for (int i = 0; i < 256; ++i)
        {
            cout << hex << setw(2) << setfill('0') << (int)program[i] << " ";
            if (i % 16 == 15) cout << endl;
        }
        return 0;
    }

    run(program);

    return 0;
}