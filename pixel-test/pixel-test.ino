#include <RBL_nRF8001.h>
#include <RBL_services.h>
#include <SPI.h>
#include <EEPROM.h>
#include <boards.h>


#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 6
#define arrlen(arr) (sizeof(arr) / sizeof(arr[0]))

#define NULL_COMMAND 0
#define COLOR_COMMAND 1

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(24, PIN, NEO_GRB + NEO_KHZ800);

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
  strip.setBrightness(16);
  strip.show(); // Initialize all pixels to 'off'

  Serial.begin(9600);
}

struct Data {
  uint8_t command;
  char message[7];
};

void loop() {
  if ( ble_available() < 7 ) {
    if (ble_available()) {
      Serial.print("Less than seven bytes available: ");
      Serial.println(ble_available());
    }
    ble_do_events();
    return;
  }

  struct Data data;
  data.command = ble_read() - '0';

  Serial.print("command: ");
  Serial.println(data.command);

  for (uint8_t k = 0; k < 6; ++k) data.message[k] = ble_read();
  data.message[6] = '\0';
   
  switch (data.command) {
    case COLOR_COMMAND:  
      Serial.print("Message: ");
      Serial.println(data.message);
  
      uint32_t hex_color = strtol(data.message, (char**) NULL, 16);
  
      Serial.print("Hex_color: ");
      Serial.println(hex_color);
      
      colorWipe(hex_color, 0);
    default:
      Serial.print("Unrecognized command: ");
      Serial.println(data.command);
  }
    
  ble_do_events();
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels() - 5; i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}
