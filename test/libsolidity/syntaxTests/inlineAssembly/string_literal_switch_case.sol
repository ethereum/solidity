contract C {
  function f() public pure {
    assembly {
      switch calldatasize()
      case "1" {}
      case "2" {}
    }
  }
}
// ----
