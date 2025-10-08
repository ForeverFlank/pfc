#include <bits/stdc++.h>

using namespace std;

int stringToInteger(string &str) {
    if (str.substr(0, 2) == "0b") {
        return stoi(str.substr(2), nullptr, 2);
    } else if (str.substr(0, 2) == "0x") {
        return stoi(str.substr(2), nullptr, 16);
    } else {
        return stoi(str, nullptr, 10);
    }
}

map<string, uint16_t> opcode_map = {
    {"nop", 0x0000},  {"mov", 0x0000},  {"and", 0x4000}, {"nor", 0x4800},
    {"nand", 0x5000}, {"nor", 0x5800},  {"add", 0x6000}, {"sub", 0x6800},
    {"xor", 0x7000},  {"shr", 0x7800},  {"imm", 0x8000}, {"st", 0xa000},
    {"ld", 0xa800},   {"call", 0xb000}, {"ret", 0xb800}, {"jz", 0xc000},
    {"jn", 0xc800},   {"jc", 0xd000},   {"jmp", 0xd800}, {"jnz", 0xe000},
    {"jnn", 0xe800},  {"jnc", 0xf000},  {"halt", 0xf800}};

uint16_t getOpcode(string &instruction) {
    if (opcode_map.count(instruction)) return opcode_map[instruction];
    throw runtime_error("Invalid opcode");
}

vector<vector<string>> tokenize(string &programString) {
    vector<vector<string>> tokens;
    istringstream lines(programString);
    string line;
    while (getline(lines, line)) {
        size_t commentPos = line.find(';');
        if (commentPos != string::npos) {
            line = line.substr(0, commentPos);
        }
        istringstream linestream(line);
        string word;
        vector<string> words;
        while (linestream >> word) {
            words.push_back(word);
        }
        if (!words.empty()) {
            tokens.push_back(words);
        }
    }
    return tokens;
}

uint8_t registerToken(string &token) {
    if (token.size() != 2) {
        throw runtime_error("Invalid register syntax: " + token);
    }
    if (token[0] != 'r') {
        throw runtime_error("Invalid register syntax: " + token);
    }
    if (!isdigit(token[1])) {
        throw runtime_error("Invalid register number: " + token);
    }

    uint8_t reg = token[1] - '0';
    if (reg < 0 || reg > 8) {
        throw runtime_error("Invalid register number: " + token);
    }
    return reg;
}

vector<uint16_t> compileAssembly(string &programString) {
    vector<uint16_t> program(2048, 0);
    vector<uint16_t> res;

    vector<vector<string>> tokens = tokenize(programString);
    map<string, int> labels;

    int address = 0;
    for (vector<string> &line : tokens) {
        if (line[0] == "label") {
            string name = line[1];
            labels[name] = address;
            continue;
        }
        address++;
    }

    address = 0;
    for (vector<string> &line : tokens) {
        if (line[0] == "label") continue;

        string opcode = line[0];
        uint8_t instruction = getOpcode(opcode);
        if (opcode == "mov") {
            instruction |= registerToken(line[1]) << 8;
            instruction |= registerToken(line[2]) << 4;
        } else if (opcode == "imm") {
            instruction |= registerToken(line[1]) << 8;
            instruction |= stringToInteger(line[2]);
        } else if (opcode == "and" || opcode == "or" || opcode == "nand" ||
                   opcode == "or" || opcode == "add" || opcode == "sub" ||
                   opcode == "xor" || opcode == "shr") {
            instruction |= registerToken(line[1]) << 8;
            instruction |= registerToken(line[2]) << 4;
            instruction |= registerToken(line[3]) << 0;
        } else if (opcode == "shr") {
            instruction |= registerToken(line[1]) << 8;
            instruction |= registerToken(line[2]) << 4;
        } else if (opcode == "jz" || opcode == "jn" || opcode == "jc" ||
                   opcode == "jnz" || opcode == "jnn" || opcode == "jnc" ||
                   opcode == "jmp" || opcode == "call") {
            instruction |= registerToken(line[1]);
        }

        res.push_back(instruction);
        address++;
    }

    copy(res.begin(), res.end(), program.begin());
    return program;
}