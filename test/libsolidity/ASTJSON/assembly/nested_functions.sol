contract C {
  function f() public pure returns (uint x) {
    assembly {
      function f1() {
        function f2() { }
      }
      x := 2
    }
  }
}
// ----
