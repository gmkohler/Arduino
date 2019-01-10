#include <RBL_nRF8001.h>
#include <RBL_services.h>
#include <SPI.h>
#include <EEPROM.h>
#include <boards.h>


#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define LOGGING 0

#define PIN 6
#define NUM_LEDS 24
#define BRIGHTNESS 32

#define arrlen(arr) (sizeof(arr) / sizeof(arr[0]))

#define NULL_COMMAND 0
#define COLOR_COMMAND 1

// Forward-define methods
uint32_t corrected_hexcolor(uint32_t hex);
void setLEDColor(uint32_t hex);

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_RGB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code

  ble_set_name("Cav Pager");
  // Init. and start BLE library.
  ble_begin();

  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.show(); // Initialize all pixels to 'off'

  if(LOGGING) {
    Serial.begin(9600);    
  }
}

struct Data {
  uint8_t command;
  char message[7];
};

void loop() {
  if ( ble_available() < 7 ) {
    if (ble_available() && LOGGING) {
      Serial.print("Less than seven bytes available: ");
      Serial.println(ble_available());
    }
    ble_do_events();
    return;
  }

  struct Data data;
  data.command = ble_read() - '0';
  for (uint8_t k = 0; k < 6; ++k) data.message[k] = ble_read();
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
    
  ble_do_events();
}

// Fill the dots one after the other with a color
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
