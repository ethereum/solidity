contract C {
  function f() public pure {
    uint X1 = 0x1234__1234__1234__123;
  }
}
// ----
// SyntaxError: (56-79): Invalid use of underscores in number literal. Only one consecutive underscores between digits allowed.
