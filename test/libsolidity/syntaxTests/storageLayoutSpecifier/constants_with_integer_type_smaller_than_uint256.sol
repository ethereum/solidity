uint8 constant base = 255;
uint16 constant offset = 32767;
contract A layout at base {}            // Ok
contract B layout at base + offset {}   // Ok, inside uint16 range
contract D layout at base * offset {}   // Error, outside uint16 range
// ----
// TypeError 2643: (193-206): Arithmetic error when computing constant value.
