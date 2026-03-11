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
    delayMicroseconds(1);
    digitalWrite(N_WE_PIN, 1);
}

uint8_t readData(uint16_t address) {
    setAddress(address);

    digitalWrite(N_OE_PIN, 0);

    uint8_t data = 0;
    for (int i = 0; i < 8; i++) {
        pinMode(DATA_PINS[i], INPUT);
    }
    for (int i = 0; i < 8; i++) {
        data |= digitalRead(DATA_PINS[i]) << i;
    }

    digitalWrite(N_OE_PIN, 1);

    return data;
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

    const char dataToWrite[] = "once upon an eeprom";

    for (size_t i = 0; i < sizeof(dataToWrite); i++) {
        uint8_t data = dataToWrite[i];

        char buf[100];
        sprintf(buf, "Writing %02x to address %04x", data, i);
        Serial.println(buf);

        writeData(data, i);
        delay(500);
    }

    delay(1);

    for (size_t i = 0; i < 256; i++) {
        if (i % 16 == 0) {
            char buf1[10];
            sprintf(buf1, "%04x | ", i);
            Serial.print(buf1);
        }
        uint8_t data = readData(i);
        char buf2[5];
        sprintf(buf2, "%02x ", data);
        Serial.print(buf2);
        if (i % 16 == 15) {
            Serial.println();
        }
    }
}

void loop() {
}
