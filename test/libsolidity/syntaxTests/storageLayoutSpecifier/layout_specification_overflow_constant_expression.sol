uint constant base = 2**256 - 1;
uint constant offset = 512;
contract C layout at base + offset {}
// ----
// TypeError 2643: (82-95): Arithmetic error when computing constant value.
