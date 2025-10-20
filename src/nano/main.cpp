#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cinttypes>
#include <cstdlib>
#include <csignal>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <algorithm>

using namespace std;

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

void print_display(uint8_t data[256])
{
    cout << "\033[2J\033[1;1H";

    for (int row = 8; row < 16; ++row)
    {
        for (int col = 0; col < 16; ++col)
        {
            char c = data[row * 16 + col];
            cout << (c == 0 ? ' ' : c);
        }
        cout << '\n';
    }
}

void sigint(int)
{
    cout << "\033[?1049l" << flush;
    exit(0);
}

void compile(ifstream &src, uint8_t *program)
{
    uint8_t curr;
    size_t line_num = 0;
    string line;

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

    while (getline(src, line))
    {
        source_lines.push_back(line);
    }

    line_num = 0;
    for (size_t i = 0; i < source_lines.size(); i++)
    {
        string &raw_line = source_lines[i];
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
                source_lines.insert(source_lines.begin() + i, "nop");
                i++;
                line_num++;
            }

            labels[label_name] = line_num;
        }

        ++line_num;
    }

    line_num = 0;
    for (string &raw_line : source_lines)
    {
        string processed_line = raw_line.substr(0, raw_line.find(';'));
        stringstream ss(processed_line);
        string opcode;
        if (!(ss >> opcode)) continue;

        uint8_t curr = 0;

        if (opcode == "label")
        {
            continue;
        }
        else if (opcode == "nop")
        {
            curr = 0;
        }
        else if (opcode == "mov")
        {
            string rd, rs; int d, s;
            ss >> rd >> rs;
            d = reg_to_num(rd); s = reg_to_num(rs);
            curr = 0b00000000 | (d << 2) | s;
        }
        else if (opcode == "in")
        {
            string rd; int d, s;
            ss >> rd >> s;
            d = reg_to_num(rd);
            curr = 0b00010000 | (d << 2) | s;
        }
        else if (opcode == "out")
        {
            string rs; int d, s;
            ss >> d >> rs;
            s = reg_to_num(rs);
            curr = 0b00100000 | (d << 2) | s;
        }
        else if (opcode == "iml")
        {
            string reg; int val, r;
            ss >> reg >> val;

            if (reg == "ra") r = 0;
            else if (reg == "rb") r = 1;
            else throw runtime_error("Unexpected register: " + reg);

            curr = 0b01000000 | (r << 4) | val;
        }
        else if (opcode == "imh")
        {
            string reg; int val, r;
            ss >> reg >> val;

            if (reg == "ra") r = 0;
            else if (reg == "rb") r = 1;
            else throw runtime_error("Unexpected register: " + reg);

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
        else if (opcode == "jnz")
        {
            string target;
            ss >> target;

            auto it = labels.find(target);
            if (it == labels.end())
            {
                throw runtime_error("Unknown label: " + target);
            }

            uint8_t addr = (it->second) >> 2;
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

    cout << "\033[?1049h" << flush;

    while (true)
    {
        print_display(ram);
        cout << endl;

        cout << "pc: " << setw(3) << (int)pc << " | ";
        cout << "ra: " << setw(3) << (int)*ra << " ";
        cout << "rb: " << setw(3) << (int)*rb << " ";
        cout << "rc: " << setw(3) << (int)*rc << " ";
        cout << "rd: " << setw(3) << (int)*rd << " ";
        cout << endl << flush;

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
            if (op == 0)
                *ra = *ra & reg[s];
            if (op == 1)
                *ra = *ra | reg[s];
            if (op == 2)
                *ra = *ra ^ reg[s];
            if (op == 3)
                *ra = *ra >> 1;
            if (op == 4)
                *ra = *ra + reg[s];
            if (op == 5)
                *ra = *ra - reg[s];
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
            if (*ra != 0) pc = dest;
        }
    }

    cout << "Press any key to continue...";
    getchar();

    cout << "\033[?1049l" << flush;

    cout << endl;

    for (size_t i = 0; i < 256; i++)
    {
        cout << hex(ram[i]) << " ";

        if (i % 16 == 15)
        {
            cout << endl;
        }
    }
}

int main(int argc, char **argv)
{
    signal(SIGINT, sigint);

    srand(time(NULL));

    string input_file;
    bool print_hex = false;
    bool run_program = false;

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "-i") == 0)
        {
            if (i + 1 >= argc)
            {
                cerr << "Error: -i requires a filename\n";
                return 1;
            }
            input_file = argv[++i];
        }
        else if (strcmp(argv[i], "-p") == 0)
        {
            print_hex = true;
        }
        else if (strcmp(argv[i], "-r") == 0)
        {
            run_program = true;
        }
        else
        {
            cerr << "Unknown option: " << argv[i] << "\n";
            cerr << "Usage: " << argv[0] << " -i <file> [-p] [-r]\n";
            return 1;
        }
    }

    if (input_file.empty())
    {
        cerr << "Error: No input file provided. Use -i <file>\n";
        return 1;
    }

    ifstream src(input_file);
    if (!src.is_open())
    {
        cerr << "Error: Failed to open file: " << input_file << "\n";
        return 1;
    }

    uint8_t program[256] = {0};
    compile(src, program);

    if (print_hex)
    {
        cout << endl;
        cout << "Compiled program:" << endl;
        cout << "   ";
        for (uint8_t i = 0; i < 16; i++)
        {
            printf("%02x ", i);
        }
        cout << endl;
        for (uint8_t i = 0; i < 16; i++)
        {
            printf("%02x ", 16 * i);
            for (uint8_t j = 0; j < 16; j++)
            {
                printf("%02x ", program[16 * i + j]);
            }
            cout << endl;
        }
        cout << endl;
    }

    if (run_program)
    {
        run(program);
    }

    return 0;
}