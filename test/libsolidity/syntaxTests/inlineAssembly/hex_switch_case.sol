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
// ParserError 1856: (92-99): Literal or identifier expected.
