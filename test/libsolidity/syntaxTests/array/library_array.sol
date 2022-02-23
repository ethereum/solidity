library L {}
contract C {
  function f() public pure {
    new L[](2);
  }
}
// ----
// TypeError 1130: (63-64): Invalid use of a library name.
