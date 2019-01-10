#ifndef WRAPPER_BLUETOOTH_DATA_H
#define WRAPPER_BLUETOOTH_DATA_H

#define NULL_COMMAND 0
#define COLOR_COMMAND 1

class BluetoothData {
    public:
    uint8_t command;
    char message[7];

    BluetoothData(char *line) {
        command = line[0] - '0';
        strlcpy(message, line + 1, 7);
    }
};

#endif