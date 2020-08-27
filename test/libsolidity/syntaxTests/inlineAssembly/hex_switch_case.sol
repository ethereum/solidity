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
// ParserError 3772: (92-99): Hex literals are not valid in this context.
