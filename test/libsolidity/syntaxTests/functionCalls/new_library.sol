library L {}
contract C {
  function f() public pure {
    new L();
  }
}
// ----
// TypeError 8696: (59-64): Cannot instantiate a library.
