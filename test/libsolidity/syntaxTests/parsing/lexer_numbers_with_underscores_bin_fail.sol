contract C {
  function f() public pure {
    uint X1 = 0b010101__1100__001100__1001001;
  }
}
// ----
// SyntaxError 2990: (56-87): Invalid use of underscores in number literal. Only one consecutive underscore between digits is allowed.
