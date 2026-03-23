#include "program.h"

const int DATA_PINS[] = { A5, A4, A3, 6, 5, 4, 3, 2 };
const int N_WE_PIN = 8;
const int N_OE_PIN = 9;

const int SER_PIN = 7;
const int SRCLK_PIN = 10;
const int RCLK_PIN = 11;

void setAddress(uint16_t address) {
    uint8_t upperAddress = address >> 8;
    uint8_t lowerAddress = address;

    shiftOut(SER_PIN, SRCLK_PIN, MSBFIRST, upperAddress);
    shiftOut(SER_PIN, SRCLK_PIN, MSBFIRST, lowerAddress);

    digitalWrite(RCLK_PIN, 1);
    digitalWrite(RCLK_PIN, 0);
}

void writeData(uint8_t data, uint16_t address) {
    setAddress(address);

    for (int i = 0; i < 8; i++) {
        pinMode(DATA_PINS[i], OUTPUT);
    }
    for (int i = 0; i < 8; i++) {
        digitalWrite(DATA_PINS[i], 1 & (data >> i));
    }

    digitalWrite(N_WE_PIN, 0);
    delay(1);
    digitalWrite(N_WE_PIN, 1);
}

uint8_t readData(uint16_t address) {
    setAddress(address);

    for (int i = 0; i < 8; i++) {
        pinMode(DATA_PINS[i], INPUT);
    }
    digitalWrite(N_OE_PIN, 0);

    uint8_t data = 0;
    for (int i = 0; i < 8; i++) {
        data |= digitalRead(DATA_PINS[i]) << i;
    }

    digitalWrite(N_OE_PIN, 1);

    return data;
}


const size_t LOG_INTERVAL = 1024;

void writeROM(const uint8_t *data, size_t length) {
    for (size_t addr = 0; addr < length; addr++) {
        if (addr % LOG_INTERVAL == 0) {
            char buf[100] = {0};
            sprintf(buf, "Writing at %04x - %04x...", addr, min(addr + LOG_INTERVAL, length) - 1);
            Serial.println(buf);
        }

        uint8_t value = pgm_read_byte(&data[addr]);
        writeData(value, addr);
    }
    Serial.println("Write done.");
}

void verifyROM(const uint8_t *data, size_t length) {
    size_t mismatches = 0;

    for (size_t addr = 0; addr < length; addr++) {
        if (addr % LOG_INTERVAL == 0) {
            char buf[100] = {0};
            sprintf(buf, "Verifying at %04x - %04x...", addr, min(addr + LOG_INTERVAL, length) - 1);
            Serial.println(buf);
        }

        uint8_t expected = pgm_read_byte(&data[addr]);
        uint8_t got = readData(addr);

        if (expected != got) {
            char buf[100] = {0};
            sprintf(buf, "Mismatch at %04x! Expected %02x, got %02x", addr, expected, got);
            Serial.println(buf);
            
            mismatches++;
            if (mismatches > (size_t)10) {
                Serial.println("Too many mismatches. Aborting...");
                return;
            }
        }
    }
    Serial.println("Verify OK.");
}

void dumpROM(size_t start, size_t end) {
    for (size_t addr = start; addr < end; addr++) {
        if (addr % 16 == 0) {
            char buf1[10];
            sprintf(buf1, "%04x | ", addr);
            Serial.print(buf1);
        }
        uint8_t data = readData(addr);
        char buf2[5];
        sprintf(buf2, "%02x ", data);
        Serial.print(buf2);
        if (addr % 16 == 15) {
            Serial.println();
        }
    }
}

void setup() {
    pinMode(SRCLK_PIN, OUTPUT);
    pinMode(RCLK_PIN, OUTPUT);
    pinMode(SER_PIN, OUTPUT);
    pinMode(N_WE_PIN, OUTPUT);
    pinMode(N_OE_PIN, OUTPUT);
    digitalWrite(SRCLK_PIN, 0);
    digitalWrite(RCLK_PIN, 0);
    digitalWrite(N_WE_PIN, 1);
    digitalWrite(N_OE_PIN, 1);

    Serial.begin(115200);

    Serial.println("Press enter to begin EEPROM programming...");
    while (Serial.available() == 0);

    const int LENGTH = 256;
    writeROM(program, LENGTH);
    // delay(1);
    verifyROM(program, LENGTH);
    dumpROM(0, LENGTH);
}

void loop() {
}
