/*
  Bluefruit Feather Tester

  This sketch provides a simple tester for Bluefruit Feather boards from Adafruit

  created 31 Jan. 2016
  by K. Townsend (KTOWN)
*/

#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "BluefruitConfig.h"

#if SOFTWARE_SERIAL_AVAILABLE
  #include <SoftwareSerial.h>
#endif

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

/**************************************************************************/
/*!

*/
/**************************************************************************/
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void setup()
{
  // initialize digital pin 13 as an output.
  pinMode(13, OUTPUT);

  // IO setup
  Serial.begin(115200);

  // Wait for the Serial Monitor to open
  while (!Serial) { yield(); }

  // Initialise the BLE module
  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }

  // Display the test suite selection menu
  display_menu();
}

/**************************************************************************/
/*!
    Reads and prints the AT response buffer until we reach OK or ERROR

    Returns 'true' if an error occurred, otherwise 'false'
*/
/**************************************************************************/
bool display_response(void)
{
  // Read the response until we get OK or ERROR
  while (ble.readline())
  {
    Serial.print(ble.buffer);
    if ( strcmp(ble.buffer, "OK") == 0 )
    {
      return false;
    }
    if ( strcmp(ble.buffer, "ERROR") == 0 )
    {
      return true;
    }
  }

  return true;
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void display_menu()
{
  delay(500);  // Short delay for cosmetic reasons
  Serial.println("");
  Serial.println("Bluetooth Blinking");
  Serial.println("-------------------------------------------------------------------------------");
  Serial.println("Tell the Arduino how many times to blink");
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void loop()
{
  
  // the setup function runs once when you press reset or power the board
  String user_input = "";
  int num_blinks = 0;

  // Wait for user feedback, then parse feedback one byte at a time
  while((Serial.available()) && !selection)
  {
    char incoming = Serial.read();
    if (isdigit(incoming))
    {
      // Append the current digit to the string placeholder
      user_input += (char)incoming;
    }
    // Parse the string on new-line
    if (incoming == '\n')
    {
      selection = user_input.toInt();
    }
    delay(2);
  }

  ble.print("Blinking ");
  ble.print(user_input);
  ble.println(" times");

  for(int j = 0; j < num_blinks; j++)
  {
    digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(500);              // wait for a second
    digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW

  }

  Serial.println("");
  }
}
