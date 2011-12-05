
/* Simple temperature controlled relay (heater thermostat)
 * Turn on the relay if temperature is < 38 degrees.
 * Run the relay for at most 10 minutes before a 5 minute cooling period.
 * A single-button interface turns on the backlight and lets you switch 
 * read-outs:
 *   Current temperature/status
 *   Min/Max temperature
 *   Elapsed time on
 * Long-press resets the elapsed/min/max values
 * The LCD sleeps with no backlight until button pushed
 *
 * Pinout:
 * LCD: digital pins 4, 5, 6, 7, 8, 9, 22k pull-ups
 * Signal LED: digital pin 13
 * Power relay: MOSFET transistor on pin 12, driving relay, 100k pull-down
 * LCD backlight: digital pin 11, MOSFET transistor on pin 11, 100k pull-down
 * Pushbutton: digital pin 2, button pulls low, 47k pull-up
 * Temperature sensor: analog input pin 0
 * 47k pull-up for and 1 (serial RX)
 * resonator, power, reset logic
 */
#include <LiquidCrystal.h>
#include <Menu.h>

inline void *operator new(size_t, void *arg) { return arg; }

LiquidCrystal lcd(4, 5, 6, 7, 8, 9);
Menu menu(lcd);
char displayData[2][16];

void setup() {
  new (lcd_) LiquidCrystal(4, 5, 6, 7, 8, 9);
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("  F/w " __DATE__);
  delay(2000);
}

void loop() {
}

