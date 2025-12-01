#include <bits/stdc++.h>

using namespace std;


string trim(string &s)
{
    auto start = find_if_not(s.begin(), s.end(), [] (unsigned char ch)
        {
            return isspace(ch);
        });
    auto end = find_if_not(s.rbegin(), s.rend(), [] (unsigned char ch)
        {
            return isspace(ch);
        }).base();

    if (start >= end) return "";
    return string(start, end);
}

string removeComment(string s)
{
    std::size_t pos = s.find(';');
    if (pos != std::string::npos)
    {
        s.erase(pos);
    }
    return s;
}

uint8_t getRegister(string str)
{
    if (str == "a") return 0;
    if (str == "b") return 1;
    if (str == "c") return 2;
    if (str == "d") return 3;
    return -1;
}

struct Inst
{
    string op;
    string arg1;
    string arg2;
    size_t pos;

    Inst(vector<string> v, size_t pos)
    {
        op = v[0];
        if (v.size() >= 2) arg1 = v[1];
        if (v.size() >= 3) arg2 = v[2];
        this->pos = pos;
    }

    bool isJump()
    {
        return
            op == "jmp" ||
            op == "jz" || op == "jn" || op == "jc" || op == "jo" ||
            op == "jnz" || op == "jnn" || op == "jnc" || op == "jno";
    }

    uint8_t getSize()
    {
        uint8_t reg = getRegister(arg1);

        bool isImm =
            op == "imm";

        bool isAddrImm =
            op == "st" && !arg2.empty() ||
            op == "ld" && !arg2.empty();

        bool isALUImm = reg == -1 && (
            op == "add" || op == "sub" ||
            op == "adc" || op == "sbc" ||
            op == "and" || op == "or" ||
            op == "xor");

        bool isJmp = isJump();

        return (isImm || isAddrImm || isALUImm || isJmp) ? 2 : 1;
    }
};

void compile(ifstream &src, uint8_t *program)
{
    unordered_map<string, uint8_t> labels;
    vector<Inst> insts;
    uint8_t addr = 0;
    string line;

    while (getline(src, line))
    {
        line = trim(line);
        line = removeComment(line);
        if (line.empty()) continue;

        stringstream ss(line);
        string s;
        vector<string> v;
        while (ss >> s)
        {
            v.push_back(s);
        }

        if (v[0] == "label")
        {
            labels[v[1]] = addr;
            continue;
        }

        Inst inst(v, addr);
        insts.push_back(inst);
        addr += inst.getSize();
    }

    for (Inst &inst : insts)
    {
        string op = inst.op;
        string arg1 = inst.arg1;
        string arg2 = inst.arg2;
        uint8_t pos = inst.pos;

        if (op == "nop")
        {
            program[pos] = 0x00;
        }
        if (op == "halt")
        {
            program[pos] = 0x10;
        }
        if (op == "mov")
        {
            program[pos] = 0x20 | (getRegister(arg1) << 2) | getRegister(arg2);
        }
        if (op == "imm")
        {
            program[pos] = 0x30 | (getRegister(arg1) << 2);
            program[pos + 1] = stoi(arg2);
        }
        if (op == "st")
        {
            // TODO: update this
            if (arg2.empty()) // it's now st rs ra
            {
                program[pos] = 0x40 | getRegister(arg1);
            }
            else
            {
                program[pos] = 0x50 | getRegister(arg1);
                program[pos + 1] = stoi(arg2);
            }
        }
        if (op == "ld")
        {
            if (arg2.empty())
            {
                program[pos] = 0x60 | (getRegister(arg1) << 2);
            }
            else
            {
                program[pos] = 0x70 | (getRegister(arg1) << 2);
                program[pos + 1] = stoi(arg2);
            }
        }
        if (op == "add" || op == "sub" || op == "adc" || op == "sbc" ||
            op == "and" || op == "or" || op == "xor" ||
            op == "shr" || op == "ror")
        {
            uint8_t byte;
            if (op == "add")
                byte = 0x80;
            if (op == "sub")
                byte = 0x84;
            if (op == "adc")
                byte = 0x88;
            if (op == "sbc")
                byte = 0x8c;
            if (op == "and")
                byte = 0x90;
            if (op == "or") 
                byte = 0x94;
            if (op == "xor")
                byte = 0x98;
            if (op == "shr")
                byte = 0x9c;
            if (op == "shr")
                byte = 0xbc;

            if (inst.getSize() == 1)
            {
                if (op != "shr" && op != "ror")
                    byte |= getRegister(arg1);
                program[pos] = byte;
            }
            else
            {
                byte |= 0x20;
                program[pos] = byte;
                program[pos + 1] = stoi(arg1);
            }
        }
        if (inst.isJump())
        {
            uint8_t byte;
            if (op == "jmp")
                byte = 0xc0;
            if (op == "jz")
                byte = 0xe0;
            if (op == "jn")
                byte = 0xe4;
            if (op == "jc")
                byte = 0xe8;
            if (op == "jo")
                byte = 0xec;
            if (op == "jnz")
                byte = 0xf0;
            if (op == "jnn")
                byte = 0xf4;
            if (op == "jnc")
                byte = 0xf8;
            if (op == "jno")
                byte = 0xfc;
            program[pos] = byte;
            program[pos + 1] = labels[arg1];
        }
    }
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        cerr << "Usage: " << argv[0] << " <source> [-p]" << endl;
        return 1;
    }

    string filename = argv[1];
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

    for (int i = 0; i < 256; ++i)
    {
        cout << hex << setw(2) << setfill('0') << (int)program[i] << " ";
        if (i % 16 == 15) cout << endl;
    }

    return 0;
}