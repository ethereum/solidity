contract C {
  function f(uint, uint) public {}
  function f(uint) public {}
  function g() public { f(1, 2, 3); }
}
// ----
// TypeError: (101-102): No matching declaration found after argument-dependent lookup.
