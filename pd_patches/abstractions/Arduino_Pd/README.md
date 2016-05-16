# Arduino_Pd

Arduino_Pd

This repository is a set of Pd abstractions (both vanilla and extended) and Arduino code that facilitates the communication between the two platforms.

"serial_print" (and its extended counterpart "serial_print_extended") works with the Serial.print() and Serial.println() functions of the Arduino.

"serial_write" works with the Serial.write() function of the Arduino.

Check their help patches and Arduino code for more information.

Written by Alexandros Drymonitis




Update December 2015: enabled sending lists including symbols under one tag

Update December 2015: corrected the issue with sending a bang while there is data saved in the abstraction

Update December 2015: enabled the word "float" to be used as a tag in "serial_print_extended" (although it's probably not a good idea to use this word in Pd...)
