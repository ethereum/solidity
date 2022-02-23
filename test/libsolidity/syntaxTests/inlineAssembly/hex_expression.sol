contract C {
  function f() public pure {
    assembly {
      pop(hex"2233")
    }
  }
}
// ----
