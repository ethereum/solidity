contract C1 {
  function f() external pure returns(int) { return 42; }
}

contract C is C1 {
   int override f;
}
// ----
// TypeError 8022: (96-110): Override can only be used with public state variables.
