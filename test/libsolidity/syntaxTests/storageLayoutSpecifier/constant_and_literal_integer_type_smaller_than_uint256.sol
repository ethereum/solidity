uint8 constant base = 2;
contract A layout at base + 256 {}
contract B layout at base + 255 {} // Error, outside of range of constant and literal
// ----
// TypeError 2643: (81-91): Arithmetic error when computing constant value.
