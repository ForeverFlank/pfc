#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cinttypes>

using namespace std;

string hex(uint8_t val)
{
    const char *digits = "0123456789abcdef";
    return string() + digits[val / 16] + digits[val % 16];
}

void compile(ifstream &src, uint8_t *program)
{
    uint8_t curr;
    size_t line_num = 0;
    string line, inst;

    while (true)
    {
        getline(src, line);
        line = line.substr(0, line.find(';'));

        stringstream ss(line);
        if (!(ss >> inst))
        {
            continue;
        }
        curr = 0;
        if (inst == "mov")
        {
            int d, s;
            ss >> d >> s;
            curr = 0 | (d << 2) | s;
        }
        else if (inst == "iml")
        {
            int r, val;
            ss >> r >> val;
            curr = 64 | (r << 4) | val;
        }
        else if (inst == "imh")
        {
            int r, val;
            ss >> r >> val;
            curr = 96 | (r << 4) | val;
        }
        else if (inst == "and")
        {
            int s;
            ss >> s;
            curr = 128 | s;
        }
        else if (inst == "or")
        {
            int s;
            ss >> s;
            curr = 132 | s;
        }
        else if (inst == "xor")
        {
            int s;
            ss >> s;
            curr = 136 | s;
        }
        else if (inst == "shr")
        {
            curr = 140;
        }
        else if (inst == "add")
        {
            int s;
            ss >> s;
            curr = 144 | s;
        }
        else if (inst == "sub")
        {
            int s;
            ss >> s;
            curr = 148 | s;
        }
        else if (inst == "st")
        {
            int s;
            ss >> s;
            curr = 160 | s;
        }
        else if (inst == "ld")
        {
            int d;
            ss >> d;
            curr = 176 | (d << 2);
        }
        else if (inst == "hlt")
        {
            curr = 191;
        }
        else if (inst == "jnz")
        {
            int dest;
            ss >> dest;
            curr = 192 | (dest >> 2);
        }

        program[line_num++] = curr;

        if (src.eof()) break;
    }
}

void run(uint8_t *program)
{
    uint8_t reg[8] = {0}, pc = 0;
    uint8_t ram[256] = {0};
    uint8_t *ra = reg + 0;
    uint8_t *rb = reg + 1;
    uint8_t *rc = reg + 2;
    uint8_t *rd = reg + 3;

    while (true)
    {
        // cout << "pc: " << hex(pc) << " | ";
        // cout << "ra: " << hex(*ra) << " ";
        // cout << "rb: " << hex(*rb) << " ";
        // cout << "rc: " << hex(*rc) << " ";
        // cout << "rd: " << hex(*rd) << " ";
        // cout << endl;

        cout << "pc: " << setw(3) << (int)pc << " | ";
        cout << "ra: " << setw(3) << (int)*ra << " ";
        cout << "rb: " << setw(3) << (int)*rb << " ";
        cout << "rc: " << setw(3) << (int)*rc << " ";
        cout << "rd: " << setw(3) << (int)*rd << " ";
        cout << endl;

        uint8_t inst = program[pc++];

        if ((inst & 0b11000000) == 0b00000000)
        {
            uint8_t d = (inst >> 2) & 3;
            uint8_t s = (inst >> 0) & 3;
            reg[d] = reg[s];
        }
        else if ((inst & 0b11100000) == 0b01000000)
        {
            uint8_t r = (inst >> 4) & 1;
            if (r == 0)
            {
                *ra = (*ra & 0xF0) | (inst & 0x0F);
            }
            else
            {
                *rb = (*rb & 0xF0) | (inst & 0x0F);
            }
        }
        else if ((inst & 0b11100000) == 0b01100000)
        {
            uint8_t r = (inst >> 4) & 1;
            if (r == 0)
            {
                *ra = (*ra & 0x0F) | ((inst & 0x0F) << 4);
            }
            else
            {
                *rb = (*rb & 0x0F) | ((inst & 0x0F) << 4);
            }
        }
        else if ((inst & 0b11100000) == 0b10000000)
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
        else if (inst == 0b10111111)
        {
            break;
        }
        else if ((inst & 0b11110000) == 0b10100000)
        {
            uint8_t s = inst & 3;
            ram[*rd] = reg[s];
        }
        else if ((inst & 0b11110000) == 0b10110000)
        {
            uint8_t d = (inst >> 2) & 3;
            reg[d] = ram[*rd];
        }
        else if ((inst & 0b11000000) == 0b11000000)
        {
            uint8_t dest = (inst & 0x3F) << 2;
            if (*ra != 0) pc = dest;
        }

        for (int i = 0; i < 10000000; i++);
    }

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
    ifstream src(argv[1]);

    uint8_t program[256] = {0};
    compile(src, program);

    cout << endl;
    for (size_t i = 0; i < 256; i++)
    {
        cout << hex(program[i]) << " ";
        if (i % 16 == 15)
        {
            cout << endl;
        }
    }
    
    cout << endl;
    run(program);

    cout << endl;

    return 0;
}