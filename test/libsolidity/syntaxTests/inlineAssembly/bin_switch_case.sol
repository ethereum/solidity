contract C {
  function f() public pure {
    assembly {
      switch codesize()
      case bin"00110011" {}
      case bin"11001100" {}
    }
  }
}
// ----
