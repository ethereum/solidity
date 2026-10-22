uint constant base = 2**256 - 1;
contract C layout at base + 1 {}
// ----
// TypeError 2643: (54-62): Arithmetic error when computing constant value.
