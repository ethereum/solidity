uint constant base = 512;
contract C layout at base - 1024 {}
// ----
// TypeError 2643: (47-58): Arithmetic error when computing constant value.
