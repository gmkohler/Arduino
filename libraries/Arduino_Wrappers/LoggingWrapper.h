#include <Arduino.h>

#if SOFTWARE_SERIAL_AVAILABLE
  #include <SoftwareSerial.h>
#endif

#ifndef WRAPPER_LOGGING_H
#define WRAPPER_LOGGING_H

#define PRINT_BUFFER_SIZE 256

class Logger {
    private:
    bool logging;
    char *buffer;

    public: 
    Logger(bool should_log) {
        logging = should_log;
        if (should_log) buffer = (char *) malloc(PRINT_BUFFER_SIZE * sizeof(char));
    }

    ~Logger() {
        if (logging) free(buffer);
    }

    void initialize() {
       if (!logging) return;

       while (!Serial);
       Serial.begin(115200);
       Serial.println("Serial available!"); 
    }

    void log(char *line, ...) {
        if (!logging) return;
        va_list args;
        va_start(args, line);
        vsprintf(buffer, line, args);
        Serial.println(buffer);
        va_end(args);
    }
};

#endif