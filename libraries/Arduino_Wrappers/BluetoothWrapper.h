#ifndef WRAPPER_BLUETOOTH_H
#define WRAPPER_BLUETOOTH_H

// Buffer size, in bytes
#define BUFFER_SIZE 64

class Bluetooth {
    public:
    virtual void initialize(char *name) = 0;
    virtual char *read_line() = 0;
    virtual void proceed() = 0;
};

#endif