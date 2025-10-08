#include <bits/stdc++.h>
#include "compiler.h"

void printHex(uint8_t value) {
    cout << hex << setfill('0') << setw(2) << static_cast<int>(value);
}

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

void printOutputFile(string& outputFilename, vector<uint16_t>& program) {
    cout << "Output hex machine code (" << outputFilename << "):" << endl;
    for (int i = 0; i < 16; i++) { // Adjusted loop for 16-bit values
        cout << " ";
        printHex(i * 8);
        cout << " | ";
        for (int j = 0; j < 8; j++) {
            cout << hex << setfill('0') << setw(4) << program[8 * i + j];
            cout << " ";
        }
        cout << endl;
    }
    cout << endl;
}

int main(int argc, char* argv[]) {
    string inputFilename;
    string outputFilename;

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "-i" && i + 1 < argc) {
            inputFilename = argv[++i];
        } else if (arg == "-o" && i + 1 < argc) {
            outputFilename = argv[++i];
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

    vector<uint16_t> program = compileAssembly(programString);

    ofstream outFile(outputFilename, ios::binary);
    outFile.write(reinterpret_cast<const char*>(program.data()),
                    program.size());
    outFile.close();
    printOutputFile(outputFilename, program);

    return 0;
}