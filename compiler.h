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

map<string, uint8_t> opcode_map = {
    {"nop", 0b00000000},  {"mov", 0b00000000},  {"imml", 0b01000000},
    {"immh", 0b01010000}, {"st", 0b10000000},   {"ld", 0b10001000},
    {"add", 0b10010000},  {"sub", 0b10011000},  {"and", 0b10100000},
    {"or", 0b10101000},   {"xor", 0b10110000},  {"shr", 0b10111000},
    {"jz", 0b11000001},   {"jn", 0b11001001},   {"jc", 0b11010001},
    {"jmp", 0b11011001},  {"jnz", 0b11100001},  {"jnn", 0b11101001},
    {"jnc", 0b11110001},  {"call", 0b11111001}, {"ret", 0b11111000},
    {"halt", 0b11111111}};

uint8_t getOpcode(string &instruction) {
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

vector<uint8_t> compileAssembly(string &programString) {
    vector<uint8_t> program(256, 0);
    vector<uint8_t> res;

    vector<vector<string>> tokens = tokenize(programString);
    map<string, int> labels;

    bool prev_jmp = false;
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

        if (prev_jmp) {
            prev_jmp = false;
            uint8_t dest;
            if (labels.count(line[0]) != 0) {
                dest = labels[line[0]];
            } else {
                dest = stringToInteger(line[0]);
            }
            res.push_back(dest);
            address++;
            continue;
        }

        string opcode = line[0];
        uint8_t instruction = getOpcode(opcode);
        if (opcode == "mov") {
            instruction |= registerToken(line[1]) << 3;
            instruction |= registerToken(line[2]);
        } else if (opcode == "imml" || opcode == "immh") {
            instruction |= stringToInteger(line[1]);
        } else if (opcode == "add" || opcode == "sub" || opcode == "and" ||
                   opcode == "or" || opcode == "xor" || opcode == "shr") {
            instruction |= registerToken(line[1]);
        } else if (opcode == "jz" || opcode == "jn" || opcode == "jc" ||
                   opcode == "jnz" || opcode == "jnn" || opcode == "jnc" ||
                   opcode == "jmp") {
            prev_jmp = true;
        }

        res.push_back(instruction);
        address++;
    }

    copy(res.begin(), res.end(), program.begin());
    return program;
}