library L {}
contract C {
  function f() public pure {
    new L[](2);
  }
}
// ----
// TypeError 4409: (59-66): Cannot create arrays of libraries.
