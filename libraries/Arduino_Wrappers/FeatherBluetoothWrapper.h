#include <Adafruit_BLE.h>
#include <Adafruit_BluefruitLE_SPI.h>
#include <Adafruit_BluefruitLE_UART.h>

#include "config/FeatherBluefruitConfig.h"
#include "BluetoothWrapper.h"

#ifndef WRAPPER_FEATHER_BLUETOOTH_H
#define WRAPPER_FEATHER_BLUETOOTH_H

class FeatherBluetooth : public Bluetooth {
    private:
    char buffer[BUFFER_SIZE];
    Adafruit_BluefruitLE_SPI ble;

    public:
    FeatherBluetooth() : ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST) {}

    void initialize(char *name) {
        ble.begin(VERBOSE_MODE);

        ble.echo(false);
        ble.info();
        ble.verbose(false);

        // AT+GAPDEVNAME= (14) + length of name + null terminator
        char *name_command = (char *) malloc((14 + strlen(name) + 1) * sizeof(char));
        strcpy(name_command, "AT+GAPDEVNAME=");
        strcat(name_command, name);

        ble.sendCommandCheckOK(name_command);

        free(name_command);

        while(!ble.isConnected()) delay(100);
    }

    char *read_line() {
        ble.println("AT+BLEUARTRX");
        ble.readline();

        if (strcmp(ble.buffer, "OK") == 0) buffer[0] = '\0';
        else strlcpy(buffer, ble.buffer, BUFFER_SIZE);

        return buffer;
    }

    void proceed() { ble.waitForOK(); }
};

#endif