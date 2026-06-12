#include <digitalWriteFast.h>

const int DATA_PINS[] = { 2, 3, 4, 5, 6, 7, 8, 9 };

const int SER1_PIN = A1;
const int SER2_PIN = A2;
const int SER3_PIN = A3;

const int SRCLK_PIN = A4;
const int RCLK_PIN = A5;

void setRegisterOutput(uint8_t d1, uint8_t d2, uint8_t d3) {
    for (int i = 7; i >= 0; i--) {
        digitalWriteFast(SER1_PIN, 1 & (d1 >> i));
        digitalWriteFast(SER2_PIN, 1 & (d2 >> i));
        digitalWriteFast(SER3_PIN, 1 & (d3 >> i));
        digitalWriteFast(SRCLK_PIN, 1);
        digitalWriteFast(SRCLK_PIN, 0);
    }
    digitalWriteFast(RCLK_PIN, 1);
    digitalWriteFast(RCLK_PIN, 0);
}

uint8_t readData() {
    uint8_t data = 0;
    for (int i = 0; i < 8; i++) {
        data |= digitalReadFast(DATA_PINS[i]) << i;
    }
    return data;
}

bool assertData(uint8_t expected, uint8_t d1, uint8_t d2, uint8_t d3) {
    setRegisterOutput(d1, d2, d3);
    uint8_t data = readData();
    bool result = data == expected;

    if (result) {
        // sprintf(buf, "Passed: %02x, %02x, %02x -> %02x",
        //         d1, d2, d3, expected);
    } else {
        char buf[200];
        sprintf(buf, "\nFailed: %02x, %02x, %02x -> expected %02x, got %02x ",
                d1, d2, d3, expected, data);
        Serial.print(buf);
    }
    
    return result;
}

void testALU() {
    for (int c = 0; c < 32; c++) {
        char buf[200];
        sprintf(buf, "Testing [%d / 32]", c);
        Serial.println(buf);

        long totalCases = 0;
        long passedCases = 0;

        for (int b = 0; b < 256; b++) {
            for (int a = 0; a < 256; a++) {
                uint8_t selAdder   = 0b01 & (c >> 0);
                uint8_t selBitwise = 0b11 & (c >> 1);
                uint8_t cin        = 0b01 & (c >> 3);
                uint8_t invB       = 0b01 & (c >> 4);

                uint8_t expected = 0;
                uint8_t actualB  =  (invB ? ~b : b);

                if (selAdder) {
                    expected = a + actualB + cin;
                } else {
                    if (selBitwise == 0) expected = a & actualB;
                    if (selBitwise == 1) expected = a | actualB;
                    if (selBitwise == 2) expected = a ^ actualB;
                    if (selBitwise == 3) expected = (a >> 1) | (cin << 7);
                }

                passedCases += assertData(expected, a, b, c) ? 1 : 0;
                totalCases++;
            }
            if (b % 4 == 3) Serial.print(".");
        }
        sprintf(buf, "\nTest passed: %ld / %ld (%ld%%)", 
                passedCases, totalCases, (long)100 * passedCases / totalCases);
        Serial.println(buf);
    }
}

void setup() {
    Serial.begin(115200);

    pinMode(SER1_PIN, OUTPUT);
    pinMode(SER2_PIN, OUTPUT);
    pinMode(SER3_PIN, OUTPUT);

    pinMode(SRCLK_PIN, OUTPUT);
    pinMode(RCLK_PIN, OUTPUT);

    digitalWriteFast(SRCLK_PIN, 0);
    digitalWriteFast(RCLK_PIN, 0);

    for (int i = 0; i < 8; i++) {
        pinMode(DATA_PINS[i], INPUT);
    }

    Serial.println("Press enter to start.");
    while (Serial.available() == 0);

    testALU();
}

void loop() {
}
