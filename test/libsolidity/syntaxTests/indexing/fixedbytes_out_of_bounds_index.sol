contract C {
  function f() public {
    bytes32 b;
    b[64];
  }
}
// ----
// TypeError 1859: (56-61): Out of bounds array access.
