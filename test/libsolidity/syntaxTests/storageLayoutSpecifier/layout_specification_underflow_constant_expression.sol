uint constant base = 256;
uint constant offset = 512;
contract C layout at base - offset {}
// ----
// TypeError 2643: (75-88): Arithmetic error when computing constant value.
