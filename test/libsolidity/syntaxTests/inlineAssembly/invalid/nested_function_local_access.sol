contract C {
  function f() public pure returns (uint x) {
    assembly {
      function f1() {
        function f2() { }
        x := 2
      }
    }
  }
}
// ----
// DeclarationError 6578: (130-131): Cannot access local Solidity variables from inside an inline assembly function.
