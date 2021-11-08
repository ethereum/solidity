contract C {
  function f() public pure {
    assembly {
      switch calldatasize()
      case hex"00" {}
      case hex"1122" {}
    }
  }
}
// ----
