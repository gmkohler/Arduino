#include "GammaCorrection.h"

#ifndef WRAPPER_BLUETOOTH_DATA_H
#define WRAPPER_BLUETOOTH_DATA_H

#define NULL_COMMAND 0
#define COLOR_COMMAND 1
#define ANIMATE_COMMAND 2

class BluetoothData {
    public:
    uint8_t command;
    char message[7];

    void read(char *line) {
        command = line[0] - '0';
        strlcpy(message, line + 1, 7);
    }

    uint32_t hex_color() {
        return strtol(message, (char **) NULL, 16);
    }

    uint32_t gamma_corrected_hex() {
        return gamma_correct_hex(hex_color());
    }
};

#endif