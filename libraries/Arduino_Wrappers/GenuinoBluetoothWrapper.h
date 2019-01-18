#include <RBL_nRF8001.h>
#include <RBL_services.h>
#include <SPI.h>
#include <EEPROM.h>
#include <boards.h>

#include "BluetoothWrapper.h"

#ifndef WRAPPER_GENUINO_BLUETOOTH_H
#define WRAPPER_GENUINO_BLUETOOTH_H

class GenuinoBluetooth : public Bluetooth {
    private:
    char buffer[BUFFER_SIZE];

    public:
    void initialize(char *name) { 
        ble_set_name(name);
        ble_begin();
    }
    
    char *read_line() {
        unsigned k;
        
        // Read into buffer
        for (k = 0; k < BUFFER_SIZE - 1 && ble_available(); ++k) buffer[k] = ble_read();
        // Null terminate buffer
        buffer[k] = '\0';

        // Return buffer
        return buffer;
    }

    void proceed() { ble_do_events(); }
};

#endif