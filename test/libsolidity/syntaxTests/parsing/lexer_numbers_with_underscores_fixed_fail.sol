contract C {
  function f() public pure {
    fixed F1 = 3.1415_;
    fixed F2 = 3__1.4__15;
    fixed F3 = 1_.2;
    fixed F4 = 1._2;
    fixed F5 = 1.2e_12;
    fixed F6 = 1._;
  }
}
// ----
// SyntaxError: (57-64): Invalid use of underscores in number literal. No trailing underscores allowed.
// SyntaxError: (81-91): Invalid use of underscores in number literal. Only one consecutive underscores between digits allowed.
// SyntaxError: (108-112): Invalid use of underscores in number literal. No underscores in front of the fraction part allowed.
// SyntaxError: (129-133): Invalid use of underscores in number literal. No underscores in front of the fraction part allowed.
// SyntaxError: (150-157): Invalid use of underscores in number literal. No underscore in front of exponent allowed.
// SyntaxError: (174-177): Invalid use of underscores in number literal. No trailing underscores allowed.
