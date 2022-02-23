contract C {
  function f() public pure {
    assembly {
      switch codesize()
      case hex"00" {}
      case hex"1122" {}
    }
  }
}
// ----
