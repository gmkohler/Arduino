#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_BLE.h>
#include <Adafruit_BluefruitLE_SPI.h>
#include <Adafruit_BluefruitLE_UART.h>

#include "BluefruitConfig.h"

#if SOFTWARE_SERIAL_AVAILABLE
  #include <SoftwareSerial.h>
#endif

#define NULL_COMMAND 0
#define COLOR_COMMAND 1

#define ARR_LEN(a) (sizeof(a) / sizeof(a[0]))

// LED Ring Attributes
#define PIN 6
#define NUM_LEDS 16
#define BRIGHTNESS 32

#define LOGGING 0

// Initialize the LED and BLE
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_RGB + NEO_KHZ800);
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

// Forward-define methods
uint32_t corrected_hexcolor(uint32_t hex);
void setLEDColor(uint32_t hex, uint8_t wait);

struct Data {
    uint8_t command;
    char message[7];
};

void setup() {
    if(LOGGING) {
      while(!Serial); // Wait for Serial to become available
      Serial.begin(115200);
      Serial.println("Serial available!");
      if(!ble.begin(VERBOSE_MODE)) {
          Serial.println("Bluefruit error!");
          while(1);
      }
      ble.factoryReset();
    } else {
      ble.begin(VERBOSE_MODE);
    }

    ble.echo(false);
    ble.info();
    ble.verbose(false);
    ble.sendCommandCheckOK(F("AT+GAPDEVNAME=Cav PagerB"));
    // Wait for connection
    while(!ble.isConnected()) delay(500);

    strip.begin();
    strip.setBrightness(BRIGHTNESS);
    strip.show(); // Initialize all pixels to 'off'
}

void loop() {
    struct Data data;

    // Check for incoming characters from Bluefruit
    ble.println("AT+BLEUARTRX");
    ble.readline();
    // Return if insufficient data
    if (strlen(ble.buffer) < 7) {
        ble.waitForOK();
        return;
    }
    if(LOGGING) {
      Serial.print("buffer: ");
      Serial.println(ble.buffer);
    }

    data.command = ble.buffer[0] - '0';
    for (uint8_t k = 0; k < 6; ++k) data.message[k] = ble.buffer[k + 1];
    data.message[6] = '\0';

    if(LOGGING) {
      Serial.print("command: ");
      Serial.println(data.command);
      Serial.print("message: ");
      Serial.println(data.message);
    }

    switch (data.command) {
        case NULL_COMMAND:
          if(LOGGING) {
            Serial.print("Null command: ");
            Serial.println(data.command);
          }
          setLEDColor(0, 35);
          break;
        case COLOR_COMMAND:
          uint32_t hex_color = strtol(data.message, (char **) NULL, 16);
          if(LOGGING) {
            Serial.print("Got color: ");
            Serial.println(hex_color);
          }
          setLEDColor(corrected_hexcolor(hex_color), 35);
          break;
        default:
          if(LOGGING) {
            Serial.print("Unrecognized command: ");
            Serial.println(data.command);
          }
          break;
    }

    ble.waitForOK();
}

void setLEDColor(uint32_t hex, uint8_t wait) {
    for (uint8_t k = 1; k <= strip.numPixels(); ++k) {
        strip.setPixelColor(NUM_LEDS - k, hex);
        strip.show();
        delay(wait);
    }
}

/////////////////////////////////////////////////
// Gamma correction nonsense
/////////////////////////////////////////////////

#define CORRECT_GAMMA(c) pgm_read_byte(&gamma8[c])

const uint8_t PROGMEM gamma8[] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,   1,   1,
    1,   1,   1,   1,   1,   1,   1,   1,   1,   2,   2,   2,   2,   2,   2,   2,
    2,   3,   3,   3,   3,   3,   3,   3,   4,   4,   4,   4,   4,   5,   5,   5,
    5,   6,   6,   6,   6,   7,   7,   7,   7,   8,   8,   8,   9,   9,   9,   10,
    10,  10,  11,  11,  11,  12,  12,  13,  13,  13,  14,  14,  15,  15,  16,  16,
    17,  17,  18,  18,  19,  19,  20,  20,  21,  21,  22,  22,  23,  24,  24,  25,
    25,  26,  27,  27,  28,  29,  29,  30,  31,  32,  32,  33,  34,  35,  35,  36,
    37,  38,  39,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  50,
    51,  52,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  64,  66,  67,  68,
    69,  70,  72,  73,  74,  75,  77,  78,  79,  81,  82,  83,  85,  86,  87,  89,
    90,  92,  93,  95,  96,  98,  99,  101, 102, 104, 105, 107, 109, 110, 112, 114,
    115, 117, 119, 120, 122, 124, 126, 127, 129, 131, 133, 135, 137, 138, 140, 142,
    144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 167, 169, 171, 173, 175,
    177, 180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213,
    215, 218, 220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252, 255
};

uint32_t corrected_hexcolor(uint32_t hex) {
    uint8_t color[3];

    color[0] = (uint8_t) CORRECT_GAMMA(hex >> 16);
    color[1] = (uint8_t) CORRECT_GAMMA((hex & 0xff00) >> 8);
    color[2] = (uint8_t) CORRECT_GAMMA(hex & 0xff);

    return ((color[0] << 16) + (color[1] << 8) + color[2]);
}
