contract C {
  function f() public pure {
    assembly {
      function g() {
        leave
      }
    }
  }
}
// ----
