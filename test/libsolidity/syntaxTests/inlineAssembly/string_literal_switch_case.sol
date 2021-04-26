contract C {
  function f() public pure {
    assembly {
      switch codesize()
      case "1" {}
      case "2" {}
    }
  }
}
// ----
