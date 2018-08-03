contract C {
  function f() public pure {
    uint D1 = 1234_;
    uint D2 = 12__34;
    uint D3 = 12_e34;
    uint D4 = 12e_34;
  }
}
// ----
// SyntaxError: (56-61): Invalid use of underscores in number literal. No trailing underscores allowed.
// SyntaxError: (77-83): Invalid use of underscores in number literal. Only one consecutive underscores between digits allowed.
// SyntaxError: (99-105): Invalid use of underscores in number literal. No underscore at the end of the mantissa allowed.
// SyntaxError: (121-127): Invalid use of underscores in number literal. No underscore in front of exponent allowed.
