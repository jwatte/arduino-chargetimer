
This project implements a timer for a battery charger that needs to run 
on an infrequent schedule (such as once a month). It also has a button 
to run "now." This is useful for when you have something like a 
cordless drill or electric bicycle or other thing that you don't use 
every day, but still need to keep in a charged state.

The circuit is:
    * Arduino Uno
    * Triac relay connected on pin D12
    * ChronoDot connected on pins A4/A5
    * Indicator LED connected on pin D13
    * 16x2 LCD connected on pins D3-D8
    * 4 push buttons (pull up, active low) connected on pins A0-A3

The push buttons are "soft buttons" indicated on the second row of the 
LCD, with A0 being leftmost and A3 being rightmost.
