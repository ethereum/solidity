contract C {
  function f() public pure {
    assembly {
      pop(bin"10110110")
    }
  }
}
// ----
