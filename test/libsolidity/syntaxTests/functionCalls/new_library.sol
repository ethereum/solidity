library L {}
contract C {
  function f() public pure {
    new L();
  }
}
// ----
// TypeError 1130: (63-64='L'): Invalid use of a library name.
